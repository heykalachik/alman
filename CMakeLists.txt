cmake_minimum_required(VERSION 4.1)
project(alman)

set(CMAKE_CXX_STANDARD 20)

add_executable(alman
    src/main.cpp
    src/Alias.cpp
    src/AliasGroup.cpp
    src/AliasStore.cpp
    src/ShellExporter.cpp
    src/ZshExporter.cpp
    src/BashExporter.cpp
    src/tui/Widget.cpp
    src/tui/Terminal.cpp
    src/tui/ListView.cpp
    src/tui/SearchBar.cpp
    src/tui/App.cpp
)

target_include_directories(alman PRIVATE src)
