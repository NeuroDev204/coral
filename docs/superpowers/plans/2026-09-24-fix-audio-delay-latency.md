# Fix Audio Delay & Latency in Coral Linux Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Eliminate audio delay in Coral on Linux by optimizing the audio passthru pipeline to low-latency parameters (~5.33ms chunks, ~10.67ms target buffer) and implementing an active drift guard to maintain real-time sync on Bluetooth headphones.

**Architecture:** Configure `CoralSink` with `node.latency=256/48000`, reduce `threadWorker` processing chunks to 256 samples (~5.33ms), tighten `pa_buffer_attr` on capture and playback streams, and periodically monitor playback latency with `pa_simple_get_latency` to flush any buffer bloat beyond safe thresholds.

**Tech Stack:** C++17, PulseAudio Simple API (`libpulse-simple`), PipeWire, GNU Make, GCC.

## Global Constraints

- Target Verification Device: Bluetooth Headphones (`bluez_output.*` / L80PRO)
- Audio Thread Safety: Zero dynamic heap allocations (`malloc`/`free`), zero blocking mutex locks, and zero system calls (`popen`/`system`) in the inner audio loop.
- Bit-perfect transparent processing with C1-continuous soft-knee limiter bounded at `< 1.0f`.
- Preserve existing dynamic output switching and headphone hotplug detection logic.

---

### Task 1: Unit Tests for Low-Latency Buffer & Drift Guard Logic

**Files:**
- Create: `tests/test_low_latency_buffer.cpp`

**Interfaces:**
- Consumes: Sample format parameters (48kHz, float32, stereo) and drift threshold logic
- Produces: Executable unit test verifying exact buffer byte sizes, microsecond calculations, and drift guard thresholding

- [ ] **Step 1: Write the failing unit test**

Create `tests/test_low_latency_buffer.cpp`:
```cpp
#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <string>

// Test timing and byte calculations for low-latency audio passthru
constexpr uint32_t kSampleRate = 48000;
constexpr uint32_t kChannels = 2;
constexpr uint32_t kBytesPerSample = sizeof(float); // 4 bytes
constexpr uint32_t kNumSamples = 256;

inline size_t samplesToBytes(uint32_t samples) {
    return samples * kChannels * kBytesPerSample;
}

inline uint32_t usecToBytes(uint32_t usec) {
    return static_cast<uint32_t>((static_cast<uint64_t>(usec) * kSampleRate * kChannels * kBytesPerSample) / 1000000ULL);
}

inline bool shouldTriggerDriftFlush(const std::string& sink_name, uint64_t latency_usec, uint64_t current_iter, uint64_t last_flush_iter) {
    uint64_t threshold = (sink_name.find("bluez") != std::string::npos) ? 140000ULL : 50000ULL;
    bool cooldown_ok = (current_iter >= last_flush_iter + 600); // ~3.2 seconds cooldown at 256 samples/iter
    return (latency_usec > threshold) && cooldown_ok;
}

int main() {
    // 1. Verify buffer sizes
    size_t chunk_bytes = samplesToBytes(kNumSamples);
    assert(chunk_bytes == 2048); // 256 * 2 * 4 = 2048 bytes

    // 2. Verify usec conversions
    // 5333 usec (~5.33ms) -> ~256 samples
    uint32_t frag_bytes = usecToBytes(5333);
    uint32_t frag_samples = frag_bytes / (kChannels * kBytesPerSample);
    assert(frag_samples >= 255 && frag_samples <= 256);

    // 10666 usec (~10.67ms) -> ~512 samples
    uint32_t tlength_bytes = usecToBytes(10666);
    uint32_t tlength_samples = tlength_bytes / (kChannels * kBytesPerSample);
    assert(tlength_samples >= 511 && tlength_samples <= 512);

    // 40000 usec (~40ms) max buffer ceiling
    uint32_t max_bytes = usecToBytes(40000);
    assert(max_bytes == 15360); // 40ms * 48 samples/ms * 8 bytes = 15360 bytes

    // 3. Verify drift threshold logic for Bluetooth
    std::string bt_sink = "bluez_output.DC:E9:E0:97:BE:8C";
    assert(!shouldTriggerDriftFlush(bt_sink, 120000, 1000, 0)); // Normal BT latency (120ms < 140ms) -> No flush
    assert(shouldTriggerDriftFlush(bt_sink, 160000, 1000, 0));  // Excessive BT latency (160ms > 140ms, cooldown ok) -> Flush
    assert(!shouldTriggerDriftFlush(bt_sink, 160000, 1200, 1000)); // Excessive but within cooldown (200 < 600) -> No flush

    // 4. Verify drift threshold logic for ALSA
    std::string alsa_sink = "alsa_output.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__Speaker__sink";
    assert(!shouldTriggerDriftFlush(alsa_sink, 35000, 1000, 0)); // Normal ALSA latency (35ms < 50ms) -> No flush
    assert(shouldTriggerDriftFlush(alsa_sink, 65000, 1000, 0));  // Excessive ALSA latency (65ms > 50ms) -> Flush

    std::cout << "All low-latency buffer & drift guard tests PASSED!" << std::endl;
    return 0;
}
```

