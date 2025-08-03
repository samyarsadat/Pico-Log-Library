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


set(TARGET_NAME ${PROJECT_NAME}_cpp)

# Add source files
add_executable(${TARGET_NAME} main.cpp)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${TARGET_NAME})

# Link to libraries
target_link_libraries(${TARGET_NAME} pico_stdlib
                                     pico_log_lib
                                     FreeRTOS-Kernel
                                     FreeRTOS-Kernel-Heap4)

# Link to include directories
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# PicoTool binary information
pico_set_program_name(${TARGET_NAME} ${TARGET_NAME})
pico_set_program_version(${TARGET_NAME} ${PROJECT_VERSION})
pico_set_program_description(${TARGET_NAME} "Pico Log Library - FreeRTOS Example")

# Enable STDIO over USB and UART
pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

# Compile definitions & options
target_compile_definitions(${TARGET_NAME} PRIVATE PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
target_compile_definitions(${TARGET_NAME} PRIVATE PICO_STDIO_DEFAULT_CRLF=0)
target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)