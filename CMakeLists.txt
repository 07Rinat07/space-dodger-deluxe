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

find_package(raylib QUIET)

if (NOT raylib_FOUND)
    include(FetchContent)

    # raylib is fetched and built automatically if not available on the system.
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
endif()

add_executable(space_dodger
    src/main.cpp
    src/Game.cpp
    src/Bullet.cpp
    src/EnemyProjectile.cpp
    src/WaveSystem.cpp
    src/Player.cpp
    src/Asteroid.cpp
    src/Pickup.cpp
    src/Particle.cpp
    src/Starfield.cpp
    src/Storage.cpp
)

target_include_directories(space_dodger PRIVATE include)
target_link_libraries(space_dodger PRIVATE raylib)

if (EMSCRIPTEN)
    set_target_properties(space_dodger PROPERTIES SUFFIX ".html")
    target_link_options(space_dodger PRIVATE
        "SHELL:-s USE_GLFW=3"
        "SHELL:-s ASYNCIFY"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    )
endif()

enable_testing()

add_executable(unit_tests
    tests/UnitTests.cpp
    src/Asteroid.cpp
    src/Bullet.cpp
    src/EnemyProjectile.cpp
    src/WaveSystem.cpp
    src/Pickup.cpp
    src/Particle.cpp
    src/Player.cpp
    src/Storage.cpp
)

target_compile_definitions(unit_tests PRIVATE UNIT_TEST=1)
target_include_directories(unit_tests PRIVATE include)
add_test(NAME unit_tests COMMAND unit_tests)

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
