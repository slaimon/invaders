cmake_minimum_required(VERSION 3.10)

project(emu8080 VERSION 0.0.0)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")
set(SOURCE ${CMAKE_SOURCE_DIR}/source)
set(EXAMPLES ${CMAKE_SOURCE_DIR}/examples)
set(ROMS ${CMAKE_SOURCE_DIR}/assets/rom)
set(SOUND ${CMAKE_SOURCE_DIR}/assets/sound)
set(BUILD ${CMAKE_BINARY_DIR})
set(DATA ${BUILD}/data) # game assets for testing
set(RELEASE ${BUILD}/release)

file(MAKE_DIRECTORY ${DATA})
file(MAKE_DIRECTORY ${DATA}/sound)
file(COPY_FILE ${ROMS}/INVADERS ${DATA}/INVADERS)
file(GLOB SOUND_FILES ${SOUND}/*wav)
file(COPY ${SOUND_FILES} DESTINATION ${DATA}/sound)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
include_directories(include ${SDL3_INCLUDE_DIR})

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
add_executable(
    step
    ${EXAMPLES}/step.c
    ${SOURCE}/i8080_disassembler.c
    ${SOURCE}/i8080_debug.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)

# space invaders
add_executable(
    invaders
    ${SOURCE}/invaders.c
    ${SOURCE}/soundplayer.c
    ${SOURCE}/viewer.c
    ${SOURCE}/i8080.c
    ${SOURCE}/bytestream.c
    ${SOURCE}/safe.c
)
target_link_libraries(invaders PRIVATE SDL3::SDL3)

# i8080 cpu tests
enable_testing()
add_test(
    NAME test_pre
    COMMAND tester ${ROMS}/8080PRE.COM
)
set_property(
    TEST test_pre
    PROPERTY PASS_REGULAR_EXPRESSION "8080 Preliminary tests complete"
)

add_test(
    NAME test_lite
    COMMAND tester ${ROMS}/TEST.COM
)
set_property(
    TEST test_lite
    PROPERTY PASS_REGULAR_EXPRESSION "CPU IS OPERATIONAL"
)
set_property(
    TEST test_lite
    PROPERTY DEPENDS test_pre
)

add_test(
    NAME test_full
    COMMAND tester ${ROMS}/8080EX1.COM
)
set_property(
    TEST test_full
    PROPERTY FAIL_REGULAR_EXPRESSION "ERROR"
)
set_property(
    TEST test_full
    PROPERTY DEPENDS test_pre
)

# custom targets
add_custom_target(
    listing
    DEPENDS disassembler
    COMMAND disassembler ${ROMS}/INVADERS > ./listing.txt
)
set_property(
    TARGET listing
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES listing.txt
)
add_custom_target(
    log_example
    DEPENDS logger
    COMMAND logger ${ROMS}/8080EX1.COM sample_log.txt 1000
)
set_property(
    TARGET log_example
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES sample_log.txt
)
add_custom_target(
    release
    DEPENDS invaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RELEASE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RELEASE}/data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RELEASE}/data/sound
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:invaders> ${RELEASE}/invaders.exe
    COMMAND ${CMAKE_COMMAND} -E copy ${ROMS}/INVADERS ${RELEASE}/data/INVADERS
    COMMAND ${CMAKE_COMMAND} -E copy ${SOUND_FILES} ${RELEASE}/data/sound
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> ${RELEASE}
)