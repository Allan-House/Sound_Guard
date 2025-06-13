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

# Source files for main project
file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/3rdparty/pre-compiled-libs/libwiringpi.a
    m
)

# ============================================================================
# DIAGNOSTIC TEST EXECUTABLE
# ============================================================================

# Create diagnostic test executable
add_executable(diagnostic_test
    ${CMAKE_SOURCE_DIR}/tests/diagnostic_main.c
)

# Link diagnostic test with wiringPi and math library
target_link_libraries(diagnostic_test
    ${CMAKE_SOURCE_DIR}/3rdparty/pre-compiled-libs/libwiringpi.a
    m
)

# Set output directory for diagnostic test
set_target_properties(diagnostic_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compile options for diagnostic test
target_compile_options(diagnostic_test PRIVATE -Wall -Wextra -O2)

# Custom target to run diagnostic tests
add_custom_target(run_diagnostic
    COMMAND sudo ${CMAKE_BINARY_DIR}/bin/diagnostic_test
    DEPENDS diagnostic_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Executando testes de diagnóstico (requer sudo)..."
)

# Custom target for quick diagnostic
add_custom_target(run_diagnostic_quick
    COMMAND sudo ${CMAKE_BINARY_DIR}/bin/diagnostic_test -q
    DEPENDS diagnostic_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Executando testes de diagnóstico rápidos (requer sudo)..."
)

# ============================================================================
# HELP TARGET
# ============================================================================

# Custom target to show available targets
add_custom_target(show_help
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== Sound Guard Build Targets ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Main targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Sound_Guard          - Compila o programa principal"
    COMMAND ${CMAKE_COMMAND} -E echo "  diagnostic_test      - Compila o programa de diagnóstico"
    COMMAND ${CMAKE_COMMAND} -E echo "  all                  - Compila tudo"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Test targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  run_diagnostic       - Executa todos os testes de diagnóstico"
    COMMAND ${CMAKE_COMMAND} -E echo "  run_diagnostic_quick - Executa testes rápidos de diagnóstico"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Usage examples:"
    COMMAND ${CMAKE_COMMAND} -E echo "  make Sound_Guard && sudo ./bin/Sound_Guard"
    COMMAND ${CMAKE_COMMAND} -E echo "  make diagnostic_test && sudo ./bin/diagnostic_test"
    COMMAND ${CMAKE_COMMAND} -E echo "  make run_diagnostic"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Mostrando ajuda dos targets disponíveis"
)