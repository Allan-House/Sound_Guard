cmake_minimum_required(VERSION 3.0)

# Project metadata
project(Sound_Guard)
message("Project Name: ${PROJECT_NAME}")

# Project paths
message("Source Dir: ${CMAKE_SOURCE_DIR}")
message("Binary Dir: ${CMAKE_BINARY_DIR}")

# C++ standard settings
set(CMAKE_C_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# Output directory configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Header files path
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files path
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)

# Define the executable target with the collected source files
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})