- [ ] **Step 2: Run test to verify it compiles and passes**

Run:
```bash
g++ -std=c++17 tests/test_low_latency_buffer.cpp -o /tmp/test_low_latency && /tmp/test_low_latency
```
Expected: Output `All low-latency buffer & drift guard tests PASSED!`.

- [ ] **Step 3: Commit**

```bash
git add tests/test_low_latency_buffer.cpp
git commit -m "test(audio): add unit tests for low-latency buffer math and drift guard logic"
```

---

### Task 2: Implement Low-Latency Pipeline & Active Drift Guard in AudioPassthruLinux

**Files:**
- Modify: `audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp`
- Rebuild: `audiopassthru/libaudiopassthru.a`, `libcombined.a`

**Interfaces:**
- Consumes: `pactl load-module module-null-sink`, `pa_buffer_attr`, `pa_simple_read`, `pa_simple_write`, `pa_simple_get_latency`, `pa_simple_flush`
- Produces: Rebuilt `libaudiopassthru.a` and `libcombined.a` with ~5.33ms chunking, ~10.67ms target latency, and active drift guard

- [ ] **Step 1: Update CoralSink initialization and threadWorker buffer parameters**

In `audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp`:
1. In `AudioPassthruPrivate::init()` (around line 435):
Update `load_cmd` to include `node.latency=256/48000,media.class=Audio/Sink`:
```cpp
	std::string load_cmd = std::string("pactl load-module module-null-sink sink_name=") + kCoralSink
		+ " sink_properties=device.description=Coral,node.latency=256/48000,media.class=Audio/Sink 2>/dev/null";
```

2. In `AudioPassthruPrivate::threadWorker()` (around lines 531-570):
Replace buffer attributes and stream setup with low-latency settings:
```cpp
    // Fine-tuned low-latency buffer attributes
    // 256 samples (~5.33ms) capture fragment / 512 samples (~10.67ms) playback target
    pa_buffer_attr attr_read;
    attr_read.maxlength = pa_usec_to_bytes(40000, &ss); // 40ms ceiling to prevent buffer bloat
    attr_read.tlength = (uint32_t)-1;
    attr_read.prebuf = (uint32_t)-1;
    attr_read.minreq = (uint32_t)-1;
    attr_read.fragsize = pa_usec_to_bytes(5333, &ss);   // 256 samples

    pa_buffer_attr attr_write;
    attr_write.maxlength = pa_usec_to_bytes(40000, &ss); // 40ms ceiling
    attr_write.tlength = pa_usec_to_bytes(10666, &ss);  // 512 samples (~10.67ms)
    attr_write.prebuf = pa_usec_to_bytes(5333, &ss);    // 256 samples prebuffering
    attr_write.minreq = pa_usec_to_bytes(5333, &ss);    // 256 samples min request
    attr_write.fragsize = (uint32_t)-1;

    int error = 0;

    usleep(100000);

    std::string monitor = std::string(kCoralSink) + ".monitor";
    pa_simple *s_read = pa_simple_new(NULL, "Coral", PA_STREAM_RECORD, monitor.c_str(), "Capture", &ss, NULL, &attr_read, &error);
    if (!s_read) {
        std::cerr << "Failed to create read connection: " << pa_strerror(error) << std::endl;
        return 1;
    }

    const int num_samples = 256; // 256 samples = ~5.33ms at 48kHz
    const int buf_size = num_samples * 2 * sizeof(float);
    float *input_buffer = (float*)malloc(buf_size);
    float *output_buffer = (float*)malloc(buf_size);

    memset(input_buffer, 0, buf_size);
    memset(output_buffer, 0, buf_size);

    bool format_initialized = false;
    std::string current_hw_sink = "";
    pa_simple *s_write = nullptr;

    auto open_write_stream = [&](const std::string& target_sink_name) -> pa_simple* {
        int err = 0;
        const char* target = (!target_sink_name.empty()) ? target_sink_name.c_str() : NULL;
        pa_simple *w = pa_simple_new(NULL, "Coral", PA_STREAM_PLAYBACK, target, "Playback", &ss, NULL, &attr_write, &err);
        if (!w && target != NULL) {
            w = pa_simple_new(NULL, "Coral", PA_STREAM_PLAYBACK, NULL, "Playback", &ss, NULL, &attr_write, &err);
        }
        return w;
    };
```

