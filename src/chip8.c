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

/**
 * If key pressed is mapped to chip8's keyboard, returns the value of the key
 * from 0 to F, else, returns 16.
 */
static uint8_t get_chip8_key(SDL_Event event) {
    switch (event.key.keysym.sym) {
        case SDLK_1:
            return (uint8_t)0x1;
            break;
        case SDLK_2:
            return (uint8_t)0x2;
            break;
        case SDLK_3:
            return (uint8_t)0x3;
            break;
        case SDLK_4:
            return (uint8_t)0xc;
            break;
        case SDLK_q:
            return (uint8_t)0x4;
            break;
        case SDLK_w:
            return (uint8_t)0x5;
            break;
        case SDLK_e:
            return (uint8_t)0x6;
            break;
        case SDLK_r:
            return (uint8_t)0xd;
            break;
        case SDLK_a:
            return (uint8_t)0x7;
            break;
        case SDLK_s:
            return (uint8_t)0x8;
            break;
        case SDLK_d:
            return (uint8_t)0x9;
            break;
        case SDLK_f:
            return (uint8_t)0xe;
            break;
        case SDLK_z:
            return (uint8_t)0xa;
            break;
        case SDLK_x:
            return (uint8_t)0x0;
            break;
        case SDLK_c:
            return (uint8_t)0xb;
            break;
        case SDLK_v:
            return (uint8_t)0xf;
            break;
        default:
            return (uint8_t)0x10;
    }
}

void CHIP8_run(Chip8 *ch8, bool *quit_flag) {
    printf("inside run. quit: %d\n", (int)(*quit_flag));
    while (!*quit_flag) {
        while (SDL_PollEvent(&ch8->event)) {
            if (ch8->event.type == SDL_QUIT) {
                *quit_flag = true;
            }
            if (ch8->event.type == SDL_KEYDOWN) {
                ch8->keyboard_state[get_chip8_key(ch8->event)] = true;
            }
            if (ch8->event.type == SDL_KEYUP) {
                ch8->keyboard_state[get_chip8_key(ch8->event)] = false;
            }
        }
        // SDL_Delay(500);
        CPU_Execute_Instruction(ch8);
    }
}
