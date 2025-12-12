cmake_minimum_required(VERSION 3.15)
project(OrderBookEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler optimizations for HFT
if(MSVC)
    add_compile_options(/O2 /W4 /arch:AVX2)
else()
    add_compile_options(-O3 -Wall -Wextra -march=native)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Main executable
add_executable(orderbook_demo
    src/main.cpp
)

# Benchmark executable
add_executable(orderbook_benchmark
    benchmark/benchmark.cpp
)

# Enable link-time optimization
set_target_properties(orderbook_demo PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
set_target_properties(orderbook_benchmark PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
