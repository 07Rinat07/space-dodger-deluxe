cmake_minimum_required(VERSION 3.22)

project(SpaceDodgerDeluxeIOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

find_package(raylib REQUIRED)

add_executable(space_dodger_ios MACOSX_BUNDLE
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

target_include_directories(space_dodger_ios PRIVATE ${PROJECT_ROOT}/include)
target_link_libraries(space_dodger_ios PRIVATE raylib)

set_target_properties(space_dodger_ios PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.spacedodger.deluxe"
    MACOSX_BUNDLE_BUNDLE_NAME "Space Dodger Deluxe"
)

add_custom_command(TARGET space_dodger_ios POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_ROOT}/assets
    $<TARGET_BUNDLE_CONTENT_DIR:space_dodger_ios>/Resources/assets
)
