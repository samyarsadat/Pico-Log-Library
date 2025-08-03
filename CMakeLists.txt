#  Pico Log - CMake configuration.
#  A fast logging library for RP2xxx microcontrollers.
#
#  Copyright 2025 Samyar Sadat Akhavi.
#  Written by Samyar Sadat Akhavi, 2025.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https: www.gnu.org/licenses/>.


# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.13)

# Check whether PICO_SDK_PATH is defined.
if (NOT EXISTS $ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "The Pico SDK path environment variable is not set!")
endif ()

# Include build functions from the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set project name and C/C++ standards
project(pico_log_lib VERSION 0.2.0 LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Library options
cmake_policy(SET CMP0077 NEW)
option(PICO_LOG_FREERTOS "Enable FreeRTOS support" OFF)
option(PICO_LOG_BUILD_EXAMPLES "Build examples" OFF)

if (PICO_LOG_BUILD_EXAMPLES)
    # Set Pico Board and Pico Platform
    set(PICO_PLATFORM rp2040)
    set(PICO_BOARD pico)
endif ()

# Initialize the Pico SDK
pico_sdk_init()

# This is intended for example builds!
if (NOT DEFINED FREERTOS_KERNEL_PATH AND PICO_LOG_FREERTOS AND PICO_LOG_BUILD_EXAMPLES)
    message("Including FreeRTOS Kernel for examples...")

    # Include the FreeRTOS Kernel
    set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/libfreertos/FreeRTOS-Kernel" CACHE STRING "FreeRTOS Kernel Path" FORCE)
    include(FreeRTOS_Kernel_import.cmake)

    # FreeRTOS config
    set(FREERTOS_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/libfreertos/Config" CACHE STRING "FreeRTOS Config Path" FORCE)
    include_directories("${FREERTOS_CONFIG_DIR}")
endif ()

# Add source subdirectories
add_subdirectory(src)
add_subdirectory(examples)

# CMake flags
set(FLAGS "-mthumb -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

message("Configured ${PROJECT_NAME} library build version ${PROJECT_VERSION}")