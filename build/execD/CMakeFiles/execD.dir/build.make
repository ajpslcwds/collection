# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wzq/code/wubuzhidao

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wzq/code/wubuzhidao/build

# Include any dependencies generated for this target.
include execD/CMakeFiles/execD.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include execD/CMakeFiles/execD.dir/compiler_depend.make

# Include the progress variables for this target.
include execD/CMakeFiles/execD.dir/progress.make

# Include the compile flags for this target's objects.
include execD/CMakeFiles/execD.dir/flags.make

execD/CMakeFiles/execD.dir/main.cc.o: execD/CMakeFiles/execD.dir/flags.make
execD/CMakeFiles/execD.dir/main.cc.o: /home/wzq/code/wubuzhidao/execD/main.cc
execD/CMakeFiles/execD.dir/main.cc.o: execD/CMakeFiles/execD.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wzq/code/wubuzhidao/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object execD/CMakeFiles/execD.dir/main.cc.o"
	cd /home/wzq/code/wubuzhidao/build/execD && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT execD/CMakeFiles/execD.dir/main.cc.o -MF CMakeFiles/execD.dir/main.cc.o.d -o CMakeFiles/execD.dir/main.cc.o -c /home/wzq/code/wubuzhidao/execD/main.cc

execD/CMakeFiles/execD.dir/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/execD.dir/main.cc.i"
	cd /home/wzq/code/wubuzhidao/build/execD && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wzq/code/wubuzhidao/execD/main.cc > CMakeFiles/execD.dir/main.cc.i

execD/CMakeFiles/execD.dir/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/execD.dir/main.cc.s"
	cd /home/wzq/code/wubuzhidao/build/execD && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wzq/code/wubuzhidao/execD/main.cc -o CMakeFiles/execD.dir/main.cc.s

# Object files for target execD
execD_OBJECTS = \
"CMakeFiles/execD.dir/main.cc.o"

# External object files for target execD
execD_EXTERNAL_OBJECTS =

execD/execD: execD/CMakeFiles/execD.dir/main.cc.o
execD/execD: execD/CMakeFiles/execD.dir/build.make
execD/execD: execD/CMakeFiles/execD.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wzq/code/wubuzhidao/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable execD"
	cd /home/wzq/code/wubuzhidao/build/execD && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/execD.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
execD/CMakeFiles/execD.dir/build: execD/execD
.PHONY : execD/CMakeFiles/execD.dir/build

execD/CMakeFiles/execD.dir/clean:
	cd /home/wzq/code/wubuzhidao/build/execD && $(CMAKE_COMMAND) -P CMakeFiles/execD.dir/cmake_clean.cmake
.PHONY : execD/CMakeFiles/execD.dir/clean

execD/CMakeFiles/execD.dir/depend:
	cd /home/wzq/code/wubuzhidao/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wzq/code/wubuzhidao /home/wzq/code/wubuzhidao/execD /home/wzq/code/wubuzhidao/build /home/wzq/code/wubuzhidao/build/execD /home/wzq/code/wubuzhidao/build/execD/CMakeFiles/execD.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : execD/CMakeFiles/execD.dir/depend

