set(CMAKE_CXX_STANDARD 23)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

project(VulkanApp)

find_package(Vulkan REQUIRED)

set(VULKAN_INCLUDE "$ENV{VULKAN_SDK}/Include")

file(GLOB SOURCES src/*.cpp src/*.h)

add_executable(VulkanApp ${SOURCES})

target_include_directories(VulkanApp PUBLIC ${VULKAN_INCLUDE})

if(UNIX AND NOT APPLE)
target_link_libraries(VulkanApp Vulkan::Vulkan X11)
else()
target_link_libraries(VulkanApp Vulkan::Vulkan)
endif()
