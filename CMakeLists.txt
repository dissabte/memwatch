cmake_minimum_required(VERSION 3.16)

project(MemoryWatcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20 REQUIRED)

add_executable(memwatch
    src/main.cpp
)