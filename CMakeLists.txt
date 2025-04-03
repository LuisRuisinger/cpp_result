cmake_minimum_required(VERSION 3.10)  # Set minimum CMake version
project(MyProject CXX)  # Project name and language (C++)

set(CMAKE_CXX_STANDARD 20)  # Use C++17 (or change to 20 if needed)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files (you can list them explicitly or use file(GLOB))
set(SOURCES
        main.cpp
)

# Add header files (optional, for IDEs and clarity)
set(HEADERS
    result.hpp
)

# Define the executable target
add_executable(MyExecutable ${SOURCES} ${HEADERS})

# Include directories for headers
target_include_directories(MyExecutable PRIVATE include)