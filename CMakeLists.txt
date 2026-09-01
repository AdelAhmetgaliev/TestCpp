cmake_minimum_required(VERSION 3.26)

project(
    TestCpp
    VERSION 0.1.0
    LANGUAGES CXX
)

# Single source of truth: every identifier below derives from PROJECT_NAME,
# so renaming the template only requires editing the project() call above
# (or running scripts/rename.sh).
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
string(REGEX REPLACE "[^A-Z0-9]" "_" PROJECT_NAME_UPPER "${PROJECT_NAME_UPPER}")

# ============================================================================
# Compiler and standard
# ============================================================================

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|MSVC")
    message(WARNING "Compiler ${CMAKE_CXX_COMPILER_ID} has not been tested."
                    "Use Clang, GCC or MSVC.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Build options
# ============================================================================

option(ENABLE_LTO "Enable Link Time Optimization" ON)
option(ENABLE_STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(ENABLE_SANITIZERS "Enable ASan+UBSan" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during the build" OFF)
option(ENABLE_TESTS "Build and register tests" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release
                 RelWithDebInfo MinSizeRel)
endif()

# ============================================================================
# Output directories
# ============================================================================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep a compile_commands.json symlink at the source root for clangd;
# regenerated on every configure, so it survives `rm -rf build`.
# The last configured preset owns the link.
if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT WIN32)
    set(_compile_commands "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json")
    if(EXISTS "${_compile_commands}")
        file(REMOVE "${_compile_commands}")
    endif()
    file(CREATE_LINK "${CMAKE_BINARY_DIR}/compile_commands.json"
                     "${_compile_commands}" SYMBOLIC)
endif()

# ============================================================================
# Compilation flags
# ============================================================================

add_library(${PROJECT_NAME}_warnings INTERFACE)
add_library(${PROJECT_NAME}::warnings ALIAS ${PROJECT_NAME}_warnings)

if(ENABLE_STRICT_WARNINGS)
    if(MSVC)
        target_compile_options(${PROJECT_NAME}_warnings INTERFACE
            /W4 /WX
            /permissive-
            /external:anglebrackets /external:W0
        )
    else()
        target_compile_options(${PROJECT_NAME}_warnings INTERFACE
            -Wall -Wextra -Wpedantic -Werror
            -Wformat -Wformat=2
            -Wconversion
            -Wimplicit-fallthrough
            -Werror=format-security
            -Wshadow
            -Wunused
        )
    endif()
endif()

add_library(${PROJECT_NAME}_hardening INTERFACE)
add_library(${PROJECT_NAME}::hardening ALIAS ${PROJECT_NAME}_hardening)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND
    CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(${PROJECT_NAME}_hardening INTERFACE
        $<$<NOT:$<CONFIG:Debug>>:-U_FORTIFY_SOURCE>
        $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=3>
        -D_GLIBCXX_ASSERTIONS
        -fstack-protector-strong
        $<$<CXX_COMPILER_ID:GNU>:--param ssp-buffer-size=4>
        -fstack-clash-protection
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        -fstrict-flex-arrays=3
    )

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|x86|amd64")
        target_compile_options(${PROJECT_NAME}_hardening INTERFACE
            -fcf-protection=full
        )
    endif()

    target_link_options(${PROJECT_NAME}_hardening INTERFACE
        $<$<CONFIG:Release>:-s>
        -Wl,-z,nodlopen
        -Wl,-z,noexecstack
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,defs
        -Wl,--as-needed
        -Wl,--no-copy-dt-needed-entries
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME}_hardening INTERFACE
        /guard:cf
        /guard:ehcont
        /sdl
        /DYNAMICBASE
        /NXCOMPAT
        /CETCOMPAT
    )

    target_link_options(${PROJECT_NAME}_hardening INTERFACE
        /guard:cf
        /guard:ehcont
        /DYNAMICBASE
        /NXCOMPAT
    )
endif()

# ============================================================================
# Sanitizers (ASan + UBSan)
# ============================================================================

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        include(CheckCXXSourceCompiles)

        if(NOT DEFINED SANITIZERS_AVAILABLE)
            set(CMAKE_REQUIRED_FLAGS "-fsanitize=address,undefined")
            check_cxx_source_compiles("int main() { return 0; }"
                                      SANITIZERS_AVAILABLE)
            unset(CMAKE_REQUIRED_FLAGS)
            set(SANITIZERS_AVAILABLE ${SANITIZERS_AVAILABLE}
                CACHE BOOL "Whether ASan+UBSan are available on this toolchain"
                           FORCE)
        endif()

        if(SANITIZERS_AVAILABLE)
            add_library(${PROJECT_NAME}_sanitizers INTERFACE)
            add_library(${PROJECT_NAME}::sanitizers ALIAS
                        ${PROJECT_NAME}_sanitizers)

            target_compile_options(${PROJECT_NAME}_sanitizers INTERFACE
                -fsanitize=address,undefined
                -fno-sanitize-recover=undefined
                -fno-omit-frame-pointer
            )

            target_link_options(${PROJECT_NAME}_sanitizers INTERFACE
                -fsanitize=address,undefined
            )
        else()
            message(WARNING "ASan+UBSan unavailable: the test program "
                            "does not link with -fsanitize=address,undefined. "
                            "Install the runtime (libasan) or use Clang.")
        endif()
    else()
        message(WARNING "Sanitizers are supported only for GCC and Clang")
    endif()
endif()

# ============================================================================
# clang-tidy (separate target; does not affect try_compile)
# ============================================================================

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-19
                 clang-tidy-18 clang-tidy-17 clang-tidy-16)

    find_program(CLANG_TIDY_RUN NAMES run-clang-tidy run-clang-tidy-19
                 run-clang-tidy-18 run-clang-tidy-17)

    if(CLANG_TIDY_EXECUTABLE)
        file(GLOB_RECURSE TIDY_SOURCES CONFIGURE_DEPENDS
             "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

        if(CLANG_TIDY_RUN)
            add_custom_target(clang-tidy ALL
                COMMAND ${CLANG_TIDY_RUN}
                        -p ${CMAKE_BINARY_DIR}
                        -quiet
                        -j 4
                        ${TIDY_SOURCES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Running clang-tidy (parallel)"
            )
        else()
            add_custom_target(clang-tidy ALL
                COMMAND ${CLANG_TIDY_EXECUTABLE}
                        -p ${CMAKE_BINARY_DIR}
                        ${TIDY_SOURCES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Running clang-tidy (sequential fallback)"
            )
        endif()
    else()
        message(WARNING "ENABLE_CLANG_TIDY=ON, but clang-tidy "
                        "was not found in PATH")
    endif()
endif()

# ============================================================================
# Library
# ============================================================================

add_library(${PROJECT_NAME}_lib STATIC
    source/lib/greeter.cpp
)
add_library(${PROJECT_NAME}::lib ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib
)

target_compile_definitions(${PROJECT_NAME}_lib PUBLIC
    PROJECT_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(${PROJECT_NAME}_lib PUBLIC
    ${PROJECT_NAME}::warnings
    ${PROJECT_NAME}::hardening
)

if(TARGET ${PROJECT_NAME}::sanitizers)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC
        ${PROJECT_NAME}::sanitizers)
endif()

# ============================================================================
# Main executable
# ============================================================================

add_executable(${PROJECT_NAME}_exe
    source/app/main.cpp
)

target_compile_definitions(${PROJECT_NAME}_exe PRIVATE
    $<$<CONFIG:Debug>:${PROJECT_NAME_UPPER}_DEBUG>
)

target_link_libraries(${PROJECT_NAME}_exe PRIVATE
    ${PROJECT_NAME}::lib
)

# ============================================================================
# Tests (Catch2 v3 via FetchContent; a system Catch2 is used if found)
# ============================================================================

if(ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.16.0
        FIND_PACKAGE_ARGS NAMES Catch2
    )

    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(catch2)

    enable_testing()

    add_executable(${PROJECT_NAME}_tests
        tests/greeter.test.cpp
    )

    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        ${PROJECT_NAME}::lib
        Catch2::Catch2WithMain
    )

    add_test(NAME ${PROJECT_NAME}.tests COMMAND ${PROJECT_NAME}_tests)
endif()

# ============================================================================
# LTO
# ============================================================================

macro(testcpp_enable_lto target)
    if(ENABLE_LTO AND ipo_supported)
        set_property(TARGET ${target} PROPERTY
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
        )
        set_property(TARGET ${target} PROPERTY
            INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
        )
    endif()
endmacro()

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output LANGUAGES CXX)

    if(ipo_supported)
        testcpp_enable_lto(${PROJECT_NAME}_lib)
        testcpp_enable_lto(${PROJECT_NAME}_exe)
        if(TARGET ${PROJECT_NAME}_tests)
            testcpp_enable_lto(${PROJECT_NAME}_tests)
        endif()
    else()
        message(WARNING "LTO not supported: ${ipo_output}")
    endif()
endif()

message(STATUS "=========================================")
message(STATUS "  ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "=========================================")
message(STATUS "CMake version:      ${CMAKE_VERSION}")
message(STATUS "Compiler:           ${CMAKE_CXX_COMPILER_ID} "
                                   "${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:       ${CMAKE_CXX_STANDARD}")
message(STATUS "=========================================")
