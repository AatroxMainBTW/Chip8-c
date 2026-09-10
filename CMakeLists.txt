cmake_minimum_required(VERSION 3.16)

project(chip8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─────────────────────────────────────────────
# SDL2
# ─────────────────────────────────────────────

find_package(SDL2 CONFIG REQUIRED)

# ─────────────────────────────────────────────
# Executable
# ─────────────────────────────────────────────

add_executable(chip8
    src/main.c
    src/chip8.c
    src/chip8.h
)

target_include_directories(chip8 PRIVATE src)

# ─────────────────────────────────────────────
# SDL2 linking
# ─────────────────────────────────────────────

if(TARGET SDL2::SDL2)
    target_link_libraries(chip8 PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        SDL2::SDL2)
else()
    target_include_directories(chip8 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES})
endif()
# ─────────────────────────────────────────────
# Compiler warnings
# ─────────────────────────────────────────────

if(MSVC)
    # Visual Studio
    target_compile_options(chip8 PRIVATE /W4)
else()
    # GCC / Clang (macOS, Linux)
    target_compile_options(chip8 PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()