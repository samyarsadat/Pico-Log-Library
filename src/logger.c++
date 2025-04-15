/*
    Pico Log - Main library source file.
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

#include "pico_log_lib/logger.h"
#include <cstdio>
#include <cstring>


/* ---- PUBLIC ---- */
Logger::Logger(stdio_driver_t* stdio_driver, logger_options_t* options) {
    assert(stdio_driver != nullptr);
    assert(options != nullptr);

    #ifdef PICO_LOG_FREERTOS
    this->log_mutex = xSemaphoreCreateMutex();
    assert(this->log_mutex != nullptr);  // There isn't much we can do if mutex creation fails...
    #else
    mutex_init(&this->log_mutex);
    #endif
    
    this->stdio_driver = stdio_driver;
    this->options = options;

    this->clear_format_tokens();
    this->msg_format_tokenize();
}

Logger::~Logger() {
    #ifdef PICO_LOG_FREERTOS
    if (this->log_mutex != nullptr) {
        xSemaphoreTake(this->log_mutex, portMAX_DELAY);
        vSemaphoreDelete(this->log_mutex);
        this->log_mutex = nullptr;
    }
    #endif
}

void Logger::log(const char* func, const char* file, const uint16_t line, LOG_LEVEL level, const char* message, ...) {
    if (this->options->logging_level <= level) {
        va_list args;
        va_start(args, message);
        this->vlog(level, message, args, func, file, line);
        va_end(args);
    }
}

void Logger::vlog(LOG_LEVEL level, const char* message, va_list args, 
                  const char* func, const char* file, const uint16_t line) {
    if (this->take_log_mutex()) {
        const bool proc_style_tags = this->options->ansi_styling && this->options->process_style_tags;
        const char* message_ptr = proc_style_tags ? this->output_buff : message;

        if (proc_style_tags) {
            msg_process_style(message, this->output_buff, LOGGER_BUFF_SIZE);
        }

        vsnprintf(this->tmp_buff, LOGGER_BUFF_SIZE, message_ptr, args);
        size_t msg_len = msg_process_format(this->output_buff, LOGGER_BUFF_SIZE, this->tmp_buff, level, func, file, line);
        
        this->stdio_driver->out_chars(this->output_buff, msg_len);
        this->release_log_mutex();
    }
}


/* ---- PRIVATE ---- */
inline bool Logger::take_log_mutex() {
    #ifdef PICO_LOG_FREERTOS
    return xSemaphoreTake(log_mutex, portMAX_DELAY) == pdTRUE;
    #else
    mutex_enter_blocking(&this->log_mutex);
    return true;
    #endif
}

inline void Logger::release_log_mutex() {
    #ifdef PICO_LOG_FREERTOS
    xSemaphoreGive(log_mutex);
    #else
    mutex_exit(&this->log_mutex);
    #endif
}

constexpr uint8_t Logger::ansi_color_code(const color_spec_t clr_spec) {
    return ((uint8_t)clr_spec.color) + 30 + (clr_spec.background ? 10 : 0) + (clr_spec.high_intensity ? 60 : 0);
}

constexpr const char* Logger::log_lvl_str(const LOG_LEVEL level) {
    switch (level) {
        case LOG_LVL_DEBUG: return "DEBUG";
        case LOG_LVL_INFO:  return "INFO";
        case LOG_LVL_WARN:  return "WARNING";
        case LOG_LVL_ERROR: return "ERROR";
        case LOG_LVL_FATAL: return "FATAL";
        default:            return "UNKNOWN";
    }
}

constexpr uint8_t Logger::log_lvl_color(const LOG_LEVEL level) {
    switch (level) {
        case LOG_LVL_DEBUG: return ansi_color_code({COLOR_CYAN, false, false, false});
        case LOG_LVL_INFO:  return ansi_color_code({COLOR_BLUE, false, false, false});
        case LOG_LVL_WARN:  return ansi_color_code({COLOR_YELLOW, false, false, false});
        case LOG_LVL_ERROR: return ansi_color_code({COLOR_RED, false, false, false});
        case LOG_LVL_FATAL: return ansi_color_code({COLOR_MAGENTA, false, false, false});
        default:            return ansi_color_code({COLOR_WHITE, false, false, false});
    }
}

