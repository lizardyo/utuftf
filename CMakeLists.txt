cmake_minimum_required(VERSION 3.16)
project(ReSkateSkinSync VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Define our plugin build output as a dynamic library (.dll)
add_library(ReSkateSkinSync SHARED src/plugin.cpp)

# Maximize file performance and optimize data handling speed benchmarks
target_compile_options(ReSkateSkinSync PRIVATE /O2 /Oi /Gy)
