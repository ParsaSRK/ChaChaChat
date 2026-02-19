cmake_minimum_required(VERSION 3.21)

project(ChaChaChat
    VERSION 0.1.0
    LANGUAGES CXX
)

# ---- Global C++ Standard ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Project Options Target ----
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_23)

# ---- Warning Policy ----
add_library(project_warnings INTERFACE)

target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /WX
        /permissive-
        /sdl
    >
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        -Wsign-conversion
        -pedantic-errors
        -Werror
    >
)

# enable extra warnings only on GCC/Clang
target_compile_options(project_warnings INTERFACE
    $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:Debug>>:
        -Wconversion
    >
)

# ---- Sanitizers (Debug only, non-MSVC) ----
if (NOT MSVC)
    target_compile_options(project_options INTERFACE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
    target_link_options(project_options INTERFACE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
endif()

# ---- Subdirectories ----
add_subdirectory(client)
add_subdirectory(server)
