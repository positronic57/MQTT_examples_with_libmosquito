cmake_minimum_required(VERSION 3.5)

# Define the project name
project(mqtt_pub_ha_sub)

# Define the destination for the binary object
set (BUILD_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Set the C++ compiler flags. Enable C++11 standard.
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O0 -Wall -fmessage-length=0")

# Set the C++ compiler flags. Enable C++11 standard.
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -O0 -g3 -Wall -fmessage-length=0")

# Look for libjsoncpp-dev installation
find_package(jsoncpp REQUIRED)
get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)

# Look for libsetila installation
find_library(LIBSETILA
  NAMES libsetila setila
  PATH /usr/local/lib
)
 
if (LIBSETILA STREQUAL LIBSETILA-NOTFOUND)
  message(FATAL_ERROR "Library libsetila not found. \
  Available on Github: https://github.com/positronic57/libsetila. \
  Please install it before proceed with the build.")
endif()

include_directories(${JSON_INC_PATH})

# Define the list of source files
set (SOURCE_LIST
mqtt_pub_ha_sub.cpp
)

# Create mqtt_pub_ha_ssub binary using the files from the SOURCE_LIST
add_executable(mqtt_pub_ha_sub ${SOURCE_LIST})

# Link the binary with the following libraries
target_link_libraries(mqtt_pub_ha_sub mosquitto setila jsoncpp_lib)

# Create target directories
install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin)

install (TARGETS mqtt_pub_ha_sub
	RUNTIME DESTINATION ${BUILD_DESTINATION}/bin
)	

