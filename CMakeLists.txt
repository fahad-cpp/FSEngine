cmake_minimum_required(VERSION 3.10)

project(VulkanApp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

find_package(Vulkan REQUIRED)
set(VULKAN_INCLUDE "$ENV{VULKAN_SDK}/Include")

file(GLOB SOURCES src/*.cpp src/*.h)

add_executable(VulkanApp ${SOURCES})
add_subdirectory(vendor/FSWindow)

target_include_directories(VulkanApp PUBLIC ${VULKAN_INCLUDE} vendor/stb)

target_compile_options(VulkanApp PRIVATE -ggdb -Wall -Wextra -Weffc++ -Werror -pedantic-errors)

target_link_libraries(VulkanApp Vulkan::Vulkan FSWindow)
