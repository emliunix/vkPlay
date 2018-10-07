cmake_minimum_required (VERSION 3.7)
project (VkPlay)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_STANDARD 11)

find_package (Vulkan)
add_subdirectory(V-EZ)
include_directories (V-EZ/Source)
set (APP_SOURCES
    application.cpp
    application.hpp
	simhashvk.cpp
	simhashvk.hpp
    utils.cpp
    utils.hpp
)

add_executable (play ${APP_SOURCES})
target_link_libraries (play Vulkan::Vulkan VEZ)

# add_custom_command(TARGET play POST_BUILD COMMAND cscope -b -q -U -R)
if (WIN32)
    file (COPY V-EZ/Bin/x86_64/VEZd.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file (COPY V-EZ/Bin/x86_64/VEZd.pdb DESTINATION ${CMAKE_BINARY_DIR}/)
endif (WIN32)

set (SHADERS
    simhash.comp
)

foreach (SHADER ${SHADERS})
    file (COPY shader/${SHADER} DESTINATION ${CMAKE_BINARY_DIR}/shader)
endforeach (SHADER)
