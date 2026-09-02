# FxSound Linux Build Fixes - Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix cross-platform header issues and build DSP + audiopassthru libraries for Linux

**Architecture:** Create Linux compatibility layer in headers, then build static libraries that will be linked into the JUCE GUI application.

**Tech Stack:** C++17, GCC, GNU Make, PulseAudio

**Spec:** `docs/superpowers/specs/2026-09-01-fxsound-linux-port-design.md`

## Global Constraints

- C++17 standard
- GCC/Clang compiler
- PulseAudio for audio (libpulse-dev)
- Preserve existing DSP algorithm implementations

---

## Task 1: Create Linux Compatibility Header

**Files:**
- Create: `audiopassthru/include/linux_compat.h`
- Modify: `audiopassthru/include/pt_defs.h`
- Modify: `audiopassthru/include/codedefs.h`

**Interfaces:**
- Produces: `linux_compat.h` - defines WPARAM, LPARAM, HWND, DWORD for Linux

- [ ] **Step 1: Create linux_compat.h**

```cpp
/*
 * Linux compatibility layer for Windows types
 */
#ifndef _LINUX_COMPAT_H_
#define _LINUX_COMPAT_H_

#include <cstdint>

// Windows types that need Linux equivalents
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

#ifndef DWORD
typedef uint32_t DWORD;
#endif

#ifndef HANDLE
typedef void* HANDLE;
#endif

#ifndef HRESULT
typedef int32_t HRESULT;
#endif

#ifndef S_OK
#define S_OK 0
#endif

#ifndef E_FAIL
#define E_FAIL -1
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Windows-like calling conventions (no-op on Linux)
#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef WINAPI
#define WINAPI
#endif

// File seeking
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#endif // !_WIN32 && !__ANDROID__

#endif // _LINUX_COMPAT_H_
```

- [ ] **Step 2: Include linux_compat.h in pt_defs.h**

Modify `audiopassthru/include/pt_defs.h` - add include after guard:

```cpp
#ifndef _PT_DEFS_H_
#define _PT_DEFS_H_

// Linux compatibility layer
#include "linux_compat.h"

#ifndef __ANDROID__
// Rest of file unchanged...
```

- [ ] **Step 3: Include linux_compat.h in codedefs.h**

Modify `audiopassthru/include/codedefs.h` - add include after guard:

```cpp
#ifndef _CODEDEFS_H_
#define _CODEDEFS_H_

// Linux compatibility layer
#include "linux_compat.h"

#ifndef __ANDROID__
```

- [ ] **Step 4: Test compilation of pt_defs.h**

Run: `g++ -c -I. -std=c++17 -fsyntax-only audiopassthru/include/pt_defs.h`
Expected: No errors

- [ ] **Step 5: Test compilation of codedefs.h**

Run: `g++ -c -I. -std=c++17 -fsyntax-only audiopassthru/include/codedefs.h`
Expected: No errors

- [ ] **Step 6: Commit**

```bash
git add audiopassthru/include/linux_compat.h
git add audiopassthru/include/pt_defs.h
git add audiopassthru/include/codedefs.h
git commit -m "fix: add Linux compatibility header for Windows types"
```

---

## Task 2: Fix spectrum.h for Linux

**Files:**
- Modify: `dsp/ptutil/include/spectrum.h`

**Issue:** WPARAM/LPARAM used before proper include order

- [ ] **Step 1: Update spectrum.h include order**

Modify `dsp/ptutil/include/spectrum.h` lines 21-23:

```cpp
// Change from:
#include "pt_defs.h"
#include "codedefs.h"
#include "slout.h"

// To:
#include "codedefs.h"  // Must come before pt_defs.h for realtype
#include "pt_defs.h"  // Contains Linux compat
#include "slout.h"
```

- [ ] **Step 2: Test compilation**

Run: `g++ -c -Iinclude -Iptutil/include -std=c++17 -fsyntax-only ptutil/include/spectrum.h`
Expected: No errors (only warnings about unused functions OK)

- [ ] **Step 3: Commit**

```bash
git add dsp/ptutil/include/spectrum.h
git commit -m "fix: correct include order in spectrum.h for Linux"
```

---

## Task 3: Update DSP Makefile for Static Library

**Files:**
- Modify: `dsp/Makefile`

**Goal:** Ensure libdfxdsp.a is built correctly

- [ ] **Step 1: Verify Makefile targets**

Read current `dsp/Makefile` - confirm it produces `libdfxdsp.a`

- [ ] **Step 2: Build DSP library**

Run:
```bash
cd dsp
make clean
make -j$(nproc)
ls -la libdfxdsp.a
```

Expected: `libdfxdsp.a` created successfully

- [ ] **Step 3: Test archive contents**

Run: `ar -t libdfxdsp.a | head -20`
Expected: List of .o files including DfxDsp.o

- [ ] **Step 4: Commit Makefile if unchanged (no-op)**

If Makefile already correct, skip to next task.

---

## Task 4: Build AudioPassthru Library

**Files:**
- Modify: `audiopassthru/Makefile`

**Goal:** Build libaudiopassthru.a successfully

- [ ] **Step 1: Clean and build**

Run:
```bash
cd audiopassthru
make clean
make -j$(nproc)
```

Expected: Build succeeds with warnings but no errors

- [ ] **Step 2: Verify library created**

Run: `ls -la libaudiopassthru.a`
Expected: File exists and contains objects

- [ ] **Step 3: Verify symbols exported**

Run: `nm libaudiopassthru.a | grep AudioPassthru`
Expected: Symbols for AudioPassthru class

- [ ] **Step 4: Commit**

