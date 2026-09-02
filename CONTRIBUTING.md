# Contributing to Coral

- Open issues and pull requests on [github.com/NeuroDev204/coral](https://github.com/NeuroDev204/coral).
- Look for existing issues before filing a new one.
- Keep changes focused. DSP and audio-routing code is high-risk — call that out in the PR.
- Coral GUI C++ follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) where the surrounding file already does.

## Pull requests

1. Fork the repository and create a branch from `main`.
2. Build on Linux: `make -C coral/Builds/LinuxMakefile CONFIG=Debug`.
3. Describe what changed and how you tested it (audio routing, tray, UI).
