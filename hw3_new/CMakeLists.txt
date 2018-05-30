cmake_minimum_required(VERSION 3.10)
project(hw3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(hw3 Factory.h Product.h Factory.cxx test_din.cpp)