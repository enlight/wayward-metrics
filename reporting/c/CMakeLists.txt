cmake_minimum_required(VERSION 2.6)
project(WAYWARD_METRICS)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "ppc;i386;x86_64" CACHE STRING "Build architectures for Mac OS X" FORCE) 
endif(APPLE)

add_subdirectory(src)
add_subdirectory(test)