```bash
git add audiopassthru/Makefile
git commit -m "build: successfully compile audiopassthru library"
```

---

## Task 5: Fix DSP Library Build Issues

**Files:**
- Modify: Various DSP source files as needed

**Issue:** Compilation errors from Task 3 or 4

- [ ] **Step 1: Identify errors**

Run: `cd dsp && make 2>&1 | grep "error:"`
Expected: List of errors (if any)

- [ ] **Step 2: Fix spectrum.h issues**

If spectrum.h errors occur, modify the problematic function declarations:

```cpp
// In spectrumMessageValues.cpp and spectrum.h:
// Replace WPARAM, LPARAM with uintptr_t, intptr_t for Linux
int PT_DECLSPEC spectrumGetBandValuesFromMessageValues_NoHandle(
    uintptr_t wparam, 
    intptr_t lparam, 
    realtype *rp_band_values, 
    int i_num_bands, 
    int *ip_num_values
);
```

- [ ] **Step 3: Fix realtype issues**

If realtype errors occur, ensure `codedefs.h` defines it before any use:

```cpp
// In codedefs.h, ensure realtype is defined early:
#ifndef realtype
#define realtype float
#endif
```

- [ ] **Step 4: Rebuild and verify**

Run: `make -j$(nproc) 2>&1 | grep -E "(error|Error)" | head -10`
Expected: No errors (warnings OK)

- [ ] **Step 5: Commit fixes**

```bash
git add dsp/ptutil/dfxp/*.cpp  # any modified files
git commit -m "fix: resolve DSP compilation issues for Linux"
```

---

## Task 6: Build DSP Library - Final Verification

**Files:**
- None (verification task)

**Goal:** Confirmed working DSP library

- [ ] **Step 1: Clean rebuild**

Run:
```bash
cd dsp
make clean
make -j$(nproc) 2>&1 | tail -20
```

Expected: Build completes with 0 errors

- [ ] **Step 2: Verify all objects compiled**

Run: `ar -t libdfxdsp.a | wc -l`
Expected: > 50 object files

- [ ] **Step 3: Check for DfxDsp symbols**

Run: `nm libdfxdsp.a | grep "DfxDsp::"`
Expected: Constructor, destructor, and method symbols

- [ ] **Step 4: Copy to expected location**

Run: `cp libdfxdsp.a ../fxsound/Builds/LinuxMakefile/build/`
Expected: Copy succeeds

- [ ] **Step 5: Commit DSP build artifacts (if tracked)**

```bash
git add dsp/libdfxdsp.a 2>/dev/null || true
git commit -m "build: complete DSP library for Linux"
```

---

## Task 7: Build AudioPassthru Library - Final Verification

**Files:**
- None (verification task)

**Goal:** Confirmed working AudioPassthru library

- [ ] **Step 1: Clean rebuild**

Run:
```bash
cd audiopassthru
make clean
make -j$(nproc) 2>&1 | tail -20
```

Expected: Build completes with 0 errors

- [ ] **Step 2: Verify symbols**

Run: `nm libaudiopassthru.a | grep -E "(AudioPassthru|DfxDsp)"`
Expected: Symbols for AudioPassthru and references to DfxDsp

- [ ] **Step 3: Copy to expected location**

Run: `cp libaudiopassthru.a ../fxsound/Builds/LinuxMakefile/build/`
Expected: Copy succeeds

- [ ] **Step 4: Commit**

```bash
git commit -m "build: complete audiopassthru library for Linux"
```

---

## Task 8: Update LinuxMakefile for Libraries

**Files:**
- Modify: `fxsound/Builds/LinuxMakefile/Makefile`

**Goal:** Ensure Makefile links to correct library paths

- [ ] **Step 1: Check current LDFLAGS**

Run: `grep "JUCE_LDFLAGS" fxsound/Builds/LinuxMakefile/Makefile | head -5`
Expected: Contains `-L../../../audiopassthru -L../../../dsp`

- [ ] **Step 2: Verify library search paths**

The Makefile should have:
```
-L../../../audiopassthru -L../../../dsp
```

If missing, add to JUCE_LDFLAGS line 48 and 69.

- [ ] **Step 3: Commit if changed**

```bash
git add fxsound/Builds/LinuxMakefile/Makefile
git commit -m "build: update LinuxMakefile for library paths"
```

---

## Task 9: Full Build Test - DSP + AudioPassthru

**Files:**
- None (verification task)

**Goal:** Both libraries build successfully

- [ ] **Step 1: Build both libraries in sequence**

Run:
```bash
cd dsp && make clean && make -j$(nproc) && echo "DSP: OK"
cd ../audiopassthru && make clean && make -j$(nproc) && echo "AudioPassthru: OK"
```

Expected: Both print "OK"

- [ ] **Step 2: Verify file sizes**

Run:
```bash
ls -lh dsp/libdfxdsp.a
ls -lh audiopassthru/libaudiopassthru.a
```

Expected: DSP library > 1MB, AudioPassthru library > 100KB

- [ ] **Step 3: Final commit**

```bash
git add -A
git commit -m "build: complete Phase 1 - DSP and AudioPassthru libraries compile"
```

---

## Success Criteria

Phase 1 is complete when:
- [ ] `dsp/libdfxdsp.a` builds without errors
- [ ] `audiopassthru/libaudiopassthru.a` builds without errors
- [ ] Both libraries contain expected symbols
- [ ] Linux compatibility headers are in place
- [ ] No undefined symbol errors for Windows types (WPARAM, LPARAM, etc.)

---

## Next Phase

After Phase 1 complete, proceed to:
- Phase 2: Build JUCE GUI application
- Phase 3: Audio pipeline testing
- Phase 4: System tray implementation
