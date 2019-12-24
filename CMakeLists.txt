cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)
add_executable(final server.cpp daemon.cpp fdpassing.cpp httphelper.cpp stringhelper.cpp filehelper.cpp)