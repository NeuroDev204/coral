# FxSound Linux Port - Technical Design Specification

**Date:** 2026-09-01
**Status:** Draft
**Author:** Claude

## 1. Project Overview

### 1.1 Goal
Port FxSound audio enhancement application from Windows to Linux with full feature parity:
- System-wide audio processing via virtual audio driver
- DSP effects (Fidelity, Ambience, Surround, Dynamic Boost, Bass)
- Equalizer with 10 bands
- Preset management
- System tray for background operation
- Output device selection

### 1.2 Scope
| Component | Windows (Source) | Linux (Target) |
|-----------|------------------|----------------|
| DSP Engine | C++/ptutil | C++/ptutil (reuse) |
| Audio Routing | WASAPI/Core Audio | PulseAudio/PipeWire |
| GUI | JUCE 6.1.6 | JUCE 6.1.6 |
| System Tray | Windows NotifyIcon | System tray via JUCE/GTK |
| Build | Visual Studio 2022 | CMake/Makefile |

---

## 2. Architecture

### 2.1 Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        FxSound GUI (JUCE)                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  Main View  │  │ System Tray │  │  Output Preference     │  │
│  │  (Presets)  │  │   (Tray)   │  │  (Device Selection)    │  │
│  └──────┬──────┘  └──────┬──────┘  └───────────┬─────────────┘  │
│         │                 │                     │                │
│         └────────────────┬┴─────────────────────┘                │
│                          │                                      │
│                   ┌──────▼──────┐                               │
│                   │ FxController │                               │
│                   └──────┬──────┘                               │
└──────────────────────────┼────────────────────────────────────────┘
                          │
           ┌───────────────┼───────────────┐
           │               │               │
    ┌──────▼──────┐ ┌──────▼──────┐ ┌──────▼──────┐
    │ AudioPassthru│ │   DfxDsp    │ │   Settings  │
    │  (Linux)    │ │   Engine    │ │  (JSON)     │
    └──────┬──────┘ └──────┬──────┘ └─────────────┘
           │               │
           │               │
    ┌──────▼───────────────▼──────┐
    │     PulseAudio/PipeWire      │
    │  (System Audio Abstraction)   │
    └──────────────────────────────┘
```

### 2.2 Audio Signal Flow (Linux)

```
┌──────────────────────────────────────────────────────────────────┐
│                     PulseAudio/PipeWire                           │
│                                                                  │
│   Application ──► System Output ──► Null Sink ──► Monitor ──► DSP │
│                                       │                          │
│   Hardware ◄──────────────────────────┘                          │
│   (Headphones/Speakers) ◄──── Processed Audio                    │
└──────────────────────────────────────────────────────────────────┘

Components:
1. Null Sink (FxSoundSink): Virtual sink that captures all system audio
2. Monitor Source: Captures the raw audio from applications
3. DSP Processing: Applies FxSound effects
4. Output: Sends processed audio to actual hardware
```

---

## 3. Technical Implementation

### 3.1 Build System

**Toolchain:**
- Compiler: GCC 11+ / Clang 14+
- C++ Standard: C++17
- Build System: CMake + Make
- JUCE: 6.1.6 (already present in `linux/JUCE`)

**Directory Structure:**
```
fxsound-app/
├── audiopassthru/          # Audio routing layer
│   ├── include/            # Headers
│   ├── src/               # Source files
│   └── Makefile           # Build script
├── dsp/                    # DSP engine
│   ├── include/            # Headers
│   ├── ptutil/            # DSP utilities
│   ├── ptechDsp/          # DSP algorithms
│   └── Makefile           # Build script
├── fxsound/                # JUCE GUI
│   ├── Source/            # C++ source
│   ├── Builds/            # Build outputs
│   └── FxSound.jucer     # JUCE project
└── Resources/              # Fonts, images, strings
```

### 3.2 Cross-Platform Header Fixes

**Priority 1: pt_defs.h**
```cpp
// Fix: Add Linux definitions for Windows types
#ifndef __ANDROID__
#if defined(_WIN32)
#include <windows.h>
#endif
#endif

