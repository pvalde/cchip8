#include "chip8.h"
#include "cpu.h"
#include <SDL.h>
#include <SDL_events.h>

bool CHIP8_initialize_SDL(Chip8 *chip8) {

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    } else {
        // Create window
        chip8->window = SDL_CreateWindow(
            "Chip8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            chip8->screen_width, chip8->screen_height, SDL_WINDOW_SHOWN);
        if (chip8->window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n",
                   SDL_GetError());
            return false;
        }

        // Create renderer
        chip8->renderer = SDL_CreateRenderer(chip8->window, -1, 0);

        if (chip8->renderer == NULL) {
            printf("Renderer could not be created! SDL_Error: %s\n",
                   SDL_GetError());
            return false;
        }
    }
    return true;
}

void CHIP8_close_SDL(Chip8 *chip8) {
    // Destroy Window
    SDL_DestroyWindow(chip8->window);
    chip8->window = NULL;

    // Destroy Renderer
    SDL_DestroyRenderer(chip8->renderer);
    chip8->renderer = NULL;

    SDL_Quit();
}

void CHIP8_load_rom(Rom *rom) { CPU_load_rom(rom); }

void CHIP8_run(Chip8 *ch8, bool *quit_flag) {
    printf("inside run. quit: %d\n", (int)(*quit_flag));
    while (!*quit_flag) {
            while (SDL_PollEvent(&ch8->event)) {
                if (ch8->event.type == SDL_QUIT) {
                    *quit_flag = true;
                }
            }
            // SDL_Delay(500);
            CPU_Execute_Instruction(ch8);
    }
}
