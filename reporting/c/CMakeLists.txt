cmake_minimum_required(VERSION 2.6)
project(WAYWARD_METRICS)

IF(APPLE)
    SET(CMAKE_OSX_ARCHITECTURES "ppc;i386;x86_64" CACHE STRING "Build architectures for Mac OS X" FORCE) 
ENDIF(APPLE)

include_directories(${WAYWARD_METRICS_SOURCE_DIR}/include)
add_library(waywardmetrics
            src/allocator.c
            src/allocator_system.c
            src/buffer.c
            src/codec.c
            src/connection.c
            src/message_queue.c
            src/posix/file.c
            src/posix/thread.c
            src/reporting/context.c
            src/reporting/memory.c
            src/reporting/reporter.c
            src/socket.c
            src/version.c
           )

add_executable(test/test
            test/main.c)

target_link_libraries(test/test
            waywardmetrics
            pthread)