// Add for Linux:
#if !defined(_WIN32) && !defined(__ANDROID__)
#ifndef WPARAM
typedef uintptr_t WPARAM;
#endif
#ifndef LPARAM
typedef intptr_t LPARAM;
#endif
#ifndef HWND
typedef void* HWND;
#endif
#endif
```

**Priority 2: codedefs.h**
```cpp
// Ensure realtype is defined before use
#ifndef realtype
#define realtype float
#endif

// Fix WPARAM/LPARAM usage for Linux
#if !defined(_WIN32)
#ifndef WPARAM
typedef uintptr_t WPARAM;
#endif
#ifndef LPARAM
typedef intptr_t LPARAM;
#endif
#endif
```

### 3.3 DSP Library Build

**Makefile target:**
```makefile
# dsp/Makefile
libdfxdsp.a: $(OBJS)
	ar rcs $@ $^
	# Create shared library for dynamic loading
	g++ -shared -fPIC -o libdfxdsp.so $(OBJS)
```

### 3.4 Audio Passthrough Layer

**Implementation: PulseAudio with fallback to PipeWire**

Key files:
- `audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp` (exists)
- `audiopassthru/src/AudioPassthru/LinuxShim.cpp` (exists)

**Audio Flow:**
```cpp
// 1. Create null sink for capturing system audio
system("pactl load-module module-null-sink sink_name=FxSoundSink");

// 2. Capture from monitor source
pa_simple *capture = pa_simple_new(NULL, "FxSound",
    PA_STREAM_RECORD, "FxSoundSink.monitor", "capture", &ss, NULL, NULL, &error);

// 3. Process with DSP
dfx_dsp->processAudio(input, output, num_samples, 0);

// 4. Output to hardware
pa_simple *playback = pa_simple_new(NULL, "FxSound",
    PA_STREAM_PLAYBACK, hw_sink_name, "playback", &ss, NULL, NULL, &error);
```

### 3.5 System Tray Implementation

**Option A: JUCE + GTK (Recommended)**
```cpp
// FxSystemTrayView.cpp - Linux implementation
#if JUCE_LINUX
#include <gtk/gtk.h>

class FxSystemTrayView {
    GtkStatusIcon* tray_icon;
    GtkMenu* tray_menu;
    
    void createTrayIcon() {
        gtk_init(nullptr, nullptr);
        tray_icon = gtk_status_icon_new();
        gtk_status_icon_set_from_icon_name(tray_icon, "audio-volume-high");
        gtk_status_icon_set_tooltip_text(tray_icon, "FxSound");
        
        // Create menu
        tray_menu = gtk_menu_new();
        // Add items: Show, Presets submenu, Exit
        
        g_signal_connect(tray_icon, "activate", 
            G_CALLBACK(onTrayActivated), this);
        g_signal_connect(tray_icon, "popup-menu",
            G_CALLBACK(onTrayPopup), this);
    }
};
#endif
```

**Option B: libappindicator (Ubuntu/Debian)**
```cpp
#include <libappindicator3-1/libappindicator.h>

