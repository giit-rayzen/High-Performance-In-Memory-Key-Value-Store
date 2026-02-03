cmake_minimum_required(VERSION 3.10)

# Project name
project(ThreadPool VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threads library (required for pthread)
find_package(Threads REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/KeyValueStore.cpp
    src/StorageEngine.cpp
    src/main.cpp
)

# Executable
add_executable(kv_store ${SOURCES})

# Link threads library
target_link_libraries(kv_store Threads::Threads)


# Print configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Include directory: ${PROJECT_SOURCE_DIR}/include")
message(STATUS "Source directory: ${PROJECT_SOURCE_DIR}/src")