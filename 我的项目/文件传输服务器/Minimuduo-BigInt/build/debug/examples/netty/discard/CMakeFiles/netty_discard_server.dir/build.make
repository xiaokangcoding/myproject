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
CMAKE_SOURCE_DIR = /root/muduo/03/jmuduo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/muduo/03/build/debug

# Include any dependencies generated for this target.
include examples/netty/discard/CMakeFiles/netty_discard_server.dir/depend.make

# Include the progress variables for this target.
include examples/netty/discard/CMakeFiles/netty_discard_server.dir/progress.make

# Include the compile flags for this target's objects.
include examples/netty/discard/CMakeFiles/netty_discard_server.dir/flags.make

examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o: examples/netty/discard/CMakeFiles/netty_discard_server.dir/flags.make
examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o: /root/muduo/03/jmuduo/examples/netty/discard/server.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /root/muduo/03/build/debug/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o"
	cd /root/muduo/03/build/debug/examples/netty/discard && g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/netty_discard_server.dir/server.cc.o -c /root/muduo/03/jmuduo/examples/netty/discard/server.cc

examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/netty_discard_server.dir/server.cc.i"
	cd /root/muduo/03/build/debug/examples/netty/discard && g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /root/muduo/03/jmuduo/examples/netty/discard/server.cc > CMakeFiles/netty_discard_server.dir/server.cc.i

examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/netty_discard_server.dir/server.cc.s"
	cd /root/muduo/03/build/debug/examples/netty/discard && g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /root/muduo/03/jmuduo/examples/netty/discard/server.cc -o CMakeFiles/netty_discard_server.dir/server.cc.s

examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.requires:
.PHONY : examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.requires

examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.provides: examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.requires
	$(MAKE) -f examples/netty/discard/CMakeFiles/netty_discard_server.dir/build.make examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.provides.build
.PHONY : examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.provides

examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.provides.build: examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o

# Object files for target netty_discard_server
netty_discard_server_OBJECTS = \
"CMakeFiles/netty_discard_server.dir/server.cc.o"

# External object files for target netty_discard_server
netty_discard_server_EXTERNAL_OBJECTS =

bin/netty_discard_server: examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o
bin/netty_discard_server: examples/netty/discard/CMakeFiles/netty_discard_server.dir/build.make
bin/netty_discard_server: lib/libmuduo_net.a
bin/netty_discard_server: lib/libmuduo_base.a
bin/netty_discard_server: examples/netty/discard/CMakeFiles/netty_discard_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../../../bin/netty_discard_server"
	cd /root/muduo/03/build/debug/examples/netty/discard && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/netty_discard_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/netty/discard/CMakeFiles/netty_discard_server.dir/build: bin/netty_discard_server
.PHONY : examples/netty/discard/CMakeFiles/netty_discard_server.dir/build

examples/netty/discard/CMakeFiles/netty_discard_server.dir/requires: examples/netty/discard/CMakeFiles/netty_discard_server.dir/server.cc.o.requires
.PHONY : examples/netty/discard/CMakeFiles/netty_discard_server.dir/requires

examples/netty/discard/CMakeFiles/netty_discard_server.dir/clean:
	cd /root/muduo/03/build/debug/examples/netty/discard && $(CMAKE_COMMAND) -P CMakeFiles/netty_discard_server.dir/cmake_clean.cmake
.PHONY : examples/netty/discard/CMakeFiles/netty_discard_server.dir/clean

examples/netty/discard/CMakeFiles/netty_discard_server.dir/depend:
	cd /root/muduo/03/build/debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/muduo/03/jmuduo /root/muduo/03/jmuduo/examples/netty/discard /root/muduo/03/build/debug /root/muduo/03/build/debug/examples/netty/discard /root/muduo/03/build/debug/examples/netty/discard/CMakeFiles/netty_discard_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/netty/discard/CMakeFiles/netty_discard_server.dir/depend

