# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/bcmac/userapps/nasapp/source/mysql-5.5.28

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bcmac/userapps/nasapp/source/mysql-5.5.28

# Utility rule file for test-force.

# Include the progress variables for this target.
include mysql-test/CMakeFiles/test-force.dir/progress.make

mysql-test/CMakeFiles/test-force:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test && echo Running tests
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test && echo OS=Linux
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test && export MTR_BUILD_THREAD=auto
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test && perl ./mysql-test-run.pl --force

test-force: mysql-test/CMakeFiles/test-force
test-force: mysql-test/CMakeFiles/test-force.dir/build.make
.PHONY : test-force

# Rule to build all files generated by this target.
mysql-test/CMakeFiles/test-force.dir/build: test-force
.PHONY : mysql-test/CMakeFiles/test-force.dir/build

mysql-test/CMakeFiles/test-force.dir/clean:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test && $(CMAKE_COMMAND) -P CMakeFiles/test-force.dir/cmake_clean.cmake
.PHONY : mysql-test/CMakeFiles/test-force.dir/clean

mysql-test/CMakeFiles/test-force.dir/depend:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test /home/bcmac/userapps/nasapp/source/mysql-5.5.28/mysql-test/CMakeFiles/test-force.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : mysql-test/CMakeFiles/test-force.dir/depend

