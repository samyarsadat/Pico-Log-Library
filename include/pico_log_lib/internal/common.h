/*
    Pico Log - Common header file.
    A fast logging library for RP2xxx microcontrollers.
    
    Copyright 2025 Samyar Sadat Akhavi.
    Written by Samyar Sadat Akhavi, 2025.
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once


// Logger verbosity levels.
typedef enum {
    LOG_LVL_DEBUG, 
    LOG_LVL_INFO, 
    LOG_LVL_WARN, 
    LOG_LVL_ERROR, 
    LOG_LVL_FATAL
} LOG_LEVEL_t;

// Logger options structure.
typedef struct {
    LOG_LEVEL_t logging_level;
    const char* log_format;
    bool ansi_styling;
    bool process_style_tags;
} logger_options_t;