# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++17 educational repository implementing all 23 Gang-of-Four (GoF) design patterns. Each pattern is a standalone single-file executable (`main.cpp`) with accompanying Chinese-language documentation in `docs/`.

## Build Commands

```bash
# Full build
mkdir build && cd build
cmake ..
cmake --build .

# Run a single pattern
./build/bin/<category>/<pattern-name>
# e.g. ./build/bin/creational/singleton
```

CMake 3.14+, C++17 required. Compiler flags: `-Wall -Wextra -Wpedantic`. Links pthread.

Each pattern is registered via `add_pattern_example(category name)` in CMakeLists.txt and produces a binary at `build/bin/<category>/<name>`.

## Architecture

- **`src/`** — Source code organized by GoF category: `creational/` (5), `structural/` (7), `behavioral/` (11). Each pattern lives in `src/<category>/<pattern-name>/main.cpp` and is fully self-contained (no shared headers or libraries).
- **`docs/`** — Mirrors `src/` structure. Each markdown file covers intent, UML (Mermaid), scenarios, pros/cons, and C++17 highlights. Written in Chinese.
- **`CMakeLists.txt`** — Single top-level file; uses `add_pattern_example()` function to add each pattern as a separate executable target.
- **`.vscode/`** — VS Code config for build tasks (g++), GDB launch, and IntelliSense.

## Conventions

- Every pattern is a **single `main.cpp`** with its own `main()` — no headers, no shared code.
- Code uses C++17 features idiomatically: `std::variant`/`std::visit`, `std::optional`, `std::string_view`, `std::shared_mutex`, `if constexpr`, structured bindings, `[[nodiscard]]`.
- Documentation and code comments are in **Chinese**.
- No test framework exists; patterns are validated by running the executable and observing console output.

## Adding a New Pattern

1. Create `src/<category>/<pattern-name>/main.cpp`
2. Add `add_pattern_example(<category> <pattern-name>)` to `CMakeLists.txt`
3. Create `docs/<category>/<pattern-name>.md`
4. Add entry to the relevant table in `README.md`
