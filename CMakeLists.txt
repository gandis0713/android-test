cmake_minimum_required(VERSION 3.26)

project(dt-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(jipu CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(dt-test main.cpp)

target_link_libraries(dt-test PRIVATE
    jipu::jipu
)
