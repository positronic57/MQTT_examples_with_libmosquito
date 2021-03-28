cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Define the project name
project(mqtt_sub)

# Define the destination for the binary object
set (BUILD_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Set the C compiler flags for Debug build.
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall -fmessage-length=0")

# Set the C compiler flags for Release build.
set(CMAKE_C_FLAGS_RELEASE "-O0 -Wall -fmessage-length=0")

# Define the include directory
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../mqtt_includes
)

# Define the list of source files
set (SOURCE_LIST
mqtt_sub.c
${CMAKE_CURRENT_SOURCE_DIR}/../worker/worker.c
${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
)

# Create mqtt_sub binary
add_executable(mqtt_sub ${SOURCE_LIST})

# Link the binary with the following libraries
target_link_libraries(mqtt_sub mosquitto rt pthread)

# Create target directories
install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin)

install (TARGETS mqtt_sub
	RUNTIME DESTINATION ${BUILD_DESTINATION}/bin
)	

