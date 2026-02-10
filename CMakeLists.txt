cmake_minimum_required(VERSION 3.16)
project(calculator C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put all executables in the top-level build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Options
option(BUILD_GUI "Build Qt6 GUI application" ON)
option(BUILD_CLI "Build CLI application" ON)
option(BUILD_TESTS "Build test suite" ON)

# --- Find dependencies ---

# MPFR and GMP (required)
find_library(MPFR_LIBRARY mpfr REQUIRED)
find_library(GMP_LIBRARY gmp REQUIRED)
find_path(MPFR_INCLUDE_DIR mpfr.h)
find_path(GMP_INCLUDE_DIR gmp.h)

# Math library (required on Linux, not needed on Windows)
include(CheckLibraryExists)
check_library_exists(m sin "" HAVE_LIBM)
if(HAVE_LIBM)
    set(MATH_LIBRARY m)
endif()

# Readline (optional, CLI only)
find_library(READLINE_LIBRARY readline)
find_path(READLINE_INCLUDE_DIR readline/readline.h)

# --- calc_core static library ---

set(CORE_SOURCES
    src/core/evaluator.c
    src/core/functions.c
    src/core/precision.c
    src/core/constants.c
    src/lexer/lexer.c
    src/lexer/tokens.c
    src/lexer/function_table.c
    src/parser/parser.c
    src/parser/parser_utils.c
    src/parser/ast.c
    src/output/formatter.c
    src/output/printer.c
)

add_library(calc_core STATIC ${CORE_SOURCES})

target_include_directories(calc_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/output
    ${CMAKE_SOURCE_DIR}/src/ui
)

if(MPFR_INCLUDE_DIR)
    target_include_directories(calc_core PUBLIC ${MPFR_INCLUDE_DIR})
endif()
if(GMP_INCLUDE_DIR)
    target_include_directories(calc_core PUBLIC ${GMP_INCLUDE_DIR})
endif()

target_link_libraries(calc_core PUBLIC
    ${MPFR_LIBRARY}
    ${GMP_LIBRARY}
    ${MATH_LIBRARY}
)

target_compile_options(calc_core PRIVATE -Wall -Wextra -pedantic)

# --- CLI executable ---

if(BUILD_CLI)
    set(CLI_SOURCES
        src/ui/main.c
        src/ui/repl.c
        src/ui/input.c
        src/ui/commands.c
    )

    add_executable(calculator-cli ${CLI_SOURCES})
    target_link_libraries(calculator-cli PRIVATE calc_core)
    target_compile_options(calculator-cli PRIVATE -Wall -Wextra -pedantic)

    if(READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
        target_compile_definitions(calculator-cli PRIVATE HAVE_READLINE)
        target_include_directories(calculator-cli PRIVATE ${READLINE_INCLUDE_DIR})
        target_link_libraries(calculator-cli PRIVATE ${READLINE_LIBRARY})
        message(STATUS "Readline support: enabled")
    else()
        message(STATUS "Readline support: disabled (install libreadline-dev for history support)")
    endif()
endif()

# --- Test executable ---

if(BUILD_TESTS)
    set(TEST_SOURCES
        tests/test_runner.c
        tests/test_lexer.c
        tests/test_parser.c
        tests/test_evaluator.c
        tests/test_precision.c
        tests/test_calculator.c
        tests/test_integration.c
        src/ui/repl.c
        src/ui/input.c
        src/ui/commands.c
    )

    add_executable(test_calculator ${TEST_SOURCES})
    target_compile_definitions(test_calculator PRIVATE TESTING)
    target_link_libraries(test_calculator PRIVATE calc_core)
    target_compile_options(test_calculator PRIVATE -Wall -Wextra -pedantic)

    if(READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
        target_compile_definitions(test_calculator PRIVATE HAVE_READLINE)
        target_include_directories(test_calculator PRIVATE ${READLINE_INCLUDE_DIR})
        target_link_libraries(test_calculator PRIVATE ${READLINE_LIBRARY})
    endif()

    enable_testing()
    add_test(NAME all_tests COMMAND test_calculator)
    add_test(NAME lexer_tests COMMAND test_calculator lexer)
    add_test(NAME parser_tests COMMAND test_calculator parser)
    add_test(NAME evaluator_tests COMMAND test_calculator evaluator)
    add_test(NAME precision_tests COMMAND test_calculator precision)
    add_test(NAME integration_tests COMMAND test_calculator integration)
endif()

# --- Qt6 GUI executable ---

if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets)
    if(Qt6_FOUND)
        message(STATUS "Qt6 found: building GUI")
        add_subdirectory(src/gui)
    else()
        message(STATUS "Qt6 not found: skipping GUI build (install qt6-base-dev)")
        set(BUILD_GUI OFF)
    endif()
endif()

# --- Summary ---

message(STATUS "")
message(STATUS "Build configuration:")
message(STATUS "  CLI:   ${BUILD_CLI}")
message(STATUS "  GUI:   ${BUILD_GUI}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "")
