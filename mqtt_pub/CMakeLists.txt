cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Define the project name
project(mqtt_pub)

# Define the destination for the binary object
set (BUILD_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Set the C compiler flags for Debug build.
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall -fmessage-length=0")

# Set the C compiler flags for Release build.
set(CMAKE_C_FLAGS_RELEASE "-O0 -Wall -fmessage-length=0")

# Define the location/search path for the libraries
set(MQTT_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

# Define the include directory
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../mqtt_includes
)

# Define the list of source files
set (SOURCE_LIST
mqtt_pub.c
${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
)

# The libraries are located here
link_directories(${MQTT_LIBS_PATH})

# Build mqtt_pub binary from the files in the SOURCE_LIST
add_executable(mqtt_pub ${SOURCE_LIST})

# Link the binary to the following libraries
target_link_libraries(mqtt_pub mosquitto)

# Create target directories
install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin)

install (TARGETS mqtt_pub
	RUNTIME DESTINATION ${BUILD_DESTINATION}/bin
)	

