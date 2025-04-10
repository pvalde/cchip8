#include "chip8.h"
#include <SDL.h>

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
