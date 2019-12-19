cmake_minimum_required(VERSION 3.15)
project(diffirenciator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(tokenizer src/main.cpp src/string_funcs.cpp)
