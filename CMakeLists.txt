cmake_minimum_required(VERSION 3.10)

project(RungeKuttaSolver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(include)

add_executable(rk4
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
)

target_link_libraries(rk4 PRIVATE Qt6::Widgets)

if(APPLE)
    set_target_properties(rk4 PROPERTIES MACOSX_BUNDLE ON)
endif()
