cmake_minimum_required(VERSION 3.29)
project(RegexToNFA)

set(CMAKE_CXX_STANDARD 20)

add_executable(RegexToNFA
        main.cpp
        RegexToNFA.h
        RegexToNFA.cpp
)
