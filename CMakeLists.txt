cmake_minimum_required(VERSION 3.20.0)

cmake_policy(SET CMP0057 NEW)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(smart-home-firmware)

FILE(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR}
    "src/*.c"
)

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
)

target_sources(app PRIVATE ${SOURCES})