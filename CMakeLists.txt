cmake_minimum_required(VERSION 3.16)

project(SailkapenaHG VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for IntelliSense path resolution
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/Player.cpp
    src/Player.h
    src/TournamentFormat.cpp
    src/TournamentFormat.h
    src/Taula.cpp
    src/Taula.h
    src/Txapelketa.cpp
    src/Txapelketa.h
    src/Txapelketa_io.cpp
    src/CountDownWidget.cpp
    src/CountDownWidget.h
    src/PublicWindow.cpp
    src/PublicWindow.h
    src/TimeFormat.h
    src/setLabel.h
)

qt_add_executable(SailkapenaHG
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

if(WIN32)
    set_target_properties(SailkapenaHG PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(SailkapenaHG PRIVATE Qt6::Widgets)

qt_finalize_executable(SailkapenaHG)
