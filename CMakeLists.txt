cmake_minimum_required(VERSION 3.10)

# Project metadata
project(Sound_Guard C)

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary Dir: ${CMAKE_BINARY_DIR}")

# C standard settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directory configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Header files
include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/WiringPi/wiringpi)

# Source files
file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# Executável
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/3rdparty/pre-compiled-libs/libwiringpi.a
)
