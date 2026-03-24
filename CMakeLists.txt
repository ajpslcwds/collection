cmake_minimum_required(VERSION 3.14)
project(collection CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------
# 可执行文件
# ---------------------------------------------------------------
set(CMAKE_BUILD_TYPE Debug)

add_executable(collection
    src/main.cpp
    src/db_loader.cpp
    src/struct_layout.cpp
)

target_include_directories(collection PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_directories(collection PRIVATE
    ${CMAKE_SOURCE_DIR}/library
)

target_link_libraries(collection PRIVATE
    sqlapi           # libsqlapi.so
    dl
    pthread
)
