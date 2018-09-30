cmake_minimum_required (VERSION 3.7)
project (VkPlay)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_STANDARD 11)

find_package (Vulkan)
add_subdirectory(V-EZ)
include_directories (V-EZ/Source)
set (APP_SOURCES
    application.cpp
)

add_executable (play ${APP_SOURCES})
target_link_libraries (play Vulkan::Vulkan VEZ)

add_custom_command(TARGET play POST_BUILD COMMAND cscope -b -q -U -R)
