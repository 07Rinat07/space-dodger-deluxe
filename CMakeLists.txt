cmake_minimum_required(VERSION 3.20)

project(SpaceDodgerDeluxe
    VERSION 1.0.0
    DESCRIPTION "A small but complete C++ arcade game pet project built with raylib"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Helpful compiler warnings for learning and cleaner GitHub project quality.
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

# raylib is fetched and built automatically, so the user does not need to install it manually.
# You can update GIT_TAG to a newer stable version later if needed.
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

# Keep raylib examples out of the user's build.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

add_executable(space_dodger
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Asteroid.cpp
    src/Pickup.cpp
    src/Particle.cpp
    src/Starfield.cpp
    src/Storage.cpp
)

target_include_directories(space_dodger PRIVATE include)
target_link_libraries(space_dodger PRIVATE raylib)

# On some Linux systems raylib examples use libm explicitly.
if (UNIX AND NOT APPLE)
    target_link_libraries(space_dodger PRIVATE m)
endif()

# Copy assets folder near executable. Currently the game draws graphics procedurally,
# but the folder is prepared for future textures/sounds.
add_custom_command(TARGET space_dodger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:space_dodger>/assets
)
