cmake_minimum_required(VERSION 3.10)
project(WordleSolver)

# Common
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OpenMP_RUNTIME "LLVM")
find_package(OpenMP REQUIRED)

# Configuration options
add_library(project_options INTERFACE)
target_compile_options(project_options INTERFACE
    -Wall
    -fno-rtti
    -fno-exceptions
    $<$<CONFIG:Debug>:-Og -g>
    $<$<CONFIG:Release>:
        -O3
        -march=native
        -mtune=native
        -flto
        -fomit-frame-pointer
    >
)
target_link_options(project_options INTERFACE
    $<$<CONFIG:Release>:
        -flto
    >
)
target_compile_definitions(project_options INTERFACE
    $<$<CONFIG:Debug>:DEBUG=1>
)

# Include and source file
include_directories(include)

add_library(common OBJECT
    src/cache.cpp
)

add_executable(solver
    src/solver.cpp
)
target_link_libraries(solver PRIVATE
    project_options
    common
    OpenMP::OpenMP_CXX
)
add_executable(build_cache
    src/build_cache.cpp
)
target_link_libraries(build_cache PRIVATE
    project_options
    common
    OpenMP::OpenMP_CXX
)
add_executable(wordle
    src/wordle.cpp
)
target_link_libraries(wordle PRIVATE
    project_options
)

# Post-build processing
add_custom_command(TARGET solver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:solver>/data
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    foreach(target solver build_cache wordle)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${target}>
        )
    endforeach()
endif()
