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
include plugin/semisync/CMakeFiles/semisync_slave.dir/depend.make

# Include the progress variables for this target.
include plugin/semisync/CMakeFiles/semisync_slave.dir/progress.make

# Include the compile flags for this target's objects.
include plugin/semisync/CMakeFiles/semisync_slave.dir/flags.make

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o: plugin/semisync/CMakeFiles/semisync_slave.dir/flags.make
plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o: plugin/semisync/semisync.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/semisync_slave.dir/semisync.cc.o -c /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync.cc

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/semisync_slave.dir/semisync.cc.i"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync.cc > CMakeFiles/semisync_slave.dir/semisync.cc.i

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/semisync_slave.dir/semisync.cc.s"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync.cc -o CMakeFiles/semisync_slave.dir/semisync.cc.s

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.requires:
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.requires

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.provides: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.requires
	$(MAKE) -f plugin/semisync/CMakeFiles/semisync_slave.dir/build.make plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.provides.build
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.provides

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.provides.build: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o: plugin/semisync/CMakeFiles/semisync_slave.dir/flags.make
plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o: plugin/semisync/semisync_slave.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/semisync_slave.dir/semisync_slave.cc.o -c /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync_slave.cc

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/semisync_slave.dir/semisync_slave.cc.i"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync_slave.cc > CMakeFiles/semisync_slave.dir/semisync_slave.cc.i

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/semisync_slave.dir/semisync_slave.cc.s"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync_slave.cc -o CMakeFiles/semisync_slave.dir/semisync_slave.cc.s

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.requires:
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.requires

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.provides: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.requires
	$(MAKE) -f plugin/semisync/CMakeFiles/semisync_slave.dir/build.make plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.provides.build
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.provides

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.provides.build: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o: plugin/semisync/CMakeFiles/semisync_slave.dir/flags.make
plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o: plugin/semisync/semisync_slave_plugin.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o -c /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync_slave_plugin.cc

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.i"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync_slave_plugin.cc > CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.i

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.s"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/semisync_slave_plugin.cc -o CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.s

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.requires:
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.requires

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.provides: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.requires
	$(MAKE) -f plugin/semisync/CMakeFiles/semisync_slave.dir/build.make plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.provides.build
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.provides

plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.provides.build: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o

# Object files for target semisync_slave
semisync_slave_OBJECTS = \
"CMakeFiles/semisync_slave.dir/semisync.cc.o" \
"CMakeFiles/semisync_slave.dir/semisync_slave.cc.o" \
"CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o"

# External object files for target semisync_slave
semisync_slave_EXTERNAL_OBJECTS =

plugin/semisync/semisync_slave.so: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o
plugin/semisync/semisync_slave.so: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o
plugin/semisync/semisync_slave.so: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o
plugin/semisync/semisync_slave.so: plugin/semisync/CMakeFiles/semisync_slave.dir/build.make
plugin/semisync/semisync_slave.so: probes_mysql.o
plugin/semisync/semisync_slave.so: libservices/libmysqlservices.a
plugin/semisync/semisync_slave.so: plugin/semisync/CMakeFiles/semisync_slave.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX shared module semisync_slave.so"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/semisync_slave.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugin/semisync/CMakeFiles/semisync_slave.dir/build: plugin/semisync/semisync_slave.so
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/build

plugin/semisync/CMakeFiles/semisync_slave.dir/requires: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync.cc.o.requires
plugin/semisync/CMakeFiles/semisync_slave.dir/requires: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave.cc.o.requires
plugin/semisync/CMakeFiles/semisync_slave.dir/requires: plugin/semisync/CMakeFiles/semisync_slave.dir/semisync_slave_plugin.cc.o.requires
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/requires

plugin/semisync/CMakeFiles/semisync_slave.dir/clean:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync && $(CMAKE_COMMAND) -P CMakeFiles/semisync_slave.dir/cmake_clean.cmake
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/clean

plugin/semisync/CMakeFiles/semisync_slave.dir/depend:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync /home/bcmac/userapps/nasapp/source/mysql-5.5.28/plugin/semisync/CMakeFiles/semisync_slave.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugin/semisync/CMakeFiles/semisync_slave.dir/depend

