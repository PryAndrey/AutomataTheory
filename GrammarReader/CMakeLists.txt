cmake_minimum_required(VERSION 3.29)
project(GrammarReader)

set(CMAKE_CXX_STANDARD 20)

add_executable(GrammarReader main.cpp
        GrammarReader.h
        GrammarReader.cpp)
