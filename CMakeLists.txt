cmake_minimum_required(VERSION 3.28)
project(Spin C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(vendor/SDL)
add_subdirectory(vendor/cglm)

find_package(OpenGL REQUIRED)

# Source files
set(SOURCES
    vendor/log/log.c
    vendor/glad/glad.c

    src/main.c
    src/camera.c
    src/keyboard.c
    src/controls.c
    src/shader.c
    src/texture.c
)

# Include directories for Sokol and shaders
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/
    ${CMAKE_CURRENT_SOURCE_DIR}/res/
)

# Add executable
add_executable(Spin ${SOURCES})

# Link system frameworks required by Sokol for macOS with OpenGL
# Cocoa and QuartzCore are provided by SDL
target_link_libraries(Spin
    OpenGL::GL
    SDL3::SDL3
    cglm
)

target_compile_definitions(Spin PRIVATE LOG_USE_COLOR)