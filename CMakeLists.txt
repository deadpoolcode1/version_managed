cmake_minimum_required(VERSION 3.10)
project(VersionChecker)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -g")

# Set source files
set(SOURCE_FILES main.cpp)

# Add the executable
add_executable(version_check.bin ${SOURCE_FILES})

# Link against required libraries
target_link_libraries(version_check.bin xxhash pthread)