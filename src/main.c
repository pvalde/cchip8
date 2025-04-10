#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rom.h"
#include "chip8.h"

#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define SCALE 10
#define SCREEN_WIDTH (CHIP8_WIDTH * SCALE)
#define SCREEN_HEIGHT (CHIP8_HEIGHT * SCALE)

int main(int argc, char *argv[]) {

    /* Read Rom */
    Rom rom;
    rom.size = 0;
    rom.data = NULL;

    if (ROM_read_rom(argv[1], &rom)) {
        return 1;
    }
    ROM_print_rom_bytes(&rom);

    /* initialize Chip8 object */
    Chip8 chip8;
    chip8.window = NULL;
    chip8.renderer = NULL;
    chip8.screen_height = SCREEN_HEIGHT;
    chip8.screen_width = SCREEN_WIDTH;

    if (!CHIP8_initialize_SDL(&chip8)) {
        printf("Failed to initialize SDL\n");
    }

    // test rect
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.h = SCALE;
    rect.w = SCALE;

    // event loop
    SDL_Event event;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Set color to black (for the background)
        SDL_SetRenderDrawColor(chip8.renderer, 0, 0, 0, 255);
        SDL_RenderClear(chip8.renderer);

        // Draw test rect on the screen
        rect.x += 1;
        rect.y += 1;

        SDL_SetRenderDrawColor(chip8.renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(chip8.renderer, &rect);
        SDL_RenderPresent(chip8.renderer);
        SDL_Delay(1000 / 60);
    }

    CHIP8_close_SDL(&chip8);
    ROM_deallocate_rom_data(&rom);

    return 0;
}


