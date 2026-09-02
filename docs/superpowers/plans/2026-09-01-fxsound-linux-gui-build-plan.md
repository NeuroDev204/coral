# FxSound Linux GUI Build - Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development

**Goal:** Build the JUCE GUI application for Linux

**Architecture:** Export JUCE project to LinuxMakefile, build with GCC, verify executable works

**Tech Stack:** C++17, GCC, JUCE 6.1.6, GTK3

**Spec:** `docs/superpowers/specs/2026-09-01-fxsound-linux-port-design.md`

## Global Constraints
- C++17 standard
- Use existing Makefile in `fxsound/Builds/LinuxMakefile/`
- Link against `libdfxdsp.a` and `libaudiopassthru.a`

---

## Task 1: Verify Libraries are in Place

**Files:** None

- [ ] Verify DSP library: `ls -lh dsp/libdfxdsp.a`
- [ ] Verify AudioPassthru library: `ls -lh audiopassthru/libaudiopassthru.a`
- [ ] Copy to build directory if needed

---

## Task 2: Export/Update LinuxMakefile

**Files:**
- Modify: `fxsound/FxSound.jucer`

**Goal:** Ensure Linux export is up to date

- [ ] Check if LinuxMakefile Makefile exists
- [ ] If needed, use Projucer to re-export: `../../JUCE/Projucer --resave FxSound.jucer`
- [ ] Verify Makefile has correct library paths

---

## Task 3: Build GUI Application

**Files:** None

- [ ] `cd fxsound/Builds/LinuxMakefile`
- [ ] `make clean`
- [ ] `make -j$(nproc) CONFIG=Release 2>&1 | tee build.log`
- [ ] Check for errors

---

## Task 4: Fix Linking Errors

**Files:** Various as needed

**Goal:** Resolve any undefined reference errors

- [ ] If missing symbols: add library paths or link order
- [ ] If compile errors: fix source files
- [ ] Rebuild

---

## Task 5: Verify Executable

**Files:** None

- [ ] Check executable exists: `ls -lh fxsound/Builds/LinuxMakefile/build/FxSound`
- [ ] Verify it's executable: `file fxsound/Builds/LinuxMakefile/build/FxSound`
- [ ] Test run (may fail due to no display - check error message)

---

## Task 6: Copy Libraries for Runtime

**Files:** None

- [ ] Copy `libdfxdsp.a` to build directory
- [ ] Copy `libaudiopassthru.a` to build directory
- [ ] Commit build artifacts

---

## Success Criteria
- [ ] `FxSound` executable builds without errors
- [ ] Executable is valid ELF binary
- [ ] Libraries linked correctly
