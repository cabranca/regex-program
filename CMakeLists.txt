cmake_minimum_required(VERSION 4.2)

project(Regex)

set(CMAKE_CXX_STANDARD 23)

add_executable(Regex src/Main.cpp
    src/Automata.cpp)