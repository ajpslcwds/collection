add_library(libB STATIC libB.cpp)
set_target_properties(libB PROPERTIES POSITION_INDEPENDENT_CODE ON)


target_link_directories(libB PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libA)
target_link_libraries(libB PRIVATE libA)

target_include_directories(libB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
