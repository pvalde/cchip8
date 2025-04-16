#ifndef CHIP8_H
#define CHIP8_H

#include "rom.h"
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <stdbool.h>

typedef struct Chip8 {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    int scale;
    int screen_height;
    int screen_width;
} Chip8;

bool CHIP8_initialize_SDL(Chip8 *chip8);
void CHIP8_close_SDL(Chip8 *chip8);
void CHIP8_load_rom(Rom *rom);
void CHIP8_run(Chip8 *ch8, bool *quit_flag);

#endif // !CHIP8_H
