/*
    Pico Log - Main library header (C API).
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
#include "pico/stdlib.h"
#include "pico/stdio/driver.h"
#include "pico_log_lib/internal/common.h"


// Opaque logger handle type.
typedef void* logger_handle_t;


#ifdef __cplusplus
extern "C" {
#endif
    /**
     * @brief Initializes a logger instance with the specified stdio driver and options.
     *
     * This function sets up a new logger using the provided stdio driver for output and
     * configuration options. It returns a handle to the initialized logger object.
     *
     * @param stdio_driver Pointer to the stdio driver to be used for logging output.
     * @param options Pointer to the logger_options structure containing configuration parameters.
     * @return Initialized logger object handle or nullptr if memory allocation fails.
     */
    logger_handle_t logger_init(stdio_driver_t* stdio_driver, logger_options_t* options);
    
    /**
     * @brief Destroys the logger instance and frees associated resources.
     *
     * This function cleans up the logger instance, releasing any allocated memory and
     * resources associated with it. NOP if the logger handle is NULL.
     *
     * @param logger Logger object handle.
     */
    void logger_destroy(logger_handle_t logger);

    /**
     * @brief Initializes the mutex for the logger.
     *
     * This function initializes the mutex used for thread-safe logging operations.
     * The logger can be used without a mutex, however, it will not be thread-safe.
     *
     * @param logger Logger object handle.
     * @return Always true when using Pico SDK mutexes or true only when the mutex
     *         is successfully created when using FreeRTOS.
     */
    bool logger_init_mutex(logger_handle_t logger);

    /**
     * @brief Reparses the log format string of the logger.
     *
     * This function updates the internal format tokens for the given logger handle.
     * It should be called if the format string associated with the logger has changed.
     *
     * @param logger Logger object handle.
     * @return true if the format was successfully reparsed,
     *         false if the mutex could not be acquired.
     */
    bool logger_reparse_format(logger_handle_t logger);
    
    /**
     * @brief Logs a formatted message with the specified log verbosity.
     *
     * This function formats and logs a message at the specified log level, including
     * function name, file name, and line number.
     *
     * @param logger Logger object handle.
     * @param func Function name where the log is called.
     * @param file Source file name where the log is called.
     * @param line Line number in the source file where the log is called.
     * @param level Log verbosity level.
     * @param message Log message (supports format specifiers).
     */
    void logger_log(logger_handle_t logger, const char* func, const char* file, const uint16_t line, 
                    LOG_LEVEL_t level, const char* message, ...);
    
    /**
     * @brief Logs a formatted message with the specified log verbosity.
     *
     * Same as logger_log(), except that it takes a va_list for the format string.
     *
     * @param logger Logger object handle.
     * @param level Log verbosity level.
     * @param message Log message (supports format specifiers).
     * @param args va_list for the format string.
     * @param func Function name where the log is called.
     * @param file Source file name where the log is called.
     * @param line Line number in the source file where the log is called.
     */
    void logger_vlog(logger_handle_t logger, LOG_LEVEL_t level, const char* message, va_list args, 
                     const char* func, const char* file, const uint16_t line);    
#ifdef __cplusplus
}
#endif
