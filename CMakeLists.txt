cmake_minimum_required(VERSION 3.22)

project(cpupg)

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

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

add_definitions(-Ofast -march=native -Wall)
# add_definitions(-g -march=native -Wall)
add_definitions(-DDEBUG)
add_subdirectory(src)
add_subdirectory(test)