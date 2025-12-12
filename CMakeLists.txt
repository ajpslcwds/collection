cmake_minimum_required(VERSION 3.10)
project(myspdlog)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)


# 添加third_party
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/tinyxml2)


# 设置输出目录
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子目录
add_subdirectory(dsflog)
add_subdirectory(demo)