AppIndicator* indicator = app_indicator_new(
    "fxsound", "audio-volume-high", 
    APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
```

### 3.6 Settings Storage

**JSON-based configuration** (replacing Windows Registry):
```json
// ~/.config/fxsound/settings.json
{
    "version": "1.2.12.0",
    "audio": {
        "outputDevice": "auto",
        "bufferSize": 1024,
        "sampleRate": 44100
    },
    "effects": {
        "power": true,
        "fidelity": 50,
        "ambience": 30,
        "surround": 40,
        "dynamicBoost": 20,
        "bass": 35
    },
    "equalizer": {
        "enabled": true,
        "bands": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    },
    "balance": 0.0,
    "volumeLeveling": 0.0
}
```

---

## 4. Build Instructions

### 4.1 Prerequisites

```bash
# Install dependencies
sudo apt install \
    build-essential \
    cmake \
    pkg-config \
    libpulse-dev \
    libgtk-3-dev \
    libayatana-appindicator3-dev \
    juce-tools
```

### 4.2 Build Steps

```bash
# 1. Build DSP library
cd fxsound-app/dsp
make clean
make -j$(nproc)

# 2. Build Audio Passthrough library
cd ../audiopassthru
make clean
make -j$(nproc)

# 3. Export JUCE project for Linux
cd ../fxsound
# Use Projucer to export to LinuxMakefile
/home/neuro/Documents/Tools/fxsound-linux/linux/JUCE/Projucer \
    --resave FxSound.jucer

# 4. Build GUI
cd Builds/LinuxMakefile
make -j$(nproc) CONFIG=Release

# 5. Install
sudo cp build/FxSound /usr/local/bin/
sudo cp -r ../Resources /usr/local/share/fxsound/
```

### 4.3 Runtime Setup

```bash
# Create user directories
mkdir -p ~/.config/fxsound
mkdir -p ~/.local/share/fxsound/presets

# Copy default presets
cp /usr/local/share/fxsound/presets/*.fac \
   ~/.local/share/fxsound/presets/

# Run
fxsound
```

---

## 5. File Changes Required

### 5.1 Headers to Fix

| File | Issue | Fix |
|------|-------|-----|
| `dsp/ptutil/include/spectrum.h` | WPARAM/LPARAM undefined | Add Linux typedefs |
| `audiopassthru/include/pt_defs.h` | Nested _WIN32 ifdefs | Restructure for Linux |
| `audiopassthru/include/codedefs.h` | realtype used before define | Reorder includes |
| `dsp/ptutil/dfxp/*.cpp` | Windows types | Add compatibility layer |

### 5.2 New Files

| File | Purpose |
|------|---------|
| `audiopassthru/include/linux_compat.h` | Linux compatibility definitions |
| `fxsound/Source/SystemTrayLinux.cpp` | GTK system tray implementation |

### 5.3 Modified Files

| File | Change |
|------|--------|
| `audiopassthru/src/AudioPassthru/AudioPassthruLinux.cpp` | Improve device detection |
| `fxsound/Source/GUI/FxSystemTrayView.cpp` | Add Linux tray support |
| `fxsound/FxSound.jucer` | Update Linux build settings |

---

## 6. Testing Plan

### 6.1 Unit Tests
- DSP processing correctness
- Preset save/load
- Settings serialization

### 6.2 Integration Tests
- Audio pipeline: capture → process → playback
- Device switching
- System tray functionality

### 6.3 User Acceptance Tests
- [ ] Application starts without errors
- [ ] System tray icon appears
- [ ] Audio enhancement is audible
- [ ] Presets can be switched
- [ ] Output device can be changed
- [ ] Application minimizes to tray
- [ ] Background operation works

---

## 7. Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| JUCE system tray Linux support limited | Medium | Use GTK StatusIcon or libappindicator |
| Audio latency issues | High | Buffer size tuning, async processing |
| Cross-compiler ABI issues | Medium | Consistent C++17, clean rebuild |
| PulseAudio not available | Low | Fallback to PipeWire via PA compatibility layer |

---

## 8. Success Criteria

The Linux port is complete when:
1. Application builds without errors
2. Application starts and displays main window
3. System tray icon is visible and functional
4. Audio is captured, processed, and played back
5. All DSP effects (Fidelity, Ambience, Surround, Dynamic Boost, Bass) work
6. Equalizer controls work
7. Presets can be saved, loaded, and switched
8. Output device selection works
9. Application runs in background via system tray
10. No audio glitches or significant latency increase

---

## 9. Implementation Phases

### Phase 1: Build Fixes (Foundation)
- Fix cross-platform header issues
- Ensure DSP library compiles
- Ensure audiopassthru library compiles
- Verify GUI builds

### Phase 2: Audio Pipeline
- Verify PulseAudio null-sink routing
- Test capture and playback
- Verify DSP processing in audio path
- Optimize buffer sizes

### Phase 3: GUI Features
- Implement system tray for Linux
- Verify all controls work
- Test preset management
- Device selection UI

### Phase 4: Polish
- Error handling improvements
- Logging and diagnostics
- Performance optimization
- Documentation
