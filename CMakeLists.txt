cmake_minimum_required(VERSION 3.10)
project(RedisPool VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# 创建连接池库
add_library(redis_pool STATIC
    redis_connection.cpp
    redis_connection.h
    redis_pool.cpp
    redis_pool.h
)

target_include_directories(redis_pool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}

)

target_link_libraries(redis_pool
    hiredis
    pthread
)


# 创建简单示例程序
add_executable(redis_pool_example
    example.cpp
)

target_include_directories(redis_pool_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HIREDIS_INCLUDE}
)

target_link_libraries(redis_pool_example
    redis_pool
    ${HIREDIS_LIB}
    pthread
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE)

# 安装目标
install(TARGETS redis_pool redis_pool_example
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES redis_connection.h redis_pool.h
    DESTINATION include
)

# 打印配置信息
message(STATUS "")
message(STATUS "配置摘要:")
message(STATUS "  项目: ${PROJECT_NAME}")
message(STATUS "  版本: ${PROJECT_VERSION}")
message(STATUS "  C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "  编译类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
