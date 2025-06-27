cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(mqtt)

set(WITH_PI_SENSE_HAT OFF CACHE STRING "Whether to build Pi Sense HAT example. Set to ON/OFF, default OFF. Requiers libsetila available on Github: https://github.com/positronic57/libsetila")

set(WITH_HA_EXAMPLE OFF CASE STRING "Whether to build Home Assistant example. Set to ON/OFF, default OFF. Requires libsetila available on Github: https://github.com/positronic57/libsetila and libjsoncpp")
 
add_subdirectory(mqtt_pub)
add_subdirectory(mqtt_sub)

if (NOT WITH_PI_SENSE_HAT MATCHES "ON|OFF")
    message(FATAL_ERROR "WITH_PI_SENSE_HAT option must be ON or OFF")
endif()

if (NOT WITH_HA_EXAMPLE MATCHES "ON|OFF")
    message(FATAL_ERROR "WITH_HA_EXAMPLE option must be ON or OFF")
endif()

if (WITH_PI_SENSE_HAT)
  add_subdirectory(mqtt_pub_sense_hat)
endif()

if (WITH_HA_EXAMPLE)
  add_subdirectory(mqtt_ha)
endif()

