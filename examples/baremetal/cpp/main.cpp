/*
    Pico Log - Baremetal Example
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

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "pico_log_lib/logger.h"


// Logger configuration
logger_options_t logger_options = {
    .logging_level = LOG_LVL_DEBUG,
    .log_format = "[%TSTMP%] [%LVL%] [%FUNC%:%LINE%] [%GRN%%BOLD%%CORE%%RST%]: %MSG%",
    .ansi_styling = true,
    .process_style_tags = true  
};

// Logger initialization with USB stdio driver & logging macro
Logger logger(&stdio_usb, &logger_options);
#define LOG(lvl, msg, ...) logger.log(__func__, __FILE__, __LINE__, lvl, msg, ##__VA_ARGS__);

// Core 1 logger
void core1_entry() {
    size_t counter = 0;
    uint32_t start_time, elapsed_time = 0;
    
    while (true) {
        start_time = time_us_32();
        LOG(LOG_LVL_FATAL, "Core 1 heartbeat... iter: %YLW%%u%RST%, l-exec-t: %RED%%u%RST%us", counter, elapsed_time)
        elapsed_time = (time_us_32() - start_time);
        sleep_ms(2000);
        counter++;
    }
}

// Core 0 logger
void core0_entry() {
    size_t counter = 0;
    uint32_t start_time, elapsed_time = 0;

    while (true) {
        start_time = time_us_32();
        LOG(LOG_LVL_DEBUG, "Core 0 heartbeat... iter: %YLW%%u%RST%, l-exec-t: %RED%%u%RST%us", counter, elapsed_time)
        elapsed_time = (time_us_32() - start_time);
        sleep_ms(1000);
        counter++;
    }
}


int main() {
    stdio_init_all();
    
    // Wait for USB connection
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    
    // Example: logging before mutex initialization
    LOG(LOG_LVL_DEBUG, "This is a %CYN%debug%RST% message.")
    LOG(LOG_LVL_INFO, "Info level log with %GRN%color%RST%.")
    LOG(LOG_LVL_WARN, "This is a warning. %YLW%Very yellow.%RST%")
    LOG(LOG_LVL_ERROR, "%RED%Error encountered!%RST%")
    LOG(LOG_LVL_FATAL, "%MGT%Fatal situation!%RST%")
    LOG(LOG_LVL_INFO, "To %BOLD%boldly go%RST% where %UDRLN%no one%RST% has gone before.")
    LOG(LOG_LVL_INFO, "%ITL%This is italic text%RST%.")
    LOG(LOG_LVL_INFO, "%STKTHR%This is strikethrough text%RST%.")
    LOG(LOG_LVL_INFO, "%BLU_BG%Background colors!%RST%")

    // Initialize logger mutex to ensure thread safety (not really threads, but cores)
    logger.init_mutex();
    
    // Launch core 1
    LOG(LOG_LVL_INFO, "Launching core 1...");
    multicore_launch_core1(core1_entry);
    
    // Core 0 loop
    core0_entry();

    return 0;
}