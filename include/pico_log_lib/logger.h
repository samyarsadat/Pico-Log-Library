/*
    Pico Log - Main library header.
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

#ifdef PICO_LOG_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#else
#include "pico/sync.h"
#endif


// Main logger buffer size.
// Note that two buffers of this size are created.
// This value is NOT the maximum length of the log message,
// but the maximum length of the log message after formatting and styling are applied.
#ifndef LOGGER_BUFF_SIZE
    #define LOGGER_BUFF_SIZE 256
#endif

// Maximum number of tokens in the log format string.
// This value is used to limit the number of tokens that can be processed in the log format string.
// The actual number of tokens in the log format string may be less than this value.
#ifndef LOG_FORMAT_MAX_TOKENS 
    #define LOG_FORMAT_MAX_TOKENS 16
#endif

// ANSI escape code constants.
constexpr const char* ANSI_RESET = "\033[0m";
constexpr const char* ANSI_BOLD = "\033[1m";
constexpr const char* ANSI_UNDERLINE = "\033[4m";
constexpr const char* ANSI_STRIKETHROUGH = "\033[9m";
constexpr const char* ANSI_ITALIC = "\033[3m";


/*
    Main logger class.
*/
class Logger {
    public:
        Logger(stdio_driver_t* stdio_driver, logger_options_t* options);
        ~Logger();

        bool init_mutex();
        void log(const char* func, const char* file, const uint16_t line, 
                 const LOG_LEVEL_t level, const char* message, ...);
        void vlog(const LOG_LEVEL_t level, const char* message, va_list args, 
                  const char* func, const char* file, const uint16_t line);
        bool reparse_format();
    
    private:
        stdio_driver_t* stdio_driver;
        logger_options_t* options;

        #ifdef PICO_LOG_FREERTOS
        SemaphoreHandle_t log_mutex;
        #else
        bool mutex_initialized = false;
        mutex_t log_mutex;
        #endif

        // Main log message buffers
        char output_buff[LOGGER_BUFF_SIZE];
        char tmp_buff[LOGGER_BUFF_SIZE];

        enum COLOR {
            COLOR_BLACK,
            COLOR_RED,
            COLOR_GREEN,
            COLOR_YELLOW,
            COLOR_BLUE,
            COLOR_MAGENTA,
            COLOR_CYAN,
            COLOR_WHITE
        };

        struct color_spec {
            COLOR color;
            bool background;
            bool high_intensity;
            bool success;
        };
        typedef struct color_spec color_spec_t;

        enum LOG_FORMAT_TOKEN_TYPE {
            FORMAT_TOKEN_END,
            FORMAT_TOKEN_TEXT,
            FORMAT_TOKEN_STYLE,
            FORMAT_TOKEN_COLOR,
            FORMAT_TOKEN_FUNC,
            FORMAT_TOKEN_FILE,
            FORMAT_TOKEN_LINE,
            FORMAT_TOKEN_TASK,
            FORMAT_TOKEN_CORE,
            FORMAT_TOKEN_LEVEL,
            FORMAT_TOKEN_TIMESTAMP,
            FORMAT_TOKEN_MSG,
        };

        // Log format pre-parser token structure.
        struct log_format_token {
            LOG_FORMAT_TOKEN_TYPE type = FORMAT_TOKEN_END;
            uint8_t color_code = 0;
            const char* str_ptr = nullptr;
            size_t txt_token_len = 0;
        };
        typedef struct log_format_token log_format_token_t;

        log_format_token_t log_format_tokens[LOG_FORMAT_MAX_TOKENS];

        inline bool take_log_mutex();
        inline void release_log_mutex();
        
        void msg_format_tokenize();
        void clear_format_tokens();
        
        inline color_spec_t process_color_spec(const COLOR color, const char* &src_ptr, const size_t ptr_skip);
        inline size_t msg_process_format(char* buff, const size_t buff_size, const char* msg, const LOG_LEVEL_t level, 
                                         const char* func, const char* file, const uint16_t line);
        inline void msg_process_style(const char* src_ptr, char* buff, const size_t buff_size);
        
        constexpr const char* log_lvl_str(const LOG_LEVEL_t level);
        constexpr uint8_t log_lvl_color(const LOG_LEVEL_t level);
        constexpr uint8_t ansi_color_code(const color_spec_t clr_spec);
};