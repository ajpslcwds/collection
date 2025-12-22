#指定Cmake版本NGVSDemoTest
cmake_minimum_required(VERSION 3.16.3)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
  cmake_policy(SET CMP0048 NEW)
endif()

#设置工程名
project(TXDsfDemo VERSION ${TX_VERSION} LANGUAGES CXX) 

# 根据是否级联(PR_ROOT_PATH)设置动态库库文件目录
if(NOT DEFINED PR_ROOT_PATH)
  string(REGEX REPLACE "(.*)/(.*)/(.*)/(.*)/(.*)" "\\1" PR_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
endif()
message(STATUS "Configuring DsfServiceDemo example...")
message(STATUS "===== PR_ROOT_PATH:${PR_ROOT_PATH}")


# # ASAN 开关（可选）
# option(USE_ASAN "Enable AddressSanitizer" ON)

# if(USE_ASAN)
#     message(STATUS "AddressSanitizer enabled")
#     set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
# endif()


#添加头文件路径
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${PR_ROOT_PATH}/include/
  ${PR_ROOT_PATH}/COM/dsf/include/ForDSF
)

# #宏定义
file(GLOB DsfServiceDemo_CPP CONFIGURE_DEPENDS "./src/*.cpp")

set(EXECUTABLE_OUTPUT_PATH ../bin/)

# set(SANITIZER_FLAGS    " -fno-stack-protector   -g")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")

#添加链接库路径
link_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${PR_ROOT_PATH}/lib
)
message(STATUS "===== CMAKE_CURRENT_SOURCE_DIR:${CMAKE_CURRENT_SOURCE_DIR}")

# #指定编译的可执行文件
add_executable(TXDsfDemo ${DsfServiceDemo_CPP})
set_property(TARGET TXDsfDemo PROPERTY POSITION_INDEPENDENT_CODE TRUE)
# #指定静态链接库
target_link_libraries(TXDsfDemo DSF TXDDS TXCDR pthread dl tinyxml2 crypto)
 