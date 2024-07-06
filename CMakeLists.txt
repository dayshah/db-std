cmake_minimum_required(VERSION 3.12)
project(MyProject)

set (CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -Wall")

file(GLOB SOURCES *.cpp)

# Create an executable target
add_executable(MyProject ${SOURCES})
