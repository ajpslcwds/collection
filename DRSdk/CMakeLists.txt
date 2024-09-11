cmake_minimum_required(VERSION 3.16)

# 项目名称
project(DRSdk)

# C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加头文件路径
include_directories(include)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# 添加库文件
file(GLOB SOURCES "source/*.cpp")
add_library(drsdk SHARED ${SOURCES})

# 控制符号导出
target_compile_definitions(drsdk PRIVATE DRSDK_EXPORTS)

# 安装库和头文件
install(TARGETS drsdk DESTINATION lib)
# install(FILES include/drsdk.h DESTINATION include)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h")


# 添加子目录 (构建 demo)
add_subdirectory(demo)
