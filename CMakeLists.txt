cmake_minimum_required(VERSION 3.5.0)
project(Cosmoria VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c  # Etsii kaikki C-tiedostot rekursiivisesti
)

add_executable(Cosmoria ${SOURCES})

target_link_libraries(Cosmoria PRIVATE Vulkan::Vulkan)

target_include_directories(Cosmoria PRIVATE 
    ${Vulkan_INCLUDE_DIRS}      # Vulkanin headerit
    ${CMAKE_SOURCE_DIR}/src     # Lisää src-kansio ja sen alikansiot include-hakemistoksi
)

