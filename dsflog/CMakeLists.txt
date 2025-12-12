cmake_minimum_required(VERSION 3.15)
project(dsflog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 创建静态库
add_library(dsflog STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dsflog.cpp)

# 头文件对用户可见
target_include_directories(dsflog PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# !!! spdlog 必须 PRIVATE，不暴露给用户 !!!
target_link_libraries(dsflog PRIVATE spdlog::spdlog tinyxml2)

# 隐藏 spdlog 对外符号（非必须，但推荐）
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(dsflog PRIVATE -fvisibility=hidden)
endif()

# install(TARGETS dsflog
#     ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib
# )

install(DIRECTORY include/ DESTINATION ${CMAKE_BINARY_DIR}/include)
