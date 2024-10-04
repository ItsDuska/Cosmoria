cmake_minimum_required(VERSION 3.5.0)
project(Cosmoria VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED)

add_executable(Cosmoria main.c)

target_link_libraries(Cosmoria PRIVATE Vulkan::Vulkan)

include_directories(${Vulkan_INCLUDE_DIRS})

