cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(mqtt)

set(WITH_PI_SENSE_HAT OFF CACHE STRING "Whether to build Pi Sense HAT example. Set to ON/OFF, default OFF. Requiers libsetila available on Github: https://github.com/positronic57/libsetila")


add_subdirectory(mqtt_pub)
add_subdirectory(mqtt_sub)

if (NOT WITH_PI_SENSE_HAT MATCHES "ON|OFF")
    message(FATAL_ERROR "WITH_PI_SENSE_HAT option must be ON or OFF")
endif()

# Look for libsetila library installation if option WITH_PI_SENSE_HAT is set to ON
if (WITH_PI_SENSE_HAT)
    find_library(LIBSETILA 
	NAMES libsetila setila
	PATH /usr/local/lib
        )
    if (LIBSETILA STREQUAL LIBSETILA-NOTFOUND)
        message(FATAL_ERROR "Library libsetila not found. \
        It is required for WITH_PI_SENSE_HAT option. \
        Available on Github: https://github.com/positronic57/libsetila. \
        Please install it before proceed with the build.")
     endif()
    add_subdirectory(mqtt_pub_sense_hat)
endif()
