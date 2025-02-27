cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(aic_demo VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Add the executable
add_executable(aic_app src/main.cpp src/aic_manager.cpp)
target_include_directories(aic_app PRIVATE ${CMAKE_SOURCE_DIR}/include)
# Link libraries
target_link_libraries(aic_app PRIVATE pthread)

# Set the output directory for the executable
set_target_properties(aic_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)