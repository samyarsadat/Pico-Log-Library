/*
    Pico Log - Logger C API.
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
    along with this program.  If not, see <https: www.gnu.org/licenses/>.
*/

#include "pico_log_lib/logger_c.h"
#include "pico_log_lib/logger.h"
#include <new>


logger_handle_t logger_init(stdio_driver_t* stdio_driver, logger_options_t* options) {
    #ifdef PICO_LOG_FREERTOS
    void* memory = pvPortMalloc(sizeof(Logger));
    if (memory == nullptr) {
        return nullptr;
    }

    return static_cast<logger_handle_t>(new (memory) Logger(stdio_driver, options));
    #else
    return static_cast<logger_handle_t>(new Logger(stdio_driver, options));
    #endif
}

void logger_destroy(logger_handle_t logger) {
    if (logger != nullptr) {
        #ifdef PICO_LOG_FREERTOS
        static_cast<Logger*>(logger)->~Logger();
        vPortFree(logger);
        #else
        delete static_cast<Logger*>(logger);
        #endif

        logger = nullptr;
    }
}

bool logger_init_mutex(logger_handle_t logger) {
    assert(logger != nullptr);
    return static_cast<Logger*>(logger)->init_mutex();
}

bool logger_reparse_format(logger_handle_t logger) {
    assert(logger != nullptr);
    return static_cast<Logger*>(logger)->reparse_format();
}

void logger_log(logger_handle_t logger, const char* func, const char* file, const uint16_t line, 
                LOG_LEVEL_t level, const char* message, ...) {
    assert(logger != nullptr);
    
    va_list args;
    va_start(args, message);
    static_cast<Logger*>(logger)->vlog(level, message, args, func, file, line);
    va_end(args);
}

void logger_vlog(logger_handle_t logger, LOG_LEVEL_t level, const char* message, va_list args, 
                 const char* func, const char* file, const uint16_t line) {
    assert(logger != nullptr);
    static_cast<Logger*>(logger)->vlog(level, message, args, func, file, line);
}
