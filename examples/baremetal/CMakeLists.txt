message("Configuring Pico Log baremetal example builds...")
project(pico_log_example VERSION 0.2.0 LANGUAGES C CXX ASM)

add_subdirectory(cpp)
add_subdirectory(c)