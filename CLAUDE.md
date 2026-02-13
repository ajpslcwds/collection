# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
mkdir build && cd build
cmake ..
make
```

Run the example binary:
```bash
./redis_pool_example
```

Install the library:
```bash
make install
```

## Architecture

This is a C++17 Redis connection pool library built on top of [hiredis](https://github.com/redis/hiredis).

### Core classes

- **`RedisConnection`** (`redis_connection.h/cpp`) — wraps a single hiredis connection. Handles auth (Redis 6.0+ ACL username/password), reconnection, health checks, and idle-time tracking.

- **`RedisPool`** (`redis_pool.h/cpp`) — singleton that owns a fixed-size pool of `RedisConnection` objects. Uses a mutex + condition variable to hand out connections to callers and a background thread for periodic health checks and reconnection.

- **`RedisConnectionGuard`** (`redis_pool.h`) — RAII handle returned by `RedisPool::GetConnection()`. Acquires a connection on construction, returns it on destruction. Exposes `Command()`, `CommandArgv()`, `AppendCommand()`/`GetReply()` (pipeline), and `MSet()`/`MGet()` (batch pipeline).

### Configuration (`RedisConfig`)

Key fields: `host`, `port`, `unix_sock_path` (takes priority over TCP), `username`, `password`, `db`, `pool_size` (default 20), `timeout_ms` (default 3000), `max_idle_time_ms` (default 60000), `max_retry_times` (default 3), `retry_interval_ms` (default 1000).

### Dependencies

- **hiredis ≥ 1.2.0** — must be installed system-wide
- **pthread** — linked automatically via CMake
- **C++17**

Target environment: Ubuntu 20.04, Redis 7.4.
