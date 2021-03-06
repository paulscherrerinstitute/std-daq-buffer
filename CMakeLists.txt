cmake_minimum_required(VERSION 3.12)

project(std-daq-buffer)
set(CMAKE_CXX_STANDARD 17)
set (SF_DAQ_BUFFER_VERSION "2.0.0")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Download and unpack googletest at configure time
configure_file(googletest.in googletest-download/CMakeLists.txt)

execute_process(
        COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
        )

execute_process(
        COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download"
        )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

if (DEBUG_OUTPUT)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_OUTPUT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_OUTPUT")
endif ()

enable_testing()

add_subdirectory(external)
add_subdirectory("std-buffer")
add_subdirectory("std-stream-send")

