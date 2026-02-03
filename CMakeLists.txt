cmake_minimum_required(VERSION 3.10)

project(VulkanApp)

find_package(Vulkan REQUIRED)

set(VULKAN_INCLUDE "$ENV{VULKAN_SDK}/Include")

file(GLOB SOURCES src/*.cpp src/*.h)

add_executable(VulkanApp ${SOURCES})

target_include_directories(VulkanApp PUBLIC ${VULKAN_INCLUDE})

target_link_libraries(VulkanApp Vulkan::Vulkan)