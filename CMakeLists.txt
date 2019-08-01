cmake_minimum_required(VERSION 2.8.9)
project(nc-geo)
set(CMAKE_BUILD_TYPE Debug)

find_package(Threads)
include_directories(src)
include_directories(test)

file(GLOB SOURCES "src/*.cpp" "test/*.cpp" "test/gtest/gtest-all.cc")

enable_testing()
add_executable(nc-geo-test ${SOURCES})

target_link_libraries(nc-geo-test pthread)
add_test(unit-test, nc-geo-test)
