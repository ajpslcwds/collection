cmake_minimum_required(VERSION 3.10)
############FOR_MODIFIY_BEGIN##########
#Setting Project Name
PROJECT (drsdk)

INCLUDE($ENV{DRDIR}CMakeCommon)

#Setting Source Files
file(GLOB DRSKD_CPP ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
Set(SRCS ${SRCS} ${DRSKD_CPP})

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME drsdk)
#Setting library type used when build a library
#STATIC/SHARED
SET(LIB_TYPE SHARED)
# # control symbol export
# target_compile_definitions(drsdk PRIVATE DRSDK_EXPORTS)

# SET(LINK_LIBS tinyxml cv6netqueue cv6comm cv6log cv6logimpl)

################FOR_MODIFIY_END###########

INCLUDE($ENV{DRDIR}CMakeCommonLib)
