cmake_minimum_required(VERSION 3.29)
project(DetermineNFA)

set(CMAKE_CXX_STANDARD 20)

add_executable(DetermineNFA main.cpp
        DetermineNFA.h
        DetermineNFA.cpp
)
