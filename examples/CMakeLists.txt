#  Pico Log - Examples
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


if (PICO_LOG_BUILD_EXAMPLES)
    if (PICO_LOG_FREERTOS)
        # FreeRTOS example    
        add_subdirectory(freertos)
    else ()
        # Baremetal example
        add_subdirectory(baremetal)
    endif ()
endif ()