cmake_minimum_required(VERSION 3.22)

project(SpaceDodgerDeluxeAndroid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../../..)

# Set RAYLIB_ANDROID_SOURCE to a local raylib checkout built for Android.
# Example: -DRAYLIB_ANDROID_SOURCE=/path/to/raylib
if (NOT DEFINED RAYLIB_ANDROID_SOURCE OR RAYLIB_ANDROID_SOURCE STREQUAL "")
    message(FATAL_ERROR "Set -DRAYLIB_ANDROID_SOURCE to a raylib checkout configured for Android")
endif()

set(PLATFORM Android CACHE STRING "raylib target platform" FORCE)
set(OPENGL_VERSION "ES 2.0" CACHE STRING "raylib OpenGL profile" FORCE)
set(WITH_PIC ON CACHE BOOL "Build raylib with position independent code" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build raylib examples" FORCE)

add_subdirectory(${RAYLIB_ANDROID_SOURCE} raylib_android)

add_library(space_dodger_android SHARED
    ${PROJECT_ROOT}/src/main.cpp
    ${PROJECT_ROOT}/src/Game.cpp
    ${PROJECT_ROOT}/src/Bullet.cpp
    ${PROJECT_ROOT}/src/EnemyProjectile.cpp
    ${PROJECT_ROOT}/src/WaveSystem.cpp
    ${PROJECT_ROOT}/src/Player.cpp
    ${PROJECT_ROOT}/src/Asteroid.cpp
    ${PROJECT_ROOT}/src/Pickup.cpp
    ${PROJECT_ROOT}/src/Particle.cpp
    ${PROJECT_ROOT}/src/Starfield.cpp
    ${PROJECT_ROOT}/src/Storage.cpp
)

target_include_directories(space_dodger_android PRIVATE ${PROJECT_ROOT}/include)
target_link_libraries(space_dodger_android PRIVATE raylib android log OpenSLES)
