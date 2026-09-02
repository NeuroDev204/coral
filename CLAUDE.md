# CLAUDE.md

Guidance for agents working in this repository.

## Project overview

Coral is a Linux system-wide audio enhancer. It creates a PulseAudio/PipeWire virtual sink, runs DSP (EQ and effects), and plays processed audio to the selected output device. The UI is JUCE; the tray uses AppIndicator.

## Architecture

1. **GUI** (`coral/`) — JUCE application (Simple/Pro views, settings, presets).
2. **Audiopassthru** (`audiopassthru/`) — virtual sink, capture, playback, device follow. Treat changes as high-risk: they affect system audio.
3. **DSP** (`dsp/`) — real-time audio processing. No allocations, locks, or blocking in the audio callback.

## Build

- Linux Makefile: `coral/Builds/LinuxMakefile`
- JUCE 7.0.5 via `JUCE_PATH` or `./JUCE`
- Debian package: `./packaging/build-deb.sh`

## Review priorities

- Flag `audiopassthru/` and `dsp/` changes as higher-risk.
- In `dsp/`, reject anything that can block the real-time thread.
- Do not invent file paths or API names — read the source first.
