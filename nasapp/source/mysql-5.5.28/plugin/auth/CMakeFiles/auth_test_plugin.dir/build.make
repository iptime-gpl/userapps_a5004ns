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

# Include any dependencies generated for this target.
include plugin/auth/CMakeFiles/auth_test_plugin.dir/depend.make

# Include the progress variables for this target.
include plugin/auth/CMakeFiles/auth_test_plugin.dir/progress.make

# Include the compile flags for this target's objects.
include plugin/auth/CMakeFiles/auth_test_plugin.dir/flags.make

plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o: plugin/auth/CMakeFiles/auth_test_plugin.dir/flags.make
plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o: plugin/auth/test_plugin.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/auth_test_plugin.dir/test_plugin.c.o   -c /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth/test_plugin.c

plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/auth_test_plugin.dir/test_plugin.c.i"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth/test_plugin.c > CMakeFiles/auth_test_plugin.dir/test_plugin.c.i

plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/auth_test_plugin.dir/test_plugin.c.s"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth/test_plugin.c -o CMakeFiles/auth_test_plugin.dir/test_plugin.c.s

plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.requires:
.PHONY : plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.requires

plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.provides: plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.requires
	$(MAKE) -f plugin/auth/CMakeFiles/auth_test_plugin.dir/build.make plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.provides.build
.PHONY : plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.provides

plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.provides.build: plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o

# Object files for target auth_test_plugin
auth_test_plugin_OBJECTS = \
"CMakeFiles/auth_test_plugin.dir/test_plugin.c.o"

# External object files for target auth_test_plugin
auth_test_plugin_EXTERNAL_OBJECTS =

plugin/auth/auth_test_plugin.so: plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o
plugin/auth/auth_test_plugin.so: plugin/auth/CMakeFiles/auth_test_plugin.dir/build.make
plugin/auth/auth_test_plugin.so: probes_mysql.o
plugin/auth/auth_test_plugin.so: libservices/libmysqlservices.a
plugin/auth/auth_test_plugin.so: plugin/auth/CMakeFiles/auth_test_plugin.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared module auth_test_plugin.so"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/auth_test_plugin.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugin/auth/CMakeFiles/auth_test_plugin.dir/build: plugin/auth/auth_test_plugin.so
.PHONY : plugin/auth/CMakeFiles/auth_test_plugin.dir/build

plugin/auth/CMakeFiles/auth_test_plugin.dir/requires: plugin/auth/CMakeFiles/auth_test_plugin.dir/test_plugin.c.o.requires
.PHONY : plugin/auth/CMakeFiles/auth_test_plugin.dir/requires

plugin/auth/CMakeFiles/auth_test_plugin.dir/clean:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth && $(CMAKE_COMMAND) -P CMakeFiles/auth_test_plugin.dir/cmake_clean.cmake
.PHONY : plugin/auth/CMakeFiles/auth_test_plugin.dir/clean

plugin/auth/CMakeFiles/auth_test_plugin.dir/depend:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/auth/CMakeFiles/auth_test_plugin.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugin/auth/CMakeFiles/auth_test_plugin.dir/depend

