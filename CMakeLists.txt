cmake_minimum_required(VERSION 3.10)

project(emu8080 VERSION 0.0.0)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")
set(SOURCE ${CMAKE_SOURCE_DIR}/source)
set(EXAMPLES ${CMAKE_SOURCE_DIR}/examples)
set(ROMS ${CMAKE_SOURCE_DIR}/assets)

find_package(SDL2 REQUIRED)
include_directories(include ${SDL2_INCLUDE_DIR})

# example programs
add_executable(
    tester
    ${EXAMPLES}/tester.c
    ${SOURCE}/i8080_cpm.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)
add_executable(
    disassembler
    ${EXAMPLES}/disassembler.c
    ${SOURCE}/i8080_disassembler.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)
add_custom_target(
    listing
    COMMAND disassembler ${ROMS}/INVADERS > ./listing.txt
)
set_property(
    TARGET listing
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES listing.txt
)
add_executable(
    logger
    ${EXAMPLES}/logger.c
    ${SOURCE}/i8080_disassembler.c
    ${SOURCE}/i8080_debug.c
    ${SOURCE}/i8080_cpm.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)
add_custom_target(
    log_example
    COMMAND logger ${ROMS}/8080EX1.COM sample_log.txt 1000
)
set_property(
    TARGET log_example
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES sample_log.txt
)
add_executable(
    step
    ${EXAMPLES}/step.c
    ${SOURCE}/i8080_disassembler.c
    ${SOURCE}/i8080_debug.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)

# i8080 cpu tests
enable_testing()
add_test(NAME test_lite COMMAND tester ${ROMS}/TEST.COM)
set_property(TEST test_lite PROPERTY PASS_REGULAR_EXPRESSION "CPU IS OPERATIONAL")
add_test(NAME test_pre COMMAND tester ${ROMS}/8080PRE.COM)
set_property(TEST test_pre PROPERTY PASS_REGULAR_EXPRESSION "8080 Preliminary tests complete")
add_test(NAME test_full COMMAND tester ${ROMS}/8080EX1.COM)
set_property(TEST test_full PROPERTY FAIL_REGULAR_EXPRESSION "ERROR")

# space invaders
add_executable(
    invaders
    ${SOURCE}/invaders.c
    ${SOURCE}/viewer.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)
target_link_libraries(invaders ${SDL2_LIBRARIES})