# (Modified) Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/usr/share/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(deja C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(deja main.c ignition.c)

# Set Project Name and Version
pico_set_program_name(deja "DEJA")
pico_set_program_version(deja "0.1")


# pull in common dependencies
target_link_libraries(deja pico_stdlib hardware_adc)

# enable usb serial output
pico_enable_stdio_usb(deja 1)
pico_enable_stdio_uart(deja 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(deja)
