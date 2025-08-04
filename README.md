<h1 align="center">Pico Log Library</h1>

<p align="center">
	<a href="LICENSE"><img src="https://img.shields.io/github/license/samyarsadat/Pico-Log-Library?color=blue"></a>
	|
	<a href="../../issues"><img src="https://img.shields.io/github/issues/samyarsadat/Pico-Log-Library"></a>
	<br><br>
</p>

<br><br>

----
This is a simple and relatively performant logging library for RP2xxx-series microcontrollers. It supports color and style tags (e.g., `%RED%`, `%BOLD%`, etc.), which are interpreted into ANSI escape codes, and a configurable logging format.

The logging format is parsed once upon initialization and tokenized for faster subsequent processing of log messages. ANSI styles and/or style tag interpretation can be optionally disabled if execution speed is a major concern.

It is also important to note that all of the memory required for the format tokens, message buffers, etc. is allocated once upon initialization (well, they're member variables of the `Logger` class itself), and so there are no subsequent memory allocation/de-allocation operations performed during logging.

The logging functions are fully thread-safe, assuming successful mutex initialization.

<br>

# Library Usage
## C-only API
> [!NOTE]
> New in v0.2.0.

The library now has a C-only API which can be used to achieve all of the same functionality as the C++ class-based API, but from C code. The function documentation can be found [here](include/pico_log_lib/logger_c.h), though it is very similar to the C++ API in most aspects.

New [examples](examples) have also been added, showcasing the C API both with and without FreeRTOS.

<br>

## Using The Library
To use this library in your project, you can simply add this repoistory as a submodule and include its directory in your CMake build configuration..

When using FreeRTOS on the Pico, you can set `PICO_LOG_FREERTOS` to `ON` to enable FreeRTOS integration. When this integration is enabled, a FreeRTOS semaphore is used as the locking mutex instead of built-in Pico SDK mutexes, and the `%TASK%` tag can be used in the log format to get the calling task's name.

If you are using FreeRTOS, make sure to properly "include" FreeRTOS in your build configuration (setting `FREERTOS_KERNEL_PATH`, including the FreeRTOS config directory, including the `FreeRTOS_Kernel_import.cmake` file) before adding this library as a subdirectory.

<br>

## Logger Configuration
The logger’s configuration options are set using the `logger_options` structure.

```c
typedef struct {
    LOG_LEVEL logging_level;
    const char* log_format;
    bool ansi_styling;
    bool process_style_tags;
} logger_options_t;
```

`LOG_LEVEL_t logging_level`:\
The minimum severity required for a message to be logged.\
Note that the following verbosity levels are available:
```c
typedef enum {
    LOG_LVL_DEBUG, 
    LOG_LVL_INFO, 
    LOG_LVL_WARN, 
    LOG_LVL_ERROR, 
    LOG_LVL_FATAL
} LOG_LEVEL_t;
```

`const char* log_format`:\
The format for log messages. Certain special tags can be used here for getting timestamps, log level, etc.

The following tags are supported:

| Tag       | Function                                                                                                                |
|-----------|-------------------------------------------------------------------------------------------------------------------------|
| `%TSTMP%` | Time since boot, formatted as `SECONDS.MILLISECONDS`.                                                                   |
| `%LVL%`   | The severity level of the log message.                                                                                  |
| `%FILE%`  | The name of the source file from which the logger is called.                                                            |
| `%LINE%`  | The line number (within the source file) from which the logger is called.                                               |
| `%FUNC%`  | The name of the function from which the logger is called.                                                               |
| `%TASK%`  | The name of the FreeRTOS task from which the logger is called. Defaults to `NO TASK` when FreeRTOS support is disabled. |
| `%CORE%`  | The core number from which the logger is called. Either `core0` or `core1`.                                             |
| `%MSG%`   | The actual log message passed to the logger by the user.                                                                |

In addition to these tags, all of the [styling tags](#style-tags) are also supported.

`bool ansi_styling`:\
Enables or disables ANSI styling escape codes. If disabled, all styling tags in the log format are ignored, but style tags in log messages are left uninterpreted.

If you are planning on disabling ANSI styling, it is a good idea to not use styling tags in log messages at all, as those tags are left as they are, and because tags are enclosed in `%` and the log message is passed to `vsnprint()` for variable formatting, you may notice undesirable artifacts in log messages. Note that `%` symbols can be escaped by using another `%` symbol (i.e., `%%`).

Disabling this option will decrease the execution time of the logging function, which may be desirable in certain situations.

`bool process_style_tags`:\
If you want to retain styling for the log format but don’t plan on using styling tags in log messages, you can disable this option. This only prevents style tags in log messages from being processed/interpreted.

Disabling this option will decrease the execution time of the logging function as well, though disabling `ansi_styling` is the best option if execution speed is absolutely critical.

> [!NOTE]
> Even with these options enabled, the logging functions are still quite fast. When using serial over USB on the RP2040 (FreeRTOS) with styling tags and variable substitutions, a typical log format (`[%TSTMP%] [%LVL%] [%FUNC%:%LINE%] [%GRN%%BOLD%%TASK%%RST%]: %MSG%`), and this log message: `Task 1 heartbeat... iter: %YLW%%u%RST%, l-exec-t: %RED%%u%RST%us`, I measured an average total execution time of around 400~450 microseconds. This was with GCC 13.2.1, and compiler optimizations were enabled.

<br>

## Function Documentation
These are all of the public member methods of the Logger class.

### Constructor
```cpp
Logger(stdio_driver_t* stdio_driver, logger_options_t* options);
```
The constructor of the `Logger` class takes two arguments. One is a pointer to a `logger_options` structure (see [logger configuration](#logger-configuration)), and the other is a pointer to a Pico SDK `stdio_driver` structure.

There are `stdio_driver` structures defined for both UART and USB in the Pico SDK. If you intend on outputing log messages over UART, you can reference `stdio_uart` (`&stdio_uart`), and for USB you can reference `stdio_usb` (`&stdio_usb`). Just make sure that the respective STDIO output is enabled in your CMake configuration:

```cmake
# For STDIO over USB:
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# For STDIO over UART:
pico_enable_stdio_uart(${PROJECT_NAME} 1)
```

You must also call `stdio_init_all()` before using the logger to initialize the STDIO output (there are functions for specifically initializing UART STDIO or USB STDIO, but calling `stdio_init_all()` is the recommended approach).

If you want log messages to be output to some other destination, you can declare your own `stdio_driver` structure. You only need to provide an implementation for `void (*out_chars)(const char *buf, int len)`, as that is the function used by the logger to output messages.

<br>

### `void log(...)`
```cpp
void log(const char* func, const char* file, const uint16_t line, LOG_LEVEL level, const char* message, ...);
```
This is the main logging function. It is thread-safe (assuming that `init_mutex()` has been called and that the mutex has been successfully created), and it does not perform any memory allocation. All of [these](#style-tags) styling tags are supported in the logging function message, in addition to normal `sprintf()` variable substitutions (i.e., `%d`, `%s`, `%f`, etc.).

It is recommended for you to define a logging macro so that you don’t have to pass the function name, file name, and line number every time:

```cpp
// Instead of this:
logger.log(__func__, __FILE__, __LINE__, LOG_LVL_INFO, "Hello world!");

// You can define a macro:
#define LOG(lvl, msg, ...) logger.log(__func__, __FILE__, __LINE__, lvl, msg, ##__VA_ARGS__);

// Then whenever you want to write a log message:
LOG(LOG_LVL_INFO, "Hello world!")
```

<br>

### `void vlog(...)`
```cpp
void vlog(LOG_LEVEL level, const char* message, va_list args, const char* func, const char* file, const uint16_t line)
```
Same as `log()`, except it takes a `va_list`.

<br>

### `bool init_mutex()`
This initializes the logging mutex to ensure thread-safe logging operation. When using FreeRTOS, it creates a FreeRTOS Semaphore Mutex; otherwise, it uses Pico SDK's built-in mutexes.

You can still use the logging function without initializing the mutex if you are not planning on using multiple cores/threads.

**RETURN VALUE:**\
Always returns `true` when using Pico SDK mutexes and returns `true` only if the mutex was created successfully when using FreeRTOS.

<br>

### `bool reparse_format()`
The log format is parsed once upon the creation of the logger object. If the log format or ANSI styling configuration are changed at some point after the creation of the logger object, you must make sure to call `reparse_format()` for the changes to take effect.

**RETURN VALUE:**\
`true` if the format was successfully reparsed, `false` if the mutex could not be acquired.

> [!WARNING]
> When the log format is parsed, a copy of it is not made. Instead, the parser will reference memory addresses to which the `log_format` points. For this reason, if the log format is changed without immediately calling `reparse_format()`, references to no longer valid memory addresses may be retained.

<br>

## Style Tags
The following special styling tags are supported, both for the log format and for the log message:

| Tag        | Function                           | ANSI Equivalent |
|------------|------------------------------------|-----------------|
| `%BLK%`    | Sets text color to black.          | `\033[0;30m`    |
| `%RED%`    | Sets text color to red.            | `\033[0;31m`    |
| `%GRN%`    | Sets text color to green.          | `\033[0;32m`    |
| `%YLW%`    | Sets text color to yellow.         | `\033[0;33m`    |
| `%BLU%`    | Sets text color to blue.           | `\033[0;34m`    |
| `%MGT%`    | Sets text color to magenta.        | `\033[0;35m`    |
| `%CYN%`    | Sets text color to cyan.           | `\033[0;36m`    |
| `%WHT%`    | Sets text color to white.          | `\033[0;37m`    |
| `%BOLD%`   | Sets text style to bold.           | `\033[1m`       |
| `%ITL%`    | Sets text style to italic.         | `\033[3m`       |
| `%UDRLN%`  | Sets text style to underlined.     | `\033[4m`       |
| `%STKTHR%` | Sets text style to strike through. | `\033[9m`       |
| `%RST%`    | Resets text style and color.       | `\033[0m`       |

Additionally, the following suffixes are also supported for all of the color tags:

| Suffix | Function                                                                           |
|--------|------------------------------------------------------------------------------------|
| `_HI`  | Changes the color to its high-intensity variant.                                   |
| `_BG`  | Makes the tag modify the background color of the text instead of the text’s color. |

Note that both suffixes can be used at the same time (ex. `%RED_HI_BG%`).

<br>

## Contact
You can contact me via e-mail.\
E-mail: samyarsadat@gigawhat.net

If you think that you have found a bug or issue please report it <a href="../../issues">here</a>.

<br>

## Credits
| Role           | Name                                                             |
| -------------- | ---------------------------------------------------------------- |
| Lead Developer | <a href="https://github.com/samyarsadat">Samyar Sadat Akhavi</a> |

<br>
<br>

Copyright © 2025 Samyar Sadat Akhavi.