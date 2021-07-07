# Copyright (c) 2021 Nikunj Gupta
#

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(Cpp-Locks CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WARNING_FLAGS "-Wall -Wno-ambiguous-reversed-operator")
set(DEBUG_FLAGS "-O0 -g ${WARNING_FLAGS}")
set(RELEASE_FLAGS "-Ofast -DNDEBUG -march=native ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_FLAGS})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

find_package(HPX REQUIRED)

include_directories(include)

enable_testing()
add_subdirectory(tests)
