cmake_minimum_required(VERSION 3.26)

project(dt-test LANGUAGES CXX)

add_executable(dt-test main.cpp)

target_link_libraries(dt-test PRIVATE
    # jipu::jipu
)
