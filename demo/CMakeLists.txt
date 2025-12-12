cmake_minimum_required(VERSION 3.15)
project(demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo main.cpp)

# 仅链接 mylogger，不链接 spdlog！
target_link_libraries(demo PRIVATE dsflog pthread)
target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/dsflog/include
)
