cmake_minimum_required(VERSION 3.22)

project(CpuPG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

add_definitions(-O3 -march=native -Wall)
add_subdirectory(src)
add_subdirectory(test)