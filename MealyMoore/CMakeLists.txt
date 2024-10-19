cmake_minimum_required(VERSION 3.24)
project(MealyMoore)

set(CMAKE_CXX_STANDARD 20)

add_executable(MealyMoore main.cpp
        MMV3/MealyGraph.cpp
        MMV3/MooreGraph.cpp)