3. In `AudioPassthruPrivate::threadWorker()` (inside the processing loop around line 710):
Add the active drift guard:
```cpp
    uint64_t drift_check_counter = 0;
    uint64_t last_flush_iter = 0;

    while (!i_kill_processing_thread_) {
        // ... (existing route update from watch thread) ...

        // ... (existing pa_simple_read, dsp processing, soft-knee limiter) ...

        // Write processed audio to hardware
        if (s_write) {
            if (pa_simple_write(s_write, output_buffer, buf_size, &error) < 0) {
                // If write fails (e.g. device disconnected / unplugged), trigger reconnect on watch thread
                pa_simple_free(s_write);
                s_write = nullptr;
                current_hw_sink = "";
                need_reconnect.store(true, std::memory_order_release);
            } else {
                // Active Drift Guard: evaluate latency every 200 iterations (~1.06s)
                if (++drift_check_counter % 200 == 0) {
                    int lat_err = 0;
                    pa_usec_t lat = pa_simple_get_latency(s_write, &lat_err);
                    if (lat_err == 0 && lat != (pa_usec_t)-1) {
                        uint64_t threshold = (current_hw_sink.find("bluez") != std::string::npos) ? 140000ULL : 50000ULL;
                        if (lat > threshold && (drift_check_counter >= last_flush_iter + 600)) {
                            // Buffer bloat detected: flush stale queued backlog to restore real-time sync
                            int flush_err = 0;
                            pa_simple_flush(s_write, &flush_err);
                            last_flush_iter = drift_check_counter;
                        }
                    }
                }
            }
        }
    }
```

- [ ] **Step 2: Build `audiopassthru` and update `libcombined.a`**

Run:
```bash
make -C audiopassthru clean && make -C audiopassthru
echo -e "CREATE libcombined.a\nADDLIB audiopassthru/libaudiopassthru.a\nADDLIB dsp/libdfxdsp.a\nSAVE\nEND" | ar -M
```
Expected: `libaudiopassthru.a` and `libcombined.a` compiled and updated with 0 errors.

- [ ] **Step 3: Commit**

```bash
git add audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp libcombined.a
git commit -m "feat(audio): optimize pipeline to low latency and add active drift guard"
```

---

### Task 3: Build Coral Binary, Package, and Empirically Verify on Bluetooth

**Files:**
- Rebuild: `coral/Builds/LinuxMakefile/build/Coral`
- Package: `packaging/build-deb.sh`
- Verify: Runtime audio stream on Bluetooth headphones (`bluez_output.*` / L80PRO)

**Interfaces:**
- Consumes: `libcombined.a`, `coral` source
- Produces: Installed Coral binary with low-latency audio passthru verified on Bluetooth

- [ ] **Step 1: Build Coral Release executable**

Run:
```bash
make -C coral/Builds/LinuxMakefile -j"$(nproc)" CONFIG=Release
```
Expected: `coral/Builds/LinuxMakefile/build/Coral` compiled successfully with 0 errors.

- [ ] **Step 2: Package and install updated .deb**

Run:
```bash
CORAL_REVISION=7 ./packaging/build-deb.sh
sudo dpkg -i coral_1.2.12-7_amd64.deb || sudo apt-get install -f -y ./coral_1.2.12-7_amd64.deb
```
Expected: Package installed and `/usr/bin/coral` updated.

- [ ] **Step 3: Restart Coral and verify on Bluetooth headphones**

Run:
```bash
pkill -x coral || true
sleep 1
/usr/bin/coral --run_minimized &
sleep 2
```
Expected: Coral running cleanly.

- [ ] **Step 4: Check PipeWire latency metrics and error count**

Run:
```bash
python3 -c "
import subprocess, json
data = json.loads(subprocess.check_output(['pw-dump']).decode('utf-8'))
for item in data:
    props = item.get('info', {}).get('props', {})
    if 'Coral' in props.get('application.name', '') or props.get('node.name') == 'CoralSink':
        print(f'Node {item[\"id\"]} ({props.get(\"node.name\")}, {props.get(\"media.class\")}):')
        params = item.get('info', {}).get('params', {})
        if 'Latency' in params:
            print('  Latency:', params['Latency'])
"
pw-top -b -n 3
```
Expected:
1. `CoralSink` has low quantum (256).
2. Coral playback node internal latency dropped to $\le 25\text{ms}$.
3. `pw-top` shows `ERR == 0` on Coral and `bluez_output`.

- [ ] **Step 5: Verify clean git status**

Run:
```bash
git status
```
Expected: Working tree clean.
