#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

# set(BOARD qemu_x86_64)
# set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.20)
# find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_scope)
target_include_directories(app PRIVATE src)
# library sources / headers
file (GLOB app_sources
    src/*.cpp
    ../src/ScopeMimicry.cpp
    )

# message(STATUS "######### ${app_sources} ############")
target_include_directories(app PRIVATE ../src)
target_sources(app PRIVATE 
    ${app_sources}
    )

