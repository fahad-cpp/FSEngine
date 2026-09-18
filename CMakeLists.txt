cmake_minimum_required(VERSION 3.10)

project(VulkanApp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

find_package(Vulkan REQUIRED)
set(VULKAN_INCLUDE "$ENV{VULKAN_SDK}/Include")

file(GLOB SOURCES src/*.cpp src/vulkan/*.cpp)

add_executable(VulkanApp ${SOURCES})
add_subdirectory(vendor/FSWindow)

target_include_directories(VulkanApp SYSTEM PRIVATE ${VULKAN_INCLUDE} vendor/stb)

target_compile_options(
    VulkanApp PRIVATE -Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion -Werror -pedantic-errors
    $<$<CONFIG:Debug>:-ggdb>
    $<$<CONFIG:Release>:-O3 -ffast-math>
)

target_link_libraries(VulkanApp Vulkan::Vulkan FSWindow)
