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

# Utility rule file for abi_check.

# Include the progress variables for this target.
include CMakeFiles/abi_check.dir/progress.make

CMakeFiles/abi_check:
	/usr/bin/cmake -DCOMPILER=/projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc -DSOURCE_DIR=/home/bcmac/userapps/nasapp/source/mysql-5.5.28 -DBINARY_DIR=/home/bcmac/userapps/nasapp/source/mysql-5.5.28 "-DABI_HEADERS=/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql/plugin_audit.h;/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql/plugin_ftparser.h;/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql.h;/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql/psi/psi_abi_v1.h;/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql/psi/psi_abi_v2.h;/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql/client_plugin.h;/home/bcmac/userapps/nasapp/source/mysql-5.5.28/include/mysql/plugin_auth.h" -P /home/bcmac/userapps/nasapp/source/mysql-5.5.28/cmake/do_abi_check.cmake

abi_check: CMakeFiles/abi_check
abi_check: CMakeFiles/abi_check.dir/build.make
.PHONY : abi_check

# Rule to build all files generated by this target.
CMakeFiles/abi_check.dir/build: abi_check
.PHONY : CMakeFiles/abi_check.dir/build

CMakeFiles/abi_check.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/abi_check.dir/cmake_clean.cmake
.PHONY : CMakeFiles/abi_check.dir/clean

CMakeFiles/abi_check.dir/depend:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles/abi_check.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/abi_check.dir/depend
