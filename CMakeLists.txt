cmake_minimum_required(VERSION 3.29)
project(Shipers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0
)
FetchContent_MakeAvailable(SFML)

add_executable(Shipers src/main.cpp
        src/entities/Boat.cpp
        src/entities/Boat.h
        src/entities/Player.cpp
        src/entities/Player.h
        src/GameManager.cpp
        src/GameManager.h)

target_link_libraries(Shipers PRIVATE SFML::Graphics SFML::Window SFML::System)