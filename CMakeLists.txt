cmake_minimum_required(VERSION 3.10)

project(emu8080 VERSION 0.0.0)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
include_directories(include ${SDL2_INCLUDE_DIR})

# example programs
add_executable(
    tester
    examples/tester.c
    source/i8080_cpm.c
    source/i8080.c
    source/bytestream.c
    source/safe.c
)
add_executable(
    disassembler
    examples/disassembler.c
    source/i8080_disassembler.c
    source/i8080.c
    source/bytestream.c
    source/safe.c
)
add_executable(
    logger
    examples/logger.c
    source/i8080_disassembler.c
    source/i8080_debug.c
    source/i8080_cpm.c
    source/i8080.c
    source/bytestream.c
    source/safe.c
)
add_executable(
    step
    examples/step.c
    source/i8080_disassembler.c
    source/i8080_debug.c
    source/i8080.c
    source/bytestream.c
    source/safe.c
)

# space invaders
add_executable(
    invaders
    source/invaders.c
    source/viewer.c
    source/i8080.c
    source/bytestream.c
    source/safe.c
)
target_link_libraries(invaders ${SDL2_LIBRARIES})