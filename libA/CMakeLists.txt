add_library(libA STATIC libA.cpp)
set_target_properties(libA PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(libA PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