inline Logger::color_spec_t Logger::process_color_spec(const COLOR color, const char* &src_ptr, const size_t ptr_skip) {
    color_spec_t clr_spec = {color, false, false, true};
    src_ptr += ptr_skip;
    
    if (*src_ptr == '%') {
        src_ptr++;
        return clr_spec;
    }

    for (size_t i = 0; *src_ptr && *src_ptr != '%' && i < 4; i++) {
        src_ptr++;
        if (*(src_ptr - 1) != '_') {
            continue;
        }

        if (memcmp(src_ptr, "HI", 2) == 0) {
            clr_spec.high_intensity = true;
            src_ptr += 2;
        } else if (memcmp(src_ptr, "BG", 2) == 0) {
            clr_spec.background = true;
            src_ptr += 2;
        }
    }

    if (*src_ptr == '%') {
        src_ptr++;
        return clr_spec;
    }

    clr_spec.success = false;
    return clr_spec;
}

#define ADD_FORMAT_TOKEN(tkn_type, ptr_skip)            \
    this->log_format_tokens[token_num].type = tkn_type; \
    token_num++;                                        \
    src_ptr += ptr_skip;                                \
    continue;

#define ADD_FORMAT_TOKEN_STL(ansi_style, ptr_skip)             \
    this->log_format_tokens[token_num].ansi_code = ansi_style; \
    ADD_FORMAT_TOKEN(FORMAT_TOKEN_STYLE, ptr_skip);

#define ADD_FORMAT_TOKEN_CLR(color, ptr_skip)                   \
    clr_spec = process_color_spec(color, src_ptr, ptr_skip);    \
    if (clr_spec.success) {                                     \
        this->log_format_tokens[token_num].clr_spec = clr_spec; \
        ADD_FORMAT_TOKEN(FORMAT_TOKEN_COLOR, 0);                \
    }

void Logger::msg_format_tokenize() {
    uint32_t token_num = 0;
    const char* src_ptr = this->options->log_format;
    color_spec_t clr_spec;
    
    while (*src_ptr && token_num < LOG_FORMAT_MAX_TOKENS) {
        if (*src_ptr == '%') {
            src_ptr++;
            if (this->log_format_tokens[token_num].type == FORMAT_TOKEN_TEXT) {
                token_num++;
            }

            switch (*src_ptr) {
                case 'T':
                    if (memcmp(src_ptr, "TSTMP%", 6) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_TIMESTAMP, 6);
                    } else if (memcmp(src_ptr, "TASK%", 5) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_TASK, 5);
                    }
                    break;
                case 'L':
                    if (memcmp(src_ptr, "LVL%", 4) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_LEVEL, 4);
                    } else if (memcmp(src_ptr, "LINE%", 5) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_LINE, 5);
                    }
                    break;
                case 'F':
                    if (memcmp(src_ptr, "FILE%", 5) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_FILE, 5);
                    } else if (memcmp(src_ptr, "FUNC%", 5) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_FUNC, 5);
                    }
                    break;
                case 'M':
                    if (memcmp(src_ptr, "MSG%", 4) == 0) {
                        ADD_FORMAT_TOKEN(FORMAT_TOKEN_MSG, 4);
                    } else if (memcmp(src_ptr, "MGT", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_MAGENTA, 3);
                    }
                    break;
                case 'I':
                    if (memcmp(src_ptr, "ITL%", 4) == 0) {
                        ADD_FORMAT_TOKEN_STL(ANSI_ITALIC, 4);
                    }
                    break;
                case 'U':
                    if (memcmp(src_ptr, "UDRLN%", 6) == 0) {
                        ADD_FORMAT_TOKEN_STL(ANSI_UNDERLINE, 6);
                    }
                    break;
                case 'S':
                    if (memcmp(src_ptr, "STKTHR%", 7) == 0) {
                        ADD_FORMAT_TOKEN_STL(ANSI_STRIKETHROUGH, 7);
                    }
                    break;
                case 'B':
                    if (memcmp(src_ptr, "BOLD%", 5) == 0) {
                        ADD_FORMAT_TOKEN_STL(ANSI_BOLD, 5);
                    } else if (memcmp(src_ptr, "BLU", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_BLUE, 3);
                    } else if (memcmp(src_ptr, "BLK", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_BLACK, 3);
                    }
                    break;
                case 'R':
                    if (memcmp(src_ptr, "RED", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_RED, 3);
                    } else if (memcmp(src_ptr, "RST%", 4) == 0) {
                        ADD_FORMAT_TOKEN_STL(ANSI_RESET, 4);
                    }
                    break;
                case 'G':
                    if (memcmp(src_ptr, "GRN", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_GREEN, 3);
                    }
                    break;
                case 'Y':
                    if (memcmp(src_ptr, "YLW", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_YELLOW, 3);
                    }
                    break;
                case 'C':
                    if (memcmp(src_ptr, "CYN", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_CYAN, 3);
                    }
                    break;
                case 'W':
                    if (memcmp(src_ptr, "WHT", 3) == 0) {
                        ADD_FORMAT_TOKEN_CLR(COLOR_WHITE, 3);
                    }
                    break;
            }

            continue;
        }
            
        if (this->log_format_tokens[token_num].type != FORMAT_TOKEN_TEXT) {
            this->log_format_tokens[token_num].type = FORMAT_TOKEN_TEXT;
            this->log_format_tokens[token_num].txt_token_ptr = src_ptr;
            this->log_format_tokens[token_num].txt_token_len = 1;
        } else {
            this->log_format_tokens[token_num].txt_token_len++;
        }

        src_ptr++;
    }
}

