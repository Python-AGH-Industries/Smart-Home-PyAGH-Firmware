cmake_minimum_required(VERSION 3.21)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(smart-home-firmware)
target_sources(app PRIVATE src/main.c)