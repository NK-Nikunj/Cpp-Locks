# Copyright (c) 2021 Nikunj Gupta
#

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(Cpp-Locks CXX)

find_package(HPX REQUIRED)
find_package(benchmark REQUIRED)

include_directories(include)

enable_testing()
add_subdirectory(tests)