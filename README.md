# TestCpp

[![CI](https://github.com/AdelAhmetgaliev/TestCpp/actions/workflows/ci.yml/badge.svg)](https://github.com/AdelAhmetgaliev/TestCpp/actions/workflows/ci.yml)

A minimal, opinionated template for modern C++ projects: C++23, CMake ≥ 3.26
with presets, strict warnings, security hardening, sanitizers, clang-tidy and
clang-format wired in.

## Requirements

| Tool         | Version                                |
| ------------ | -------------------------------------- |
| CMake        | ≥ 3.26                                 |
| Ninja        | any recent                             |
| GCC          | ≥ 14 (`<print>` support)               |
| Clang        | ≥ 18 (libc++/libstdc++ with `<print>`) |
| clang-format | ≥ 19                                   |
| clang-tidy   | ≥ 17 recommended                       |

## Quick start

```sh
cmake --workflow --preset clang-debug     # configure + build (+ ctest)
./build/clang-debug/bin/TestCpp_exe
```

All builds go to `build/<presetName>/`, binaries to `build/<presetName>/bin/`.

## Presets

| Preset           | Purpose                                            |
| ---------------- | -------------------------------------------------- |
| `clang-debug`    | Main development line (Clang, Debug)               |
| `clang-tidy`     | Debug + clang-tidy runs on every build             |
| `clang-sanitize` | Debug + ASan/UBSan (abort on UB)                   |
| `clang-release`  | Release with LTO                                   |
| `gcc-*`          | Same matrix on GCC for portability checking        |
| `gcc-reldebinfo` | Release with debug info (profiling/debugging)      |

Each preset has matching build/test/workflow presets, so the common loop is:

```sh
cmake --workflow --preset <name>
```

## Options

| Option             | Default | Description                     |
| ------------------ | ------- | ------------------------------- |
| `ENABLE_LTO`       | `ON`    | Link Time Optimization (Release/RelWithDebInfo) |
| `ENABLE_STRICT_WARNINGS` | `ON` | Strict warning set (`-Wall -Wextra -Wpedantic -Wconversion -Wshadow …`, MSVC `/W4 /WX`) |
| `ENABLE_SANITIZERS` | `OFF`  | ASan + UBSan (with `-fno-sanitize-recover=undefined`) |
| `ENABLE_CLANG_TIDY` | `OFF`  | clang-tidy custom target run on every build |
| `ENABLE_TESTS`      | `ON`   | Build tests (Catch2 v3 via FetchContent) and register them with CTest |

Example:

```sh
cmake --preset clang-debug -DENABLE_SANITIZERS=ON
```

The first configure with `ENABLE_TESTS=ON` downloads Catch2 into
`build/<preset>/_deps` once; an installed system Catch2 is used instead when
found. Set `ENABLE_TESTS=OFF` for a fully offline, dependency-free build.

## Project layout

```
source/
├── app/        # executables (thin entry points)
│   └── main.cpp
└── lib/        # library code (${ProjectName}::lib)
    ├── greeter.hpp
    └── greeter.cpp
tests/          # Catch2 tests (${ProjectName}_tests, run via CTest)
```

Quality-of-life targets are INTERFACE libraries attached to
`${ProjectName}::lib` and inherited by consumers:
`${ProjectName}::warnings`, `${ProjectName}::hardening`,
`${ProjectName}::sanitizers`.

## Formatting and linting

```sh
./scripts/format.sh          # format all tracked sources
./scripts/format.sh --check  # CI mode: fail if not formatted
cmake --workflow --preset clang-tidy
```

Formatting requires clang-format ≥ 19; the `.clang-format` uses keys
introduced in v17–v19.

## CI

`.github/workflows/ci.yml` runs on every push/PR:

- `format-check` — `scripts/format.sh --check`
- `clang-tidy` — full tidy pass via the `clang-tidy` preset
- `build` — matrix over clang/gcc × debug/sanitize/release workflow presets

The install step pins LLVM 19 and GCC 14 toolchains via apt and symlinks them
to plain `clang++`/`g++`, matching what the presets invoke.

## Renaming the template

All CMake identifiers derive from the `project()` name, so a rename is a
single command:

```sh
./scripts/rename.sh MyProject     # add --dry-run to preview
rm -rf build/*                    # old caches reference stale target names
```

The script rewrites targets/aliases (`MyProject::lib`, …), the debug macro
(`MYPROJECT_DEBUG`) and the C++ namespace in one pass. Rename the repository
directory yourself afterwards.

Note: cache variables are intentionally unprefixed (`ENABLE_*`). If you embed
this project into another CMake tree via `add_subdirectory`, consider adding a
prefix back.

## Notes

- Tests use [Catch2 v3](https://github.com/catchorg/Catch2), fetched with a
  pinned tag; the presets' `ctest` steps pick them up automatically.
- No dependency manager is wired up by design beyond that; add
  FetchContent/vcpkg/conan entries when more dependencies appear.
