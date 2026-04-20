# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C++17 logging library component called **dsflog** that wraps [spdlog](https://github.com/gabime/spdlog) into a static library (`libdsflog`). The goal is to expose a clean logging API while hiding all spdlog symbols from consumers, so downstream modules only depend on `dsflog.h` and never on spdlog directly.

## Build

```bash
# First time: initialize submodules
git submodule update --init

# Build
mkdir build && cd build
cmake ..
make
```

Outputs:
- `build/lib/libdsflog.a` — the static library
- `build/bin/demo` — demo executable
- `build/include/dsflog.h` — installed public header

## Running the Demo

```bash
cd build
./bin/demo
```

The demo logs 100 iterations at DEBUG/INFO/WARN/ERROR levels with 100ms sleep between each. It reads `conf/dsflog.conf` for configuration.

## Architecture

**Symbol hiding:** The library is built with `-fvisibility=hidden` and links spdlog/tinyxml2 as `PRIVATE` dependencies. Consumers see only the `dsflog` API.

**Pimpl pattern:** `Logger` (public) delegates to `LoggerImpl` (private, in `dsflog.cpp`). This keeps spdlog types out of the public header entirely.

**Singleton:** `Logger::GetInstance()` returns the single logger instance. Initialized via `LOG_INIT(confPath)`, shut down via `LOG_SHUTDOWN()`.

**Configuration:** XML file (`conf/dsflog.conf`) defines named loggers. Each logger entry supports:
- `level` — TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL
- `filename` — output log file path
- `max_size` — max file size in MB before rotation
- `max_files` — number of rotated files to keep
- `save_days` — days to retain daily logs (alternative rotation mode)
- `async` — async write mode

**Macro API** (from `dsflog.h`):
```cpp
LOG_INIT("conf/dsflog.conf");
LOG_DEBUG("message {}", value);
LOG_INFO("message");
LOG_WARN("message");
LOG_ERROR("message");
LOG_SHUTDOWN();
```

Macros automatically capture `__FILE__`, `__FUNCTION__`, `__LINE__`.

## Dependencies

Managed as git submodules under `third_party/`:
- `spdlog` v1.15.0
- `tinyxml2`
