# DupHunter

[![CI](https://github.com/CompileVoid/DupHunter/actions/workflows/ci.yml/badge.svg)](https://github.com/CompileVoid/DupHunter/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A fast, cross-platform disk usage analyzer and duplicate file finder written in modern C++23.
CLI-first design, Qt-based GUI planned.

## ✨ Features

- **Three-stage parallel deduplication pipeline** — size → partial hash (FNV-1a, 4 KB) → full hash. Hashing is parallelized via custom ThreadPool for maximum I/O and CPU utilization.
- **Parallel filesystem scanner** — custom thread pool with `std::jthread` and C++20 `std::stop_token` for graceful cancellation.
- **Smart error handling** — skips inaccessible directories without aborting the whole scan.
- **Rich CLI output** — colored output, human-readable sizes, per-group breakdown.
- **Filtering** — `--min-size`, `--exclude` patterns, `--no-color`.
- **Cross-platform** — builds and runs on Windows (MSVC / Clang), Linux (GCC/Clang), macOS.
- **Modern C++** — C++23, `-Wall -Wextra -Wshadow -Wconversion -Werror`, GoogleTest, ASan/UBSan in CI.

## 📊 Status

- [x] **M0** — Walking skeleton (CMake, vcpkg, GoogleTest, CI matrix)
- [x] **M1** — Single-threaded filesystem scanner with error handling
- [x] **M2** — Thread pool + parallel scanning with `std::stop_token` cancellation
- [x] **M3** — Deduplication pipeline (size → partial hash → full hash)
- [x] **M3.5** — CLI polish: colored output, size filters, human-readable sizes
- [ ] **M4** — SQLite cache for incremental rescans
- [ ] **M5** — Qt GUI

## 🚀 Build

Requirements: CMake 3.24+, C++23 compiler, [vcpkg](https://vcpkg.io).

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev          
```

To run with sanitizers (Linux/macOS):

```bash
cmake --preset sanitize
cmake --build --preset sanitize
ctest --preset sanitize
```

## 💻 Usage

```bash
duphunter <path> [options]
```

### Options

| Option | Description |
| --- | --- |
| `--min-size <size>` | Skip files smaller than size. Supports suffixes: `B`, `KB`, `MB`, `GB` (e.g. `--min-size 1MB`) |
| `--exclude <patterns>` | Comma-separated list of substrings to exclude from results (e.g. `--exclude .git,cmake-build-debug,vcpkg_installed`) |
| `--no-color` | Disable ANSI colored output |
| `-h, --help` | Show help message |

### Examples

Scan a directory and find all duplicates:

```bash
duphunter /home/user/Photos
```

Find only duplicates larger than 1 MB, excluding build and VCS directories:

```bash
duphunter С:\Projects --min-size 1MB --exclude .git,cmake-build-debug,vcpkg_installed
```

### Sample output

```
Scanning /home/user/Photos...
Found 14382 files (48.23 GB)
2 access errors

=== RESULTS ===
Found 47 duplicate groups (312 files, 2.14 GB potential savings)

--- Group 1 (4 files, 8.42 MB each, waste: 25.26 MB) ---
  [KEEP] /home/user/Photos/vacation/IMG_001.jpg
  [DUP]  /home/user/Photos/backup/IMG_001 (copy).jpg
  [DUP]  /home/user/Downloads/IMG_001.jpg
  [DUP]  /home/user/Desktop/IMG_001 - Copy.jpg

...

=== SUMMARY ===
Total files scanned: 14382
Duplicate groups:    47
Total duplicates:    312
Potential savings:   2.14 GB

Time elapsed: 924 ms
```

## 🏗️ Architecture

```
┌────────────────────────────────────────────────────┐
│                   CLI / (Qt GUI)                   │
└──────────────────────┬─────────────────────────────┘
                       │
                       ▼
┌────────────────────────────────────────────────────┐
│              Scanner (ThreadPool)                  │
│  recursive_directory_iterator → task queue → pool  │
└──────────────────────┬─────────────────────────────┘
                       │ ScanResult
                       ▼
┌────────────────────────────────────────────────────┐
│             Deduplication pipeline                 │
│  1. Group by size        → ~10% remain             │
│  2. Partial hash (4 KB)  → ~1%  remain             │
│  3. Full hash (FNV-1a)   → final groups            │
└──────────────────────┬─────────────────────────────┘
                       │
                       ▼
                DuplicateGroup list
```

## 🛣️ Roadmap

- [ ] **M4** — Incremental scans via SQLite (track mtime + size, skip unchanged files)
- [ ] **M5** — Qt 6 GUI with tree view, progress bar, deletion actions
- [ ] xxHash3 integration for ~10× faster hashing on large files
- [ ] Interactive deletion / symlink / hardlink creation