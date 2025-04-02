# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)
project(vascodagammaray)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIRS ON)

find_package(Qt6 6.5 NO_MODULE REQUIRED COMPONENTS Gui Network Quick)

if (ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(GenerateExportHeader)
add_library(vascodagammaray SHARED
    src/lib.cpp
)

generate_export_header(
    vascodagammaray EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/vascodagammaray_export.h" EXPORT_MACRO_NAME MYLIB_EXPORT
)

target_link_libraries(
    vascodagammaray Qt6::Gui Qt6::Network Qt6::Quick
)

target_include_directories(vascodagammaray PUBLIC ${CMAKE_BINARY_DIR})

enable_testing()

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
