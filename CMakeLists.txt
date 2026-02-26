cmake_minimum_required(VERSION 3.10)

project(RungeKuttaSolver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

include_directories(
    include
    3rdparty/qcustomplot
)

add_executable(rk4
    src/main.cpp
    
    src/interface/mainwindow.cpp
    include/interface/mainwindow.h
    src/interface/mainwindow.ui

    include/SimulationData.h
    include/TaskSolver.h
    src/TaskSolver.cpp
    
    include/math/Vector.hpp
    include/math/RK4Solver.hpp

    3rdparty/qcustomplot/qcustomplot.cpp
    3rdparty/qcustomplot/qcustomplot.h
)

target_link_libraries(rk4 PRIVATE Qt6::Widgets Qt6::PrintSupport)

if(APPLE)
    set_target_properties(rk4 PROPERTIES MACOSX_BUNDLE ON)
endif()
