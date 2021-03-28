cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Define the project name
project(mqtt_pub_sense_hat)

# Define the destination for the binary object
set (BUILD_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Set the C++ compiler flags. Enable C++11 standard.
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O0 -Wall -fmessage-length=0")

# Set the C++ compiler flags. Enable C++11 standard.
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -O0 -g3 -Wall -fmessage-length=0")

# Define the location/search path for the libraries
set(MQTT_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

# Define the include directory
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../mqtt_includes
)

# Define the list of source files
set (SOURCE_LIST
mqtt_pub_sense_hat.cpp
${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
)

# The libraries are located here
link_directories(${MQTT_LIBS_PATH})

# Create mqtt_pub_sense_hat binary using the files from the SOURCE_LIST 
add_executable(mqtt_pub_sense_hat ${SOURCE_LIST})

# Link the binary with the following libraries
target_link_libraries(mqtt_pub_sense_hat mosquitto setila)

# Create target directories
install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin)

install (TARGETS mqtt_pub_sense_hat
	RUNTIME DESTINATION ${BUILD_DESTINATION}/bin
)	

