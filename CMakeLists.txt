cmake_minimum_required(VERSION 3.5)
project(Shipers)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------
# 1. Server
# --------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "--- Kompilacja na LINUX (WSL / Raspberry Pi) ---")
    message(STATUS "Budowanie TYLKO serwera w czystym C (bez SFML).")

    add_executable(ShipersServer
            src/server/main.c
            src/server/managers/GameManager.c
            src/server/managers/GameManager.h
            src/server/packets_handlers/ConnectionHandler.c
            src/server/packets_handlers/ConnectionHandler.h
            src/server/Router.c
            src/server/Router.h
            src/server/entities/Boat.c
            src/server/entities/Boat.h
            src/server/entities/Track.c
            src/server/entities/Track.h
            src/server/entities/Coin.c
            src/server/entities/Coin.h
            src/server/packets_handlers/StateManager.c
            src/server/packets_handlers/StateManager.h
            src/server/ServerPackets.h
            src/server/tracks/TrackLoader.c
            src/server/tracks/TrackLoader.h
            src/server/tracks/tracks/Track1.c
            src/server/tracks/tracks/Track1.h
    )

    target_link_libraries(ShipersServer m)

    # Copy top-level track.txt (if present) into the server binary directory so the server
    # will load the same track configuration at runtime. Overwrite existing file.
    if(EXISTS "${CMAKE_SOURCE_DIR}/track.txt")
        add_custom_command(TARGET ShipersServer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_BINARY_DIR}/track.txt"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/track.txt" "${CMAKE_CURRENT_BINARY_DIR}/track.txt"
                COMMENT "Wymuszone kopiowanie track.txt do katalogu serwera (stary plik usuniety, nadpisanie)..."
        )
    endif()

else()
    # --------------------------------------------------------
    # 2. Client
    # --------------------------------------------------------
    message(STATUS "--- Kompilacja na WINDOWS / MAC ---")
    message(STATUS "Pobieranie SFML i budowanie Klienta oraz Edytora w C++.")

    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF)

    FetchContent_Declare(
            SFML
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 3.0.0
    )
    FetchContent_MakeAvailable(SFML)

    add_executable(ShipersClient
            src/client/main.cpp
            src/client/managers/Renderer.cpp
            src/client/managers/Renderer.h
            src/client/managers/Panel.cpp
            src/client/managers/Panel.h
            src/client/managers/TextField.cpp
            src/client/managers/TextField.h
            src/client/managers/Terminal.cpp
            src/client/managers/Terminal.h
            src/client/managers/GameManager.cpp
            src/client/managers/GameManager.h
            src/client/entities/Player.cpp
            src/client/entities/Player.h
            src/client/entities/Boat.cpp
            src/client/entities/Boat.h
            src/client/entities/Coin.cpp
            src/client/managers/web_managers/ConnectionManager.cpp
            src/client/managers/web_managers/ConnectionManager.h
            src/client/managers/web_managers/StateManager.cpp
            src/client/managers/web_managers/StateManager.h
            src/client/managers/web_managers/PacketHandler.cpp
            src/client/managers/web_managers/PacketHandler.h
            src/client/ServerPackets.h
            src/client/entities/Track.h
            src/client/entities/Track.cpp
            src/client/managers/EndGameScoreboard.cpp
            src/client/managers/EndGameScoreboard.h
            src/client/tracks/TrackLoader.cpp
            src/client/tracks/TrackLoader.h
            src/client/tracks/tracks/Track1.cpp
            src/client/tracks/tracks/Track1.h
    )

    if(MINGW)
        target_link_options(ShipersClient PRIVATE -static -static-libgcc -static-libstdc++)
    endif()

    target_link_libraries(ShipersClient PRIVATE
            SFML::Graphics
            SFML::Window
            SFML::System
            SFML::Network
    )

    add_custom_target(CopyShipersAssets
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/assets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/client/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
            COMMENT "Wymuszone kopiowanie assets..."
    )

    add_dependencies(ShipersClient CopyShipersAssets)

    # Copy top-level track.txt (if present) into the client binary directory so the client
    # and editor will have access to the same track.txt. Overwrite existing file.
    if(EXISTS "${CMAKE_SOURCE_DIR}/track.txt")
        add_custom_command(TARGET ShipersClient POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_BINARY_DIR}/track.txt"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/track.txt" "${CMAKE_CURRENT_BINARY_DIR}/track.txt"
                COMMENT "Wymuszone kopiowanie track.txt do katalogu klienta (stary plik usuniety, nadpisanie)..."
        )
    endif()

    # --------------------------------------------------------
    # 3. Track Editor
    # --------------------------------------------------------
    add_executable(TrackEditor
            src/tools/TrackEditor.cpp
    )

    if(MINGW)
        target_link_options(TrackEditor PRIVATE -static -static-libgcc -static-libstdc++)
    endif()

    target_link_libraries(TrackEditor PRIVATE
            SFML::Graphics
            SFML::Window
            SFML::System
    )
endif()