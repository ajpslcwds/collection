cmake_minimum_required(VERSION 3.10)
project(myspdlog)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认安装前缀
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory" FORCE)

# 配置 spdlog：静态库，禁用共享库，使用内置 fmt
set(SPDLOG_INSTALL ON CACHE BOOL "Enable spdlog installation" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as static library" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use bundled fmt library" FORCE)

# 为 spdlog 和 myspdlog 启用 -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(external/spdlog)

# 创建对象库，包含 spdlog 和 myspdlog 的对象文件
add_library(myspdlog_objects OBJECT src/myspdlog.cpp)
target_include_directories(myspdlog_objects PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include
)
target_link_libraries(myspdlog_objects PUBLIC spdlog)

# 创建静态库，合并对象文件
add_library(myspdlog STATIC $<TARGET_OBJECTS:myspdlog_objects> $<TARGET_OBJECTS:spdlog>)
target_include_directories(myspdlog PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include
)

# 安装目标
install(TARGETS myspdlog ARCHIVE DESTINATION lib)
install(FILES include/myspdlog.h DESTINATION include)

add_subdirectory(examples)