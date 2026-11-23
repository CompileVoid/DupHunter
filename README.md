# DupHunter

[![CI](https://github.com/CompileVoid/DupHunter/actions/workflows/ci.yml/badge.svg)](https://github.com/CompileVoid/DupHunter/actions/workflows/ci.yml)

Disk usage analyzer & duplicate file finder. C++23 core, CLI first, Qt GUI planned.

## Status

- [x] M0: Walking skeleton (CMake, vcpkg, GoogleTest, CI)
- [x] M1: Single-threaded filesystem scanner with error handling
- [ ] M2: Thread pool + parallel scanning
- [ ] M3: Deduplication pipeline (size → partial hash → full hash)
- [ ] M4: SQLite cache for incremental rescans
- [ ] M5: Qt GUI

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev