# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wzq/code/aic/aic_app

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wzq/code/aic/aic_app/build

# Include any dependencies generated for this target.
include CMakeFiles/aic_app.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/aic_app.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/aic_app.dir/flags.make

CMakeFiles/aic_app.dir/src/main.cpp.o: CMakeFiles/aic_app.dir/flags.make
CMakeFiles/aic_app.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wzq/code/aic/aic_app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/aic_app.dir/src/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/aic_app.dir/src/main.cpp.o -c /home/wzq/code/aic/aic_app/src/main.cpp

CMakeFiles/aic_app.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/aic_app.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wzq/code/aic/aic_app/src/main.cpp > CMakeFiles/aic_app.dir/src/main.cpp.i

CMakeFiles/aic_app.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/aic_app.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wzq/code/aic/aic_app/src/main.cpp -o CMakeFiles/aic_app.dir/src/main.cpp.s

CMakeFiles/aic_app.dir/src/aic_manager.cpp.o: CMakeFiles/aic_app.dir/flags.make
CMakeFiles/aic_app.dir/src/aic_manager.cpp.o: ../src/aic_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wzq/code/aic/aic_app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/aic_app.dir/src/aic_manager.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/aic_app.dir/src/aic_manager.cpp.o -c /home/wzq/code/aic/aic_app/src/aic_manager.cpp

CMakeFiles/aic_app.dir/src/aic_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/aic_app.dir/src/aic_manager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wzq/code/aic/aic_app/src/aic_manager.cpp > CMakeFiles/aic_app.dir/src/aic_manager.cpp.i

CMakeFiles/aic_app.dir/src/aic_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/aic_app.dir/src/aic_manager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wzq/code/aic/aic_app/src/aic_manager.cpp -o CMakeFiles/aic_app.dir/src/aic_manager.cpp.s

# Object files for target aic_app
aic_app_OBJECTS = \
"CMakeFiles/aic_app.dir/src/main.cpp.o" \
"CMakeFiles/aic_app.dir/src/aic_manager.cpp.o"

# External object files for target aic_app
aic_app_EXTERNAL_OBJECTS =

../bin/aic_app: CMakeFiles/aic_app.dir/src/main.cpp.o
../bin/aic_app: CMakeFiles/aic_app.dir/src/aic_manager.cpp.o
../bin/aic_app: CMakeFiles/aic_app.dir/build.make
../bin/aic_app: CMakeFiles/aic_app.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wzq/code/aic/aic_app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../bin/aic_app"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/aic_app.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/aic_app.dir/build: ../bin/aic_app

.PHONY : CMakeFiles/aic_app.dir/build

CMakeFiles/aic_app.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/aic_app.dir/cmake_clean.cmake
.PHONY : CMakeFiles/aic_app.dir/clean

CMakeFiles/aic_app.dir/depend:
	cd /home/wzq/code/aic/aic_app/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wzq/code/aic/aic_app /home/wzq/code/aic/aic_app /home/wzq/code/aic/aic_app/build /home/wzq/code/aic/aic_app/build /home/wzq/code/aic/aic_app/build/CMakeFiles/aic_app.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/aic_app.dir/depend

