# Coral

Linux system-wide audio enhancer. Coral creates a virtual sink, processes desktop audio (EQ, ambience, surround, bass, dynamic boost), and plays it back with high fidelity and ultra-low latency on your headphones or speakers.

## Features

- **Ultra-Low Latency Pipeline**: Tuned buffer configuration (512 samples / ~10.67ms chunks at 48kHz) delivering ~35–42ms end-to-end latency — well below the ITU ±45ms threshold for zero perceptible video lip-sync delay.
- **Native 32-bit Float Processing**: Runs end-to-end as `float32le` 2ch 48000Hz, matching native PipeWire audio server architecture and eliminating integer rounding artifacts.
- **Transparent Soft-Knee Limiter**: Smooth $C^1$-continuous saturation curve keeps output bounded within $[-0.999, +0.999]$, eliminating clipping and distortion even under heavy Bass Boost at maximum volume.
- **Bluetooth & ALSA Drift Protection**: Hard 60ms server-side buffer ceiling (`maxlength = 23040` bytes) guarantees zero latency accumulation or clock drift during long listening sessions, without any real-time IPC overhead.
- **Real-Time Thread Safety**: Audio processing loop operates with zero dynamic heap allocations (`malloc`/`free`), zero blocking mutex locks, and zero synchronous system calls. Audio thread is prioritized with `SCHED_RR`.
- **Dynamic Device Switching**: Automatically detects hardware sink changes and headphone hotplugging, seamlessly redirecting the audio stream without dropouts.
- **System Tray & Autostart**: Runs minimized in the background with AppIndicator tray support, launches automatically on login.

## Install (Ubuntu / Debian amd64)

Download the latest `.deb` from [Releases](https://github.com/NeuroDev204/coral/releases/latest) and install:

```bash
sudo apt install ./coral_1.2.12-8_amd64.deb
```

No extra setup required. Coral starts immediately after install, pulls in required runtime dependencies via apt, and auto-starts on login.

> **Note**: An apt note about an unsandboxed local file is normal for a `.deb` installed from your Downloads folder — it does not mean installation failed.

Open **Coral** from your application menu or access it via the tray icon. Closing or minimizing the window keeps it running in the tray.

*GNOME Users*: Ensure an AppIndicator extension (e.g. `gnome-shell-extension-appindicator`) is enabled to view the tray icon.

## What the Package Installs

- `/usr/bin/coral` — Main executable
- Desktop launcher (`/usr/share/applications/coral.desktop`) + autostart (`/etc/xdg/autostart/coral.desktop`)
- High-resolution application icons (hicolor scalable + raster pixmaps)
- Standard factory presets (`/usr/share/coral/Factsoft`)
- Runtime dependencies automatically installed via apt: GTK 3, PulseAudio/PipeWire, OpenGL, AppIndicator, `pulseaudio-utils` (`pactl`)

## Build from Source

Requirements: JUCE **7.0.5** and development libraries for GTK 3, PulseAudio, and OpenGL:

```bash
sudo apt install build-essential pkg-config libgtk-3-dev libpulse-dev \
  libfreetype6-dev libgl1-mesa-dev libx11-dev libcurl4-openssl-dev \
  libwebkit2gtk-4.0-dev libayatana-appindicator3-dev

# Clone JUCE 7.0.5 if not already present
git clone --branch 7.0.5 --depth 1 https://github.com/juce-framework/JUCE.git JUCE

# Build Release binary
make -C coral/Builds/LinuxMakefile -j"$(nproc)" CONFIG=Release JUCE_PATH=/path/to/JUCE
```

To build an installable Debian package:

```bash
./packaging/build-deb.sh
```

## License

GNU Affero General Public License v3.0. See [LICENSE](LICENSE).
