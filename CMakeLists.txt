add_executable(screen_test
        screen_test.c
        )

# pull in common dependencies
target_link_libraries(screen_test pico_stdlib hardware_adc)

# enable usb serial output
pico_enable_stdio_usb(screen_test 1)
pico_enable_stdio_uart(screen_test 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(screen_test)
