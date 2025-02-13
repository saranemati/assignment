cmake_minimum_required(VERSION 3.22)  # Minimum required CMake version

project(excerise-8 VERSION 1.0
                    DESCRIPTION "Excerise 8")

enable_testing()

set(CMAKE_CXX_STANDARD 17)  # Set C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(test_queue test.cpp)  # Create an executable from test.cpp

target_link_libraries(test_queue PRIVATE gtest gmock gtest_main)

add_test(NAME queue_test COMMAND test_queue)