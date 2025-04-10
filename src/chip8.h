#ifndef CHIP8_H
#define CHIP8_H

#include <SDL_render.h>
#include <SDL_video.h>
#include <stdbool.h>

typedef struct Chip8 {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int screen_height;
    int screen_width;
} Chip8;

bool CHIP8_initialize_SDL(Chip8 *chip8);
void CHIP8_close_SDL(Chip8 *chip8);

#endif // !CHIP8_H
