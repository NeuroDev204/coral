# Coral

Linux system-wide audio enhancer. Coral creates a virtual sink, processes desktop audio (EQ, ambience, surround, bass, dynamic boost), and plays it back on your headphones or speakers.

This is a Linux port of [FxSound](https://github.com/fxsound2/fxsound-app) (AGPL-3.0).

## Install (Ubuntu / Debian amd64)

Download `coral_1.2.12-1_amd64.deb` from [Releases](https://github.com/NeuroDev204/coral/releases) and install with apt so libraries are pulled in automatically:

```bash
sudo apt install ./coral_1.2.12-1_amd64.deb
```

Then open **Coral** from the app menu, or click the tray icon. Close / minimize hides to the tray.

GNOME: enable an AppIndicator extension so the tray icon is visible.

## What the package installs

- `/usr/bin/coral`
- Desktop launcher + login autostart
- Icons (hicolor + pixmaps)
- Factory presets (`/usr/share/coral/Factsoft`)
- Dependencies via apt: GTK 3, PulseAudio/PipeWire, OpenGL, AppIndicator, `pactl`

## Build from source

Needs JUCE **7.0.5** and the usual GTK / Pulse development packages:

```bash
sudo apt install build-essential pkg-config libgtk-3-dev libpulse-dev \
  libfreetype6-dev libgl1-mesa-dev libx11-dev libcurl4-openssl-dev \
  libwebkit2gtk-4.0-dev libayatana-appindicator3-dev

git clone --branch 7.0.5 --depth 1 https://github.com/juce-framework/JUCE.git JUCE
make -C coral/Builds/LinuxMakefile -j"$(nproc)" CONFIG=Release
```

If JUCE is not in `./JUCE`, set `JUCE_PATH`:

```bash
make -C coral/Builds/LinuxMakefile CONFIG=Release JUCE_PATH=/path/to/JUCE
```

Package a `.deb`:

```bash
./packaging/build-deb.sh
```

## License

GNU Affero General Public License v3.0. See [LICENSE](LICENSE).
