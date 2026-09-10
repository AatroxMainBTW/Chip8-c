cmake_minimum_required(VERSION 3.16)
project(chip8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd / your editor

find_package(SDL2 REQUIRED)

add_executable(chip8
    src/main.c
    src/chip8.c
    Src/chip8.h
)

target_include_directories(chip8 PRIVATE src)

# SDL2::SDL2 is the modern imported target (Homebrew, vcpkg, recent SDL2).
# Fall back to the classic variables if it isn't defined.
if(TARGET SDL2::SDL2)
    target_link_libraries(chip8 PRIVATE SDL2::SDL2)
else()
    target_include_directories(chip8 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES})
endif()

target_compile_options(chip8 PRIVATE -Wall -Wextra -Wpedantic)
