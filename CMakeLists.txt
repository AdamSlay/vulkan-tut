cmake_minimum_required(VERSION 3.24)
project(initial_engine)
set(CMAKE_CXX_STANDARD 17)

# versions
set(GLFW_VERSION "3.3.8")
set(GLM_VERSION "0.9.9.8")
set(Vulkan_VERSION "1.3.243.0")
# paths
set(HOMEBREW_CELLAR "/opt/homebrew/Cellar")
set(Vulkan_SDK "/Users/adamslay/Documents/Dev/SDKs/VulkanSDK/${Vulkan_VERSION}/macOS")

# Vulkan paths
set(Vulkan_LIBRARY "${Vulkan_SDK}/lib/libvulkan.dylib")
set(Vulkan_INCLUDE_DIR "${Vulkan_SDK}/include")


# LIBRARIES
# include
include_directories(
        ${HOMEBREW_CELLAR}/glfw/${GLFW_VERSION}/include
        ${HOMEBREW_CELLAR}/glm/${GLM_VERSION}/include
        ${Vulkan_INCLUDE_DIR}
)
# link
link_directories(
        ${HOMEBREW_CELLAR}/glfw/${GLFW_VERSION}/lib
)

# DEBUG
if(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Build type: Release")
    add_definitions(-DNDEBUG)
else()
    message(STATUS "Build type: Debug")
endif()

# EXECUTABLE
add_executable(initial_engine main.cpp)

target_link_libraries(initial_engine PRIVATE
        glfw
        ${Vulkan_LIBRARY})


