cmake_minimum_required(VERSION 3.16)
project(jolt-vehicle-demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Jolt Physics configuration
set(PHYSICS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/JoltPhysics")
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)

# Use FetchContent to get Jolt
include(FetchContent)
FetchContent_Declare(
    JoltPhysics
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG v5.2.0
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR Build
)
FetchContent_MakeAvailable(JoltPhysics)

# Find SDL2
find_package(SDL2 REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Main executable
add_executable(jolt-vehicle-demo
    main.cpp
)

target_include_directories(jolt-vehicle-demo PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(jolt-vehicle-demo PRIVATE
    Jolt
    ${SDL2_LIBRARIES}
    OpenGL::GL
    OpenGL::GLU
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(jolt-vehicle-demo PRIVATE
        mingw32
        SDL2main
    )
endif()

# Enable multi-threading for Jolt
target_compile_definitions(jolt-vehicle-demo PRIVATE
    JPH_PROFILE_ENABLED
    JPH_DEBUG_RENDERER
)