void Logger::clear_format_tokens() {
    for (uint32_t i = 0; i < LOG_FORMAT_MAX_TOKENS; i++) {
        this->log_format_tokens[i].type = FORMAT_TOKEN_END;
    }
}

#define BUFFER_CONCAT(str_ptr)                            \
    str_len = strnlen(str_ptr, buff_size - buff_pos - 1); \
    memcpy(buff + buff_pos, str_ptr, str_len);            \
    buff_pos += str_len;                                  \
    continue;

#define BUFF_SPRINTF(fmt, ...)                                                             \
    buff_pos_size_n = buff_size - buff_pos - 1;                                            \
    str_len = snprintf(buff + buff_pos, buff_pos_size_n, fmt, __VA_ARGS__);                \
    if (str_len > 0) {                                                                     \
        buff_pos += str_len - (str_len > buff_pos_size_n ? str_len - buff_pos_size_n : 0); \
    }                                                                                      \
    continue;

inline size_t Logger::msg_process_format(char* buff, const size_t buff_size, const char* msg, LOG_LEVEL level, 
                                         const char* func, const char* file, const uint16_t line) {
    size_t token_len, str_len, buff_pos_size_n, buff_pos = 0;
    uint32_t ms_since_boot, timestamp_sec;
    uint16_t timestamp_millisec;
    int str_len_diff;
    
    for (uint32_t i = 0; i < LOG_FORMAT_MAX_TOKENS && buff_pos < buff_size; i++) {
        switch (this->log_format_tokens[i].type) {
            case FORMAT_TOKEN_TEXT:
                token_len = this->log_format_tokens[i].txt_token_len;
                str_len_diff = (buff_size - buff_pos - 1) - token_len;
                
                if (str_len_diff < 0) {
                    token_len += str_len_diff;
                }
                
                memcpy(buff + buff_pos, this->log_format_tokens[i].txt_token_ptr, token_len);
                buff_pos += token_len;
                continue;
            case FORMAT_TOKEN_STYLE:
                if (this->options->ansi_styling) {
                    BUFFER_CONCAT(this->log_format_tokens[i].ansi_code);
                }
                break;
            case FORMAT_TOKEN_COLOR:
                if (this->options->ansi_styling) {
                    BUFF_SPRINTF("\033[0;%dm", ansi_color_code(this->log_format_tokens[i].clr_spec));
                }
                break;
            case FORMAT_TOKEN_FUNC:
                BUFFER_CONCAT(func);
            case FORMAT_TOKEN_FILE:
                BUFFER_CONCAT(file);
            case FORMAT_TOKEN_LINE:
                BUFF_SPRINTF("%u", line);
            case FORMAT_TOKEN_TASK:
                #ifdef PICO_LOG_FREERTOS
                if (xTaskGetCurrentTaskHandle() != nullptr) {
                    BUFFER_CONCAT(pcTaskGetName(nullptr));
                } else {
                    BUFFER_CONCAT("UNKNOWN");
                }
                #else
                BUFFER_CONCAT("NO TASK");
                #endif
            case FORMAT_TOKEN_LEVEL:
                if (this->options->ansi_styling) {
                    BUFF_SPRINTF("\033[0;%dm%s%s", log_lvl_color(level), log_lvl_str(level), ANSI_RESET);
                }
                
                BUFFER_CONCAT(log_lvl_str(level));
            case FORMAT_TOKEN_TIMESTAMP:
                ms_since_boot = to_ms_since_boot(get_absolute_time());
                timestamp_sec = ms_since_boot / 1000;
                timestamp_millisec = ms_since_boot - (timestamp_sec * 1000);
                BUFF_SPRINTF("%lu.%03u", timestamp_sec, timestamp_millisec);
            case FORMAT_TOKEN_MSG:
                BUFFER_CONCAT(msg);
            case FORMAT_TOKEN_END:
                goto exit_loop;
        }
    }

    exit_loop:
    if (buff_size >= buff_pos) {
        buff[buff_pos]     = '\r';
        buff[buff_pos + 1] = '\n';
        buff[buff_pos + 2] = '\0';
        return buff_pos + 2;
    }

    if (buff_pos >= 2) {
        buff[buff_pos - 2] = '\r';
        buff[buff_pos - 1] = '\n';
        buff[buff_pos]     = '\0';
    }

    return buff_pos;
}

