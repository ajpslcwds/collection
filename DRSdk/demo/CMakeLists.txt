

# 如果第三方应用引DRSdk，需要添加DRSdk库的路径
# 假设在父目录下build后的安装目录为 install
# link_directories(${CMAKE_SOURCE_DIR}/../DRSdk/build/install/lib)
# include_directories(${CMAKE_SOURCE_DIR}/../DRSdk/build/install/include)

# Boost 1.69 的安装路径
set(BOOST_ROOT "/usr/local")
# set(BOOST_ROOT "../repos/boost_1_69_0")
# 阻止 CMake 搜索系统路径中的 Boost（/usr/lib/x86_64-linux-gnu/cmake 记录 apt 安装的 Boost 1.71 CMake 配置的部分）
set(Boost_NO_SYSTEM_PATHS ON)
find_package(Boost 1.69 REQUIRED COMPONENTS system thread)

# 编译主程序
add_executable(drsdk_demo main.cpp)

# 链接DRSdk库
target_link_libraries(drsdk_demo drsdk)
target_link_libraries(drsdk_demo Boost::system Boost::thread)

# 设置可执行文件的输出目录
set_target_properties(drsdk_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##################################################################
# 编译主程序
add_executable(fake_ds fake_ds.cpp)

# 链接DRSdk库
target_link_libraries(fake_ds drsdk)
target_link_libraries(fake_ds Boost::system Boost::thread)

# 设置可执行文件的输出目录
set_target_properties(fake_ds PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
