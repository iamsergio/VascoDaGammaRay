# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.21)
project(test)

option (USE_QT6 "Use Qt6" ON)

if (USE_QT6)
    find_package(Qt6 6.5 NO_MODULE REQUIRED COMPONENTS Quick)
    set(QTMAJOR 6)
else()
    find_package(Qt5 5.15 NO_MODULE REQUIRED COMPONENTS Quick)
    set(QTMAJOR 5)
endif()


qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(
    testvasco
    main.cpp
    resources.qrc
)

target_link_libraries(testvasco PUBLIC Qt${QTMAJOR}::Quick vascodagammaray)
