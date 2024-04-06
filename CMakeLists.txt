cmake_minimum_required(VERSION 3.16)

project(libscheisse VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(scheisse
    lib.cpp
)
