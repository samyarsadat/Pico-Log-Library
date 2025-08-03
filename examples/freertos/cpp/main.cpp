/*
    Pico Log - FreeRTOS Example
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
#include "pico_log_lib/logger.h"
#include "FreeRTOS.h"
#include "task.h"


// FreeRTOS hooks
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    (void) xTask;
    (void) pcTaskName;
    panic("Stack overflow!");
}

void vApplicationMallocFailedHook() {
    panic("Memory allocation failed!");
}


// Logger configuration
logger_options_t logger_options = {
    .logging_level = LOG_LVL_DEBUG,
    .log_format = "[%TSTMP%] [%LVL%] [%FUNC%:%LINE%]: %MSG%",
    .ansi_styling = true,
    .process_style_tags = true  
};

// Logger initialization with USB stdio driver & logging macro
Logger logger(&stdio_usb, &logger_options);
#define LOG(lvl, msg, ...) logger.log(__func__, __FILE__, __LINE__, lvl, msg, ##__VA_ARGS__);

// Multi-threaded logging example
void log_task1(void* arg) {
    (void) arg;
    size_t counter = 0;
    uint32_t start_time, elapsed_time = 0;

    while (true) {
        start_time = time_us_32();
        LOG(LOG_LVL_DEBUG, "Task 1 heartbeat... iter: %YLW%%u%RST%, l-exec-t: %RED%%u%RST%us", counter, elapsed_time)
        elapsed_time = (time_us_32() - start_time);
        vTaskDelay(pdMS_TO_TICKS(1000));
        counter++;
    }
}

void log_task2(void* arg) {
    (void) arg;
    size_t counter = 0;
    uint32_t start_time, elapsed_time = 0;
    
    while (true) {
        start_time = time_us_32();
        LOG(LOG_LVL_FATAL, "Task 2 heartbeat... iter: %YLW%%u%RST%, l-exec-t: %RED%%u%RST%us", counter, elapsed_time)
        elapsed_time = (time_us_32() - start_time);
        vTaskDelay(pdMS_TO_TICKS(2000));
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

    // Example: changing the log format after initialization
    logger_options.log_format = "[%TSTMP%] [%LVL%] [%GRN%%BOLD%%TASK%%RST%:%FUNC%] [%CORE%]: %MSG%";
    logger.reparse_format();

    xTaskCreate(log_task1, "LogTask1", 1024, NULL, 1, NULL);
    xTaskCreate(log_task2, "LogTask2", 1024, NULL, 1, NULL);
    
    logger.init_mutex();  // Initialize logger mutex to ensure thread safety
    vTaskStartScheduler();

    return 0;  // Should never reach here
}