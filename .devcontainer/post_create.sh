#!/usr/bin/env bash
# Devcontainer post-create script.
# This script is run after the devcontainer is created.

set -e
echo "--> post_create.sh started!"
echo "-> First container run, running setup..."

echo "-> Setting folder permissions..."
sudo chown -R nonroot: $HOME/pico_ws/

echo "-> Donwloading the FreeRTOS Kernel..."
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel $HOME/pico_ws/libfreertos/FreeRTOS-Kernel
cp "$HOME/pico_ws/libfreertos/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake" \
    $HOME/pico_ws/FreeRTOS_Kernel_import.cmake

echo "-> Installing ELF size analyzer..."
pip install --no-warn-script-location --break-system-packages elf-size-analyze
echo "export PATH='$PATH:$HOME/.local/bin'" >> $HOME/.bashrc

echo "--> post_create.sh done!"