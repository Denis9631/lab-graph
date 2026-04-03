cmake_minimum_required(VERSION 3.10)
project(GraphoDro4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "g++")

add_executable(graphodro4
    cli/main.cpp
)

target_include_directories(graphodro4 PRIVATE include)