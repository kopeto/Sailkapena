cmake_minimum_required(VERSION 3.5)

project(SailkapenaHG VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IntelliSense path resolution
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(QT6CMAKE_PATH "/opt/Qt/6.10.2/gcc_64/lib/cmake/")

# set(CMAKE_PREFIX_PATH
#     ${QT6CMAKE_PATH}"Qt6"
#     ${QT6CMAKE_PATH}"Qt6CoreTools"
#     ${QT6CMAKE_PATH}"Qt6GuiTools")

find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

message(STATUS "QT_VERSION: " ${QT_VERSION})

FILE(GLOB PROJECT_SOURCES 
    src/*.cpp
    src/*.h
) 

qt_add_executable(SailkapenaHG
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

if(WIN32)
    set_target_properties(SailkapenaHG PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(SailkapenaHG PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SailkapenaHG)
endif()
