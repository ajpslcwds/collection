
add_executable(test test.cpp)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test myspdlog pthread)

install(TARGETS test ARCHIVE DESTINATION bin)
 
