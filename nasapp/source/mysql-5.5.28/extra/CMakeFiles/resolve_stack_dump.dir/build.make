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
include extra/CMakeFiles/resolve_stack_dump.dir/depend.make

# Include the progress variables for this target.
include extra/CMakeFiles/resolve_stack_dump.dir/progress.make

# Include the compile flags for this target's objects.
include extra/CMakeFiles/resolve_stack_dump.dir/flags.make

extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o: extra/CMakeFiles/resolve_stack_dump.dir/flags.make
extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o: extra/resolve_stack_dump.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/bcmac/userapps/nasapp/source/mysql-5.5.28/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o   -c /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra/resolve_stack_dump.c

extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.i"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra/resolve_stack_dump.c > CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.i

extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.s"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra && /projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra/resolve_stack_dump.c -o CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.s

extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.requires:
.PHONY : extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.requires

extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.provides: extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.requires
	$(MAKE) -f extra/CMakeFiles/resolve_stack_dump.dir/build.make extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.provides.build
.PHONY : extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.provides

extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.provides.build: extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o

# Object files for target resolve_stack_dump
resolve_stack_dump_OBJECTS = \
"CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o"

# External object files for target resolve_stack_dump
resolve_stack_dump_EXTERNAL_OBJECTS =

extra/resolve_stack_dump: extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o
extra/resolve_stack_dump: extra/CMakeFiles/resolve_stack_dump.dir/build.make
extra/resolve_stack_dump: mysys/libmysys.a
extra/resolve_stack_dump: dbug/libdbug.a
extra/resolve_stack_dump: mysys/libmysys.a
extra/resolve_stack_dump: dbug/libdbug.a
extra/resolve_stack_dump: strings/libstrings.a
extra/resolve_stack_dump: probes_mysql.o
extra/resolve_stack_dump: extra/CMakeFiles/resolve_stack_dump.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable resolve_stack_dump"
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/resolve_stack_dump.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
extra/CMakeFiles/resolve_stack_dump.dir/build: extra/resolve_stack_dump
.PHONY : extra/CMakeFiles/resolve_stack_dump.dir/build

extra/CMakeFiles/resolve_stack_dump.dir/requires: extra/CMakeFiles/resolve_stack_dump.dir/resolve_stack_dump.c.o.requires
.PHONY : extra/CMakeFiles/resolve_stack_dump.dir/requires

extra/CMakeFiles/resolve_stack_dump.dir/clean:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra && $(CMAKE_COMMAND) -P CMakeFiles/resolve_stack_dump.dir/cmake_clean.cmake
.PHONY : extra/CMakeFiles/resolve_stack_dump.dir/clean

extra/CMakeFiles/resolve_stack_dump.dir/depend:
	cd /home/bcmac/userapps/nasapp/source/mysql-5.5.28 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra /home/bcmac/userapps/nasapp/source/mysql-5.5.28 /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra /home/bcmac/userapps/nasapp/source/mysql-5.5.28/extra/CMakeFiles/resolve_stack_dump.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : extra/CMakeFiles/resolve_stack_dump.dir/depend