#define PROCESS_COLOR_SPEC(color, ptr_skip)                   \
    clr_spec = process_color_spec(color, src_ptr, ptr_skip);  \
    if (clr_spec.success) {                                   \
        BUFF_SPRINTF("\033[0;%dm", ansi_color_code(clr_spec)) \
    }

inline void Logger::msg_process_style(const char* src_ptr, char* buff, const size_t buff_size) {
    const size_t buff_size_n = buff_size - 1;
    color_spec_t clr_spec;
    size_t buff_pos_size_n, str_len, buff_pos = 0;
    
    while (*src_ptr && buff_pos < buff_size_n) {
        if (*src_ptr == '%') {
            src_ptr++;

            switch (*src_ptr) {
                case 'I':
                    if (memcmp(src_ptr, "ITL%", 4) == 0) {
                        src_ptr += 4;
                        BUFFER_CONCAT(ANSI_ITALIC);
                    }
                    break;
                case 'U':
                    if (memcmp(src_ptr, "UDRLN%", 6) == 0) {
                        src_ptr += 6;
                        BUFFER_CONCAT(ANSI_UNDERLINE);
                    }
                    break;
                case 'S':
                    if (memcmp(src_ptr, "STKTHR%", 7) == 0) {
                        src_ptr += 7;
                        BUFFER_CONCAT(ANSI_STRIKETHROUGH);
                    }
                    break;
                case 'B':
                    if (memcmp(src_ptr, "BOLD%", 5) == 0) {
                        src_ptr += 5;
                        BUFFER_CONCAT(ANSI_BOLD);
                    } else if (memcmp(src_ptr, "BLU", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_BLUE, 3);
                    } else if (memcmp(src_ptr, "BLK", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_BLACK, 3);
                    }
                    break;
                case 'R':
                    if (memcmp(src_ptr, "RED", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_RED, 3);
                    } else if (memcmp(src_ptr, "RST%", 4) == 0) {
                        src_ptr += 4;
                        BUFFER_CONCAT(ANSI_RESET);
                    }
                    break;
                case 'G':
                    if (memcmp(src_ptr, "GRN", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_GREEN, 3);
                    }
                    break;
                case 'Y':
                    if (memcmp(src_ptr, "YLW", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_YELLOW, 3);
                    }
                    break;
                case 'M':
                    if (memcmp(src_ptr, "MGT", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_MAGENTA, 3);
                    }
                    break;
                case 'C':
                    if (memcmp(src_ptr, "CYN", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_CYAN, 3);
                    }
                    break;
                case 'W':
                    if (memcmp(src_ptr, "WHT", 3) == 0) {
                        PROCESS_COLOR_SPEC(COLOR_WHITE, 3);
                    }
                    break;
            }

            buff[buff_pos++] = '%';
            continue;
        }
            
        buff[buff_pos++] = *src_ptr++;
    }

    buff[buff_pos] = '\0';
}