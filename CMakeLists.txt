cmake_minimum_required(VERSION 3.10)
project(WordleSolver)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fopenmp -O3 -march=native -mtune=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fopenmp -O3 -march=native -mtune=native")

add_executable(solver
    src/main.cpp
)
add_executable(wordle
    src/wordle.cpp
)

target_include_directories(wordle PRIVATE src)
target_include_directories(solver PRIVATE src)
