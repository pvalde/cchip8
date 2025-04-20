#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "chip8.h"
#include "rom.h"

#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define SCALE 10
#define SCREEN_WIDTH (CHIP8_WIDTH * SCALE)
#define SCREEN_HEIGHT (CHIP8_HEIGHT * SCALE)

int main(int argc, char *argv[]) {
    bool debugging_mode = false;
    /* Read Rom */
    Rom rom;
    rom.size = 0;
    rom.data = NULL;
    if (argc < 2) {
        printf("ERROR: No ROM provided.\n");
        return 0;
    }
    if (ROM_read_rom(argv[1], &rom)) {
        return 1;
    }

    // printf("arg 2: '%s'\n", argv[2]);
    if (argc == 3 && !strcmp(argv[2], "debugging")) {
        debugging_mode = true;
    }

    // ROM_print_rom_bytes(&rom);

    /* initialize Chip8 object */
    Chip8 chip8;
    chip8.window = NULL;
    chip8.renderer = NULL;
    chip8.screen_height = SCREEN_HEIGHT;
    chip8.screen_width = SCREEN_WIDTH;
    chip8.scale = SCALE;

    if (!CHIP8_initialize_SDL(&chip8)) {
        printf("Failed to initialize SDL\n");
    }

    CHIP8_load_rom(&rom);

    // event loop
    // SDL_Event event;
    bool quit = false;
    if (debugging_mode) {
        CHIP8_run_debugging_mode(&chip8, &quit);
    } else {
        CHIP8_run(&chip8, &quit); // main loop
    }

    // close everything
    CHIP8_close_SDL(&chip8);
    ROM_deallocate_rom_data(&rom);

    return 0;
}
