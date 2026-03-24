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

# i8080 cpu tests
enable_testing()
add_test(NAME test_lite COMMAND tester ../assets/TEST.COM)
set_property(TEST test_lite PROPERTY PASS_REGULAR_EXPRESSION "CPU IS OPERATIONAL")
add_test(NAME test_pre COMMAND tester ../assets/8080PRE.COM)
set_property(TEST test_pre PROPERTY PASS_REGULAR_EXPRESSION "8080 Preliminary tests complete")
add_test(NAME test_full COMMAND tester ../assets/8080EX1.COM)
set_property(TEST test_full PROPERTY FAIL_REGULAR_EXPRESSION "ERROR")

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