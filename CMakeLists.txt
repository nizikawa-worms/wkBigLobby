cmake_minimum_required(VERSION 3.17)
project(wkBigLobby)

set(CMAKE_CXX_STANDARD 23)

add_library(hacklib STATIC
        lib/hacklib/ExeFile.cpp lib/hacklib/ExeFile_WIN32.cpp
        lib/hacklib/PatternScanner.cpp lib/hacklib/Memory_WIN32.cpp)

link_directories(lib)
include_directories(include lib/hacklib)

configure_file(
        resource.rc.in
        resource.rc
)

add_definitions(-DASMJIT_STATIC)

add_library(wkBigLobby SHARED
        src/dllmain.cpp
        src/Config.cpp src/Config.h
        src/DevConsole.cpp src/DevConsole.h
        src/Hooks.cpp src/Hooks.h
        src/Debugf.h
        src/SlotList.cpp src/SlotList.h
        src/LobbyChat.cpp src/LobbyChat.h
        ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
        )

set_property(TARGET wkBigLobby hacklib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(wkBigLobby hacklib capstone Polyhook_2 version asmjit)
