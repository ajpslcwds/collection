add_library(libC SHARED libC.cpp)

target_link_directories(libC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../libB)
target_link_libraries(libC PUBLIC libB)

target_include_directories(libC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(libC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)


 
