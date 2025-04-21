#  Pico Log - FreeRTOS Example
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


message("Configuring Pico Log FreeRTOS example build...")
project(pico_log_freertos_example VERSION 0.1.0 LANGUAGES C CXX ASM)

# Add source files
add_executable(${PROJECT_NAME} main.cpp)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to libraries
target_link_libraries(${PROJECT_NAME} pico_stdlib
                                      pico_log_lib
                                      FreeRTOS-Kernel
                                      FreeRTOS-Kernel-Heap4)

# Link to include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# PicoTool binary information
pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_version(${PROJECT_NAME} ${PROJECT_VERSION})
pico_set_program_description(${PROJECT_NAME} "Pico Log Library - FreeRTOS Example")

# Enable STDIO over USB and UART
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Compile definitions & options
target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_STDIO_DEFAULT_CRLF=0)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)