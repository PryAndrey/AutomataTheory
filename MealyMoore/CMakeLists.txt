cmake_minimum_required(VERSION 3.24)
project(MealyMoore)

set(CMAKE_CXX_STANDARD 20)

add_executable(MealyMoore MM/main.cpp
        MM/MMV3/MealyGraph.cpp
        MM/MMV3/MooreGraph.cpp)

add_executable(MinimizeMM MMM/main.cpp
        MMM/MealyGraph.cpp
        MMM/MooreGraph.cpp)
