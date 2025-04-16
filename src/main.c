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
    chip8.scale = SCALE;

    if (!CHIP8_initialize_SDL(&chip8)) {
        printf("Failed to initialize SDL\n");
    }

    CHIP8_load_rom(&rom);

    // event loop
    // SDL_Event event;
    bool quit = false;
    CHIP8_run(&chip8, &quit); // main loop

    // close everything
    CHIP8_close_SDL(&chip8);
    ROM_deallocate_rom_data(&rom);

    return 0;
}
