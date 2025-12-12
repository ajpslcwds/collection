# AGENTS.md - dsflog 项目开发指南

## 项目概述

本项目用于封装 spdlog 日志库为 `libdsflog.so`，作为日志库组件供其他业务模块使用。libdsflog 对外隐藏 spdlog 库的符号。

- **语言**: C++17
- **构建系统**: CMake 3.15+
- **依赖**: spdlog, tinyxml2 (通过 git submodule 管理)

---

## 构建命令

### 初始化子模块
```bash
git submodule update --init
```

### 配置项目
```bash
# 使用预设 (推荐)
cmake --preset dsflog

# 或手动配置
cmake -B build/dsflog -DCMAKE_BUILD_TYPE=Debug
```

### 编译项目
```bash
# 编译全部目标
cmake --build build/dsflog

# 或使用 make
cd build && make
```

### 编译单个目标
```bash
# 仅编译 dsflog 库
cmake --build build/dsflog --target dsflog

# 仅编译 demo
cmake --build build/dsflog --target demo
```

### 运行 demo
```bash
./build/dsflog/bin/demo
```

### 清理构建
```bash
rm -rf build/*
```

---

## 测试

**当前状态**: 项目暂无单元测试。

如需添加测试框架，推荐使用 **Google Test (gtest)**。

### 添加测试示例
在 `CMakeLists.txt` 中添加:
```cmake
include(GoogleTest)
add_subdirectory(tests)
```

### 运行单个测试
```bash
# 使用 ctest
ctest --test-dir build/dsflog -R test_name

# 或直接运行测试可执行文件
./build/dsflog/tests/mytest
```

---

## 代码风格规范

### 基本规范
- **C++ 标准**: C++17
- **缩进**: 2 空格
- **行尾**: LF (Unix 风格)
- **头文件**: 使用 `#pragma once`

### 头文件包含顺序
```cpp
// 1. 标准库头文件
#include <vector>
#include <string>
#include <unordered_map>

// 2. 第三方库头文件
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

// 3. 项目内部头文件
#include "dsflog.h"
```

### 命名约定
- **类/结构体/枚举**: `CamelCase` (如 `Logger`, `LogLevel`)
- **变量/函数**: `snake_case` (如 `logger_impl_`, `InitLogConf`)
- **成员变量**: 建议使用 `snake_case_` 带后缀下划线 (如 `logger_impl_`)
- **常量**: `kCamelCase` 或 `SCREAMING_SNAKE_CASE`
- **宏/枚举值**: `SCREAMING_SNAKE_CASE` (如 `LOG_INIT`, `LogLevel::INFO`)

### 类型使用
- **字符串**: 优先使用 `std::string`
- **定宽整数**: 使用 `<cstdint>` 中的类型 (`int32_t`, `uint64_t` 等)
- **智能指针**: 使用 `std::shared_ptr`, `std::unique_ptr`
- **布尔值**: 使用 `bool` 或 `std::atomic_bool`

### 错误处理
- **返回值**: 使用 `int32_t` 返回码，`0` 表示成功，负数表示错误
- **不推荐**: 尽量避免使用异常 (项目未启用异常)
- **空指针检查**: 使用 `nullptr` 进行检查

### 符号可见性 (关键)
- **spdlog/tinyxml2 链接**: 必须使用 `PRIVATE`，禁止暴露给用户
- **隐藏符号**: 库内部使用 `-fvisibility=hidden` 编译选项

```cmake
# 正确示例
target_link_libraries(dsflog PRIVATE spdlog::spdlog tinyxml2)

# 错误示例 - 会暴露 spdlog 符号给用户
# target_link_libraries(dsflog PUBLIC spdlog::spdlog)
```

### 代码组织
- **命名空间**: 所有代码放在 `dsflog` 命名空间中
- **头文件**: 放在 `dsflog/include/` 目录
- **源文件**: 放在 `dsflog/src/` 目录
- **PIMPL 模式**: 使用 `LoggerImpl` 隐藏实现细节

### 日志宏定义规范
```cpp
// 文件: dsflog/include/dsflog.h

// 宏命名: LOG_xxx 格式，使用大写
#define LOG_INIT(file) dsflog::Logger::GetInstance().Init(file)
#define LOG_INFO(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
```

### 代码示例

**头文件 (dsflog.h)**:
```cpp
#pragma once
#include <stdint.h>
#include <atomic>
#include <memory>
#include <string>

namespace dsflog {

enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL };

class LoggerImpl;  // 前向声明

class Logger {
 public:
  static Logger& GetInstance();
  int32_t Init(const std::string& file);
  int32_t Shutdown();

 private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
};

}  // namespace dsflog
```

**源文件 (dsflog.cpp)**:
```cpp
#include "dsflog.h"

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <unordered_map>
#include <vector>

namespace dsflog {

// 实现...

}  // namespace dsflog
```

---

## 项目结构

```
.
├── CMakeLists.txt           # 顶层 CMake 配置
├── CMakePresets.json        # CMake 预设配置
├── dsflog/                   # 库源码
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── dsflog.h         # 对外头文件
│   └── src/
│       └── dsflog.cpp       # 库实现
├── demo/                     # 示例程序
│   ├── CMakeLists.txt
│   └── main.cpp
├── third_party/              # 第三方依赖
│   ├── spdlog/               # 日志库
│   └── tinyxml2/             # XML 解析库
├── conf/                     # 配置文件
│   └── dsflog.conf           # 日志配置文件
└── build/                    # 构建输出目录
```

---

## Git 使用规范

- **禁止自动提交**: 不要主动提交代码，除非用户明确要求
- **分支策略**: 遵循项目现有分支策略
- **子模块**: 更新子模块需谨慎，确保与项目兼容

---

## 常见问题

### 编译报错: tinyxml2 xmltest
**原因**: tinyxml2 默认尝试构建测试程序，但资源路径错误。  
**解决**: 在顶层 `CMakeLists.txt` 中设置 `BUILD_TESTING=OFF`

### 链接错误: undefined reference to spdlog
**原因**: 可能错误地将 spdlog 链接为 PUBLIC。  
**解决**: 确保使用 `target_link_libraries(dsflog PRIVATE spdlog::spdlog)`

### 运行 demo 无输出
**原因**: `LOG_INIT()` 可能未正确初始化。  
**解决**: 确保传入正确的配置文件路径。

---

## 开发建议

1. **先编译再修改**: 任何代码修改后先确保编译通过
2. **保持符号隐藏**: 新增依赖必须使用 PRIVATE 链接
3. **遵循现有风格**: 修改代码时保持与现有代码风格一致
4. **配置验证**: 修改 CMake 后运行 `cmake .. && make` 验证
