# Design Specification: Fix Audio Delay & Latency in Coral Linux

- **Date**: 2026-09-24
- **Topic**: Fix Audio Delay & Latency (Low-Latency Audio Pipeline & Drift Guard)
- **Target Verification Device**: Bluetooth Headphones (`bluez_output.*` / L80PRO)
- **Status**: Approved by User

---

## 1. Context & Problem Statement

### 1.1 Symptoms
When using Coral for system-wide audio enhancement on Linux (PipeWire / PulseAudio bridge), users experience a constant audio delay of approximately **100ms – 200ms**. This causes noticeable lip-sync mismatch when watching videos (e.g., YouTube in Brave browser), delayed UI feedback sounds, and audio-video desynchronization.

### 1.2 Root Cause Analysis
Empirical inspection of the running PipeWire graph (`pw-dump`, `pw-top`, `pactl list sink-inputs`) and authoritative source code in [`audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp`](file:///home/neuro/Documents/Tools/fxsound-linux/linux/coral-app/audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp) identified excessive static buffer allocations along the entire audio pipeline:

1. **Playback Stream Buffer Bloat (`tlength` & `prebuf`)**:
   - `tlength` was configured to `48ms`, which PipeWire expanded to `27648` bytes (**72ms / 3456 samples**).
   - `prebuf` was set to `(uint32_t)-1`, which forced PipeWire to buffer `18440` bytes (**48ms**) before allowing playback to start.
2. **Capture Stream Fragment Delay (`fragsize`)**:
   - `fragsize` was configured to `24ms` (`9216` bytes / 1152 samples), delaying captured audio from `CoralSink.monitor` before delivery to the processing thread.
3. **Large Chunk Processing Window (`num_samples`)**:
   - `num_samples` was set to `1024` samples (**~21.33ms**).
4. **Missing Latency Hint on Virtual Sink (`CoralSink`)**:
   - `module-null-sink` was loaded without `node.latency`, causing PipeWire to schedule `CoralSink` with a large default quantum (1024 samples).
5. **Measured Coral Pipeline Latency**:
   - PipeWire directly reported **`minNs: 137,187,499` ns (~137.2ms)** of internal latency for Coral's playback node (Node 98).
   - Adding Bluetooth A2DP packet transport latency (~60–80ms), total latency reached **~200–220ms**.
6. **No Drift Protection & Unbounded `maxlength`**:
   - `maxlength` was set to `(uint32_t)-1` (4MB = ~11 seconds). Any temporary playback hiccup or clock drift between the virtual sink driver and hardware DAC/Bluetooth would accumulate in the playback buffer without recovery.

*(Note: The buffers were previously increased in older commits because the audio thread was executing mutex locks and `pactl` system calls. Now that commits `846c8ff` and `6709e62` have fully made the audio thread lock-free and zero-alloc, large buffer sizes are obsolete and harmful).*

---

## 2. Solution Architecture

We will implement **Option 1: Comprehensive Low-Latency Audio Pipeline & Active Drift Guard** in [`AudioPassthruLinux.cpp`](file:///home/neuro/Documents/Tools/fxsound-linux/linux/coral-app/audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp).

```
[ App: Brave / System Audio ]
              │
              ▼ (PipeWire low-quantum: 256 samples / ~5.33ms)
       [ CoralSink (module-null-sink) ]
              │
              ▼ (fragsize: 256 samples / ~5.33ms)
       [ pa_simple_read (s_read) ]
              │
              ▼
   [ DFX DSP (256 samples) + C1 Limiter ]  < 0.05ms CPU time
              │
              ▼ (tlength: 512 samples / ~10.67ms, prebuf: 256 samples / ~5.33ms)
       [ pa_simple_write (s_write) ]
              │
              ▼ (Active Drift Guard monitoring via pa_simple_get_latency)
   [ Hardware Sink: Bluetooth Headphones (L80PRO) ]
```

---

## 3. Detailed Technical Specifications

### 3.1 Virtual Sink Configuration (`init()`)
When initializing `CoralSink`, pass low-latency properties to `module-null-sink`:
```cpp
std::string load_cmd = std::string("pactl load-module module-null-sink sink_name=") + kCoralSink
    + " sink_properties=device.description=Coral,node.latency=256/48000,media.class=Audio/Sink 2>/dev/null";
```
This forces PipeWire to allocate a 256-sample quantum (~5.33ms at 48kHz) to `CoralSink`.

### 3.2 Processing Chunk Size (`num_samples`)
- Reduce `num_samples` from `1024` to `256` samples.
- At 48kHz stereo 32-bit float:
  $$\text{buf\_size} = 256 \times 2 \times \text{sizeof(float)} = 2048 \text{ bytes}$$
- Total duration per processing chunk: $256 / 48000 \approx 5.33\text{ ms}$.

### 3.3 Buffer Attributes (`pa_buffer_attr`)
Replace the monolithic `48ms` / `24ms` settings with fine-grained low-latency attributes:

1. **Capture Buffer Attributes (`attr_read`)**:
   - `fragsize`: `pa_usec_to_bytes(5333, &ss)` (256 samples = 2048 bytes = ~5.33ms).
   - `maxlength`: `pa_usec_to_bytes(40000, &ss)` (40ms maximum ceiling).
   - `tlength`, `prebuf`, `minreq`: `(uint32_t)-1` (not applicable for record streams).

2. **Playback Buffer Attributes (`attr_write`)**:
   - `tlength`: `pa_usec_to_bytes(10666, &ss)` (512 samples = 4096 bytes = ~10.67ms).
   - `minreq`: `pa_usec_to_bytes(5333, &ss)` (256 samples = 2048 bytes = ~5.33ms).
   - `prebuf`: `pa_usec_to_bytes(5333, &ss)` (256 samples = 2048 bytes = ~5.33ms).
   - `maxlength`: `pa_usec_to_bytes(40000, &ss)` (40ms ceiling, preventing runaway memory/latency bloat).

### 3.4 Active Drift Guard
Inside the main audio loop in `threadWorker()`:
- Add a loop counter `guard_counter`.
- Every 200 iterations (~1.06 seconds):
  - Call `pa_usec_t lat = pa_simple_get_latency(s_write, &error);`.
  - Evaluate drift threshold:
    - If sink is Bluetooth (`current_hw_sink.find("bluez") != std::string::npos`): threshold is `140,000` $\mu$s (140ms).
    - If sink is ALSA/USB: threshold is `50,000` $\mu$s (50ms).
  - If measured `lat > threshold`:
    - Check hysteresis cooldown (ensure at least 3 seconds have passed since the previous flush).
    - If cooldown satisfied:
      - Call `pa_simple_flush(s_write, &error);`.
      - Reset cooldown timer.
- Zero allocations, zero system calls, zero mutexes on the audio thread.

---

## 4. Verification & Testing Plan

### 4.1 Target Device Focus
As requested by the user, testing and empirical verification will focus specifically on **Bluetooth Headphones** (`bluez_output.DC:E9:E0:97:BE:8C` / L80PRO).

### 4.2 Build Verification
- Compile `audiopassthru` library:
  ```bash
  cd audiopassthru && make clean && make
  ```
- Compile `coral` binary:
  ```bash
  cd coral/Builds/LinuxMakefile && make -j$(nproc)
  ```
- Confirm **0 compiler warnings, 0 errors**.

### 4.3 PipeWire Latency Metrics Verification
- Run the updated `coral` binary.
- Inspect latency parameters using `pw-dump`:
  - Verify Coral playback node latency drops from `137.2ms` to **$\le$ 25ms**.
  - Verify `CoralSink` has `node.latency = "256/48000"`.

### 4.4 Runtime Stability & Audio Quality Check
- Play audio in Brave / media player through Bluetooth headphones.
- Run `pw-top -b -n 10` to monitor:
  - Verify `ERR` column remains **0** (no underruns / buffer dropouts / crackling).
  - Verify seamless, crystal-clear audio passthrough.
- Verify real-time video synchronization (lip-sync is completely aligned).
