<h1 align="center">Pico Log Library</h1>

<p align="center">
	<a href="LICENSE"><img src="https://img.shields.io/github/license/samyarsadat/Pico-Log-Library?color=blue"></a>
	|
	<a href="../../issues"><img src="https://img.shields.io/github/issues/samyarsadat/Pico-Log-Library"></a>
	<br><br>
</p>

<br><br>

----
This is a simple, and relatively performant logging library for RP2xxx-series microcontrollers. It supports color and style tags (ex. `%RED%`, `%BOLD%`, etc.) which are interpreted into ANSI escape codes, and a configurable logging format.

The logging format is parsed once upon initialization and tokenized for faster subsequent processing of log messages. ANSI styles and/or style tag interpretation can be optionally disabled if execution speed is a major concern.

It is also important to note that all of the memory required for the format tokens, message buffers, etc. is allocated once upon initialization, and so there are no subsequent memory allocation/de-allocation operations performed during logging.

When using FreeRTOS on the Pico, you can set `PICO_LOG_FREERTOS` to `ON` to enable FreeRTOS intergration. When this integration is enabled, a FreeRTOS semaphore is used as the locking mutex instead of built-in Pico SDK mutexes, and the `%TASK%` tag can be used in the log format to get the calling task's name.

The logging functions are fully thread-safe.

W.I.P.
<br>


<br>

## Contact
You can contact me via e-mail.<br>
E-mail: samyarsadat@gigawhat.net<br>
<br>
If you think that you have found a bug or issue please report it <a href="../../issues">here</a>.

<br>

## Credits
| Role           | Name                                                             |
| -------------- | ---------------------------------------------------------------- |
| Lead Developer | <a href="https://github.com/samyarsadat">Samyar Sadat Akhavi</a> |

<br>
<br>

Copyright © 2025 Samyar Sadat Akhavi.