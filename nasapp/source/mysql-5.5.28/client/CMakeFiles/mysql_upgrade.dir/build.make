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
include client/CMakeFiles/mysql_upgrade.dir/depend.make

# Include the progress variables for this target.
include client/CMakeFiles/mysql_upgrade.dir/progress.make

# Include the compile flags for this target's objects.
include client/CMakeFiles/mysql_upgrade.dir/flags.make

client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o: client/CMakeFiles/mysql_upgrade.dir/flags.make
client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o: client/mysql_upgrade.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o   -c /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client/mysql_upgrade.c

client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.i"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client/mysql_upgrade.c > CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.i

client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.s"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client/mysql_upgrade.c -o CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.s

client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.requires:
.PHONY : client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.requires

client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.provides: client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.requires
	$(MAKE) -f client/CMakeFiles/mysql_upgrade.dir/build.make client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.provides.build
.PHONY : client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.provides

client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.provides.build: client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o

# Object files for target mysql_upgrade
mysql_upgrade_OBJECTS = \
"CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o"

# External object files for target mysql_upgrade
mysql_upgrade_EXTERNAL_OBJECTS =

client/mysql_upgrade: client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o
client/mysql_upgrade: client/CMakeFiles/mysql_upgrade.dir/build.make
client/mysql_upgrade: libmysql/libmysqlclient.a
client/mysql_upgrade: probes_mysql.o
client/mysql_upgrade: client/CMakeFiles/mysql_upgrade.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable mysql_upgrade"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mysql_upgrade.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
client/CMakeFiles/mysql_upgrade.dir/build: client/mysql_upgrade
.PHONY : client/CMakeFiles/mysql_upgrade.dir/build

client/CMakeFiles/mysql_upgrade.dir/requires: client/CMakeFiles/mysql_upgrade.dir/mysql_upgrade.c.o.requires
.PHONY : client/CMakeFiles/mysql_upgrade.dir/requires

client/CMakeFiles/mysql_upgrade.dir/clean:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client && $(CMAKE_COMMAND) -P CMakeFiles/mysql_upgrade.dir/cmake_clean.cmake
.PHONY : client/CMakeFiles/mysql_upgrade.dir/clean

client/CMakeFiles/mysql_upgrade.dir/depend:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client /home/bcmac/userapps/nasapp/source/mysql-5.5.28/client/CMakeFiles/mysql_upgrade.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : client/CMakeFiles/mysql_upgrade.dir/depend
