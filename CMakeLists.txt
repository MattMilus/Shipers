cmake_minimum_required(VERSION 3.5)
project(Shipers)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --------------------------------------------------------
# 1. Server
# --------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "--- Kompilacja na LINUX (WSL / Raspberry Pi) ---")
    message(STATUS "Budowanie TYLKO serwera w czystym C (bez SFML).")

    add_executable(ShipersServer src/server/main.c
            src/server/managers/GameManager.c
            src/server/managers/GameManager.h
            src/server/packets_handlers/ConnectionHandler.c
            src/server/packets_handlers/ConnectionHandler.h
            src/server/Router.c
            src/server/Router.h
            src/server/ServerPackets.h
            src/server/entities/Boat.c
            src/server/entities/Boat.h
            src/server/packets_handlers/StateManager.c
            src/server/packets_handlers/StateManager.h)

    target_link_libraries(ShipersServer m)

else()
    # --------------------------------------------------------
    # 2. Client
    # --------------------------------------------------------
    message(STATUS "--- Kompilacja na WINDOWS / MAC ---")
    message(STATUS "Pobieranie SFML i budowanie Klienta w C++.")

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
            src/client/managers/web_managers/ServerPackets.h
            src/client/managers/web_managers/ConnectionManager.cpp
            src/client/managers/web_managers/ConnectionManager.h
            src/client/managers/web_managers/StateManager.cpp
            src/client/managers/web_managers/StateManager.h
            src/client/managers/web_managers/PacketHandler.cpp
            src/client/managers/web_managers/PacketHandler.h
     "src/client/managers/Terminal.h" "src/client/managers/Terminal.cpp")

    target_link_libraries(ShipersClient PRIVATE
            SFML::Graphics
            SFML::Window
            SFML::System
            SFML::Network
    )

    add_custom_command(TARGET ShipersClient POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory
        "$<TARGET_FILE_DIR:ShipersClient>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/client/assets"
        "$<TARGET_FILE_DIR:ShipersClient>/assets"
        COMMENT "Kopiowanie assets..."
    )
endif()
