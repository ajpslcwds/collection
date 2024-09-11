cmake_minimum_required(VERSION 3.16)

# 项目名称
project(DRSdkApp)

# C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Boost 1.69 的安装路径
set(BOOST_ROOT "/usr/local")
# 阻止 CMake 搜索系统路径中的 Boost（/usr/lib/x86_64-linux-gnu/cmake 记录 apt 安装的 Boost 1.71 CMake 配置的部分）
set(Boost_NO_SYSTEM_PATHS ON)
find_package(Boost 1.69 REQUIRED COMPONENTS system thread)

# 如果第三方应用引DRSdk，需要添加DRSdk库的路径
# 假设在父目录下build后的安装目录为 install
link_directories(${CMAKE_SOURCE_DIR}/../DRSdk/build/install/lib)
include_directories(${CMAKE_SOURCE_DIR}/../DRSdk/build/install/include)


# 编译主程序
add_executable(drsdk_app main.cpp)

# 链接DRSdk库
target_link_libraries(drsdk_app drsdk)
target_link_libraries(drsdk_app Boost::system Boost::thread)
