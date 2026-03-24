# AGENTS.md - Developer Guidelines

C++17 CMake project running on WSL2 (Ubuntu 20.04). Reads MySQL tables via SQLAPI++,
caches them in memory, and computes struct memory layouts (offsets, alignment, sizes).

## Build Commands

### Build (incremental)
```bash
cmake -S . -B build && cmake --build build
```

### Clean Build
```bash
rm -rf build && cmake -S . -B build && cmake --build build
```

### Run
```bash
./build/collection
```
Requires a running MySQL 8.4.6 instance at 127.0.0.1:3306 (credentials in `src/main.cpp`).

### Test
No test framework. A standalone alignment-verification file exists:
```bash
g++ -std=c++17 -o src/test src/test.cpp && ./src/test
```
This prints `sizeof` results for structs under various `#pragma pack` settings.

### Lint / Format
No linter or formatter is configured. Follow the style rules below manually.

## Project Structure

```
├── CMakeLists.txt          # Build config (Debug, C++17)
├── src/
│   ├── main.cpp            # Entry point, DB connect → load → calculate → print
│   ├── db_structs.h        # Record structs for every DB table + shared_ptr aliases
│   ├── db_loader.h         # DbLoader class, DbCache aggregate struct
│   ├── db_loader.cpp       # SQL queries and field-reading helpers
│   ├── struct_layout.h     # StructLayoutCalculator, layout result structs
│   ├── struct_layout.cpp   # Alignment/offset calculation, JSON parsing, path resolution
│   └── test.cpp            # Manual sizeof/offsetof alignment tests
├── include/
│   └── sqlapi/             # SQLAPI++ 5.4.0 headers
├── library/                # Shared libraries (libsqlapi.so)
├── db_info/                # Reference data: SQL schema, sample JSON
└── build/                  # Generated (gitignored)
```

## Code Style

### General
- **Standard**: C++17 (`CMAKE_CXX_STANDARD 17`)
- Use `#pragma once` — never use include guards
- Always qualify STL types with `std::` — no `using namespace std`
- Comments are written in **Chinese** (consistent with existing codebase)

### Include Order
Own header first, then project headers, then third-party, then standard library:
```cpp
#include "struct_layout.h"          // 1. own header
#include "db_loader.h"              // 2. project headers
#include <nlohmann/json.hpp>        // 3. third-party
#include <algorithm>                // 4. standard library
#include <string>
```

### Naming Conventions
| Element          | Convention    | Examples                                   |
|------------------|---------------|--------------------------------------------|
| Structs/Classes  | PascalCase    | `DbLoader`, `ModelRecord`, `TypeExtra`     |
| Public members   | snake_case    | `driver_id`, `prop_name`, `total_length`   |
| Private members  | m_ prefix     | `m_conn`, `m_host`, `m_modelLayouts`       |
| Type aliases     | PascalCase+Ptr| `DriverRecordPtr`, `ModelPropLayoutPtr`     |
| Functions        | PascalCase    | `Connect()`, `LoadAll()`, `GetTypeAlign()` |
| Static helpers   | PascalCase    | `ParseAlign()`, `TrimString()`             |
| Local variables  | camelCase     | `currentOffset`, `typeExtraPtr`            |
| Files            | snake_case    | `db_loader.cpp`, `struct_layout.h`         |

### Types
- Use fixed-width integers: `uint32_t`, `int32_t`, `int16_t`, `uint8_t`, `int8_t`
- Use `std::string` for text, `std::vector<uint8_t>` for BLOBs
- Use `std::vector<T>` for ordered collections
- Use `std::unordered_map<K,V>` for keyed lookups
- Use `std::shared_ptr<T>` for heap-allocated records; define aliases:
  ```cpp
  using DriverRecordPtr = std::shared_ptr<DriverRecord>;
  ```

### Formatting
- **4-space** indentation (no tabs)
- Braces on **same line** for functions and control flow
- Braces on **new line** only for struct/class declarations
- Max line length: **120 characters**
- Blank line between logical sections; use `// ====` banner comments for major sections

### Functions
- Pass strings as `const std::string&`
- Pass primitive types by value
- Return `int32_t` for success/failure: **0 = success, -1 = failure**
- Mark file-local helpers as `static`
- Use `explicit` on single-argument constructors

### Error Handling
- Wrap all SQLAPI++ calls in `try/catch (SAException &x)`
- Log errors to `std::cerr` with a prefix like `"LoadDrivers error: "`
- Return `-1` on failure instead of throwing from public API
- For non-critical warnings use `[WARN]` prefix on stderr
- Catch `std::exception&` and `...` in `main()` as a safety net

### Database Code Patterns
- Use `SACommand` with inline SQL strings (no parameterized queries currently)
- Field indices are **1-based** (SQLAPI++ convention)
- Always check nullable fields via helper functions before reading:
  - `ReadOptStr(cmd, idx)` — returns `""` if NULL
  - `ReadOptUint32(cmd, idx)` — returns `0` if NULL
  - `ReadOptInt16(cmd, idx)` — returns `0` if NULL
  - `ReadOptInt32(cmd, idx)` — returns `0` if NULL
  - `ReadOptDouble(cmd, idx)` — returns `0.0` if NULL
  - `ReadBlob(cmd, idx)` — returns empty `vector<uint8_t>` if NULL
- Standard load pattern:
  ```cpp
  SACommand cmd(m_conn, "SELECT ... FROM T_DD_SM_XXX");
  cmd.Execute();
  while (cmd.FetchNext()) {
      auto rec = std::make_shared<XxxRecord>();
      rec->field = static_cast<uint32_t>(cmd.Field(1).asULong());
      // ...
      cache.xxx[rec->key] = rec;
  }
  ```

### Memory Management
- `SAConnection*` is managed manually (`new`/`delete` in ctor/dtor)
- All record structs are heap-allocated via `std::make_shared<T>()`
- `DbCache` owns all records through `shared_ptr`; no raw owning pointers for data

### JSON Handling
- Uses **nlohmann/json** (header-only, at `<nlohmann/json.hpp>`)
- Define `from_json()` free functions for custom struct deserialization
- Parse with `json::parse(str)` inside try/catch

## Key Dependencies
- **SQLAPI++ 5.4.0** — MySQL client wrapper (headers in `include/sqlapi/`, lib in `library/`)
- **nlohmann/json** — JSON parsing for `TYPE_EXTRA` fields
- **MySQL 8.4.6** — backend database
- **CMake 3.14+** — build system
- **g++ with C++17** — compiler
