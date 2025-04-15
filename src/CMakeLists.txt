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
cmake_minimum_required(VERSION 3.12)

# Add source files
add_library(${PROJECT_NAME} logger.c++)

# Include header files
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

# Link to libraries
target_link_libraries(${PROJECT_NAME} pico_stdlib)

if (PICO_LOG_FREERTOS)
    target_link_libraries(${PROJECT_NAME} FreeRTOS-Kernel)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_LOG_FREERTOS=1)
else()
    target_link_libraries(${PROJECT_NAME} pico_sync)
endif()

# Enable all warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)