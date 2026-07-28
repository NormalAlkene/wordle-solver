cmake_minimum_required(VERSION 3.10)
project(WordleSolver)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -march=native -mtune=native")

add_executable(wordle-solver
    src/main.cpp
    src/model.cpp
)
target_include_directories(wordle-solver PRIVATE src)
