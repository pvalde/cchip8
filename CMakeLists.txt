cmake_minimum_required(VERSION 3.10)

project(chip8 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)
add_executable(${PROJECT_NAME} src/main.c 
                               src/rom.c
                               src/rom.h
                               src/chip8.c
                               src/chip8.h
                               src/cpu.h
                               src/cpu.c)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
