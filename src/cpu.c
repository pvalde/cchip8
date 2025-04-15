#include "cpu.h"
#include "chip8.h"
#include "rom.h"
#include <SDL_render.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ROM_ADDRESS 0x200
#define BEGIN_FONTS_ADDRESS 0x000
#define END_FONTS_ADDRESS 0x1FF
#define CH8_SCREEN_COLS 64
#define CH8_SCREEN_ROWS 32

/*****************************************************************************
 * CPU internals 
 ****************************************************************************/
static uint8_t memory[0xFFF] = {0};
static uint8_t v[0x10] = {0};
static uint16_t stack[0x10] = {0};
static uint16_t I = 0;
static uint8_t delayTimer = 0;
static uint8_t soundTimer = 0;
static uint16_t pc = 0;
static uint8_t sp = 0;
static bool screen_data[CH8_SCREEN_ROWS][CH8_SCREEN_COLS] = {false};
static bool font_loaded = false;
static bool rom_loaded = false;
static bool inc_pc = true;

/*****************************************************************************
 * static functions' declarations
 ****************************************************************************/
static void CPU_clear_state(void);
static void CPU_load_fonts(void);
static uint16_t CPU_get_instruction(void);
static void CPU_decode_execute_instruction(uint16_t instruction, Chip8 *ch8);
static void CPU_clear_screen_data(void);
static void CPU_draw_on_SDL_screen(Chip8 *ch8);

/*****************************************************************************
 * functions' definitions
 ****************************************************************************/
void CPU_load_rom(Rom *rom) {

    CPU_clear_state();
    CPU_load_fonts();

    for (int m_i = ROM_ADDRESS, i = 0; i < rom->size; i++, m_i++) {
        memory[m_i] = rom->data[i];
    }

    rom_loaded = true;
    pc = ROM_ADDRESS;
    // for(int i = ROM_ADDRESS; i < rom->size + ROM_ADDRESS; i++) {
    //      printf("%.4x: %.2x\n", i, memory[i]);
    // }
}

void CPU_Execute_Instruction(Chip8 *ch8) {
    if (rom_loaded && font_loaded) {
        uint16_t instruction = CPU_get_instruction();
        CPU_decode_execute_instruction(instruction, ch8);
        if (inc_pc) {
            pc += 2; // increments to next instruction
        } else {
            inc_pc = true;
        }
    } else {
        printf("no rom loaded");
        exit(1);
    }
}

static uint16_t CPU_get_instruction(void) {
    printf("addresses [%.2x-%.2x]: ", pc, pc + 1);
    uint8_t high_byte = memory[pc];
    uint8_t low_byte = memory[pc + 1];
    uint16_t instruction = 0x0000 | (high_byte << 8);
    return instruction | low_byte;
}

static void CPU_load_fonts(void) {
    uint8_t fonts[] = {
        0xf0, // 0
        0x90, 0x90, 0x90, 0xf0,
        0x20, // 1
        0x60, 0x20, 0x20, 0x70,
        0xf0, // 2
        0x10, 0xf0, 0x80, 0xf0,
        0xf0, // 3
        0x10, 0xf0, 0x10, 0xf0,
        0x90, // 4
        0x90, 0xf0, 0x10, 0x10,
        0xf0, // 5
        0x80, 0xf0, 0x10, 0xf0,
        0xf0, // 6
        0x80, 0xf0, 0x90, 0xf0,
        0xf0, // 7
        0x10, 0x20, 0x40, 0x40,
        0xf0, // 8
        0x90, 0xf0, 0x90, 0xf0,
        0xf0, // 9
        0x90, 0xf0, 0x10, 0xf0,
        0xf0, // A
        0x90, 0xf0, 0x90, 0x90,
        0xe0, // B
        0x90, 0xe0, 0x90, 0xe0,
        0xf0, // C
        0x80, 0x80, 0x80, 0xf0,
        0xe0, // D
        0x90, 0x90, 0x90, 0xe0,
        0xf0, // E
        0x80, 0xf0, 0x80, 0xf0,
        0xf0, // F
        0x80, 0xf0, 0x80, 0x80,
    };

    for (int m_i = BEGIN_FONTS_ADDRESS, i = 0; i <= END_FONTS_ADDRESS;
         i++, m_i++) {
        memory[m_i] = fonts[i];
    }

    font_loaded = true;

    // for (int i = BEGIN_FONTS_ADDRESS; i <= END_FONTS_ADDRESS; i++) {
    //     printf("%.2x: %.2x\n", i, memory[i]);
    // }
}

static void CPU_clear_screen_data(void) {
    for (int i = 0; i < CH8_SCREEN_ROWS; i++) {
        for (int j = 0; j < CH8_SCREEN_COLS; j++) {
            screen_data[i][j] = false;
        }
    }
}

static void CPU_draw_on_SDL_screen(Chip8 *ch8) {
    // set color to black (background)
    SDL_SetRenderDrawColor(ch8->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ch8->renderer);

    // set color to white (foreground)
    SDL_SetRenderDrawColor(ch8->renderer, 255, 255, 255, 255);
    SDL_Rect pixel;
    pixel.x = 0;
    pixel.y = 0;
    pixel.w = ch8->scale;
    pixel.h = ch8->scale;

    for (int i = 0; i < CH8_SCREEN_ROWS; i++) {
        for (int j = 0; j < CH8_SCREEN_COLS; j++) {
            pixel.x = j * ch8->scale;
            pixel.y = i * ch8->scale;
            if (screen_data[i][j]) {
                SDL_RenderFillRect(ch8->renderer, &pixel);
            }
        }
    }
}

/*
 * Set every register and memory to 0.
 */
static void CPU_clear_state(void) {
    // clear memory
    for (size_t i = 0; i > 0xFFF; i++) {
        memory[i] = 0;
    }
    rom_loaded = false;
    font_loaded = false;

    // clear v registers
    for (int i = 0; i < 0x10; i++) {
        v[i] = 0;
    }

    // clear stack
    for (int i = 0; i < 0x10; i++) {
        stack[i] = 0;
    }

    // clear I register
    I = 0;

    // clear delay and sound timers
    delayTimer = 0;
    soundTimer = 0;

    // clear program counter
    pc = 0;

    // clear stack pointer
    sp = 0;
}

static void CPU_decode_execute_instruction(uint16_t instruction, Chip8 *ch8) {
    uint16_t n1 = instruction & 0xf000;
    uint16_t n2 = instruction & 0x0f00;
    uint16_t n3 = instruction & 0x00f0;
    uint16_t n4 = instruction & 0x000f;

    printf("%.4x -> ", instruction);

    switch (n1) {
        case 0x0000:
            switch (instruction) {
                case 0x00E0:
                    // CLS
                    printf("CLS\n");

                    CPU_clear_screen_data();
                    CPU_draw_on_SDL_screen(ch8);
                    SDL_RenderPresent(ch8->renderer);
                    break;
                case 0x00EE:
                    // RET
                    break;
                default:
                    // SYS addr
                    printf("SYS addr\n");
                    pc = n2 | n3 | n4;
                    inc_pc = false;
                    break;
            }
            break;
        case 0x1000:
            // JP addr
            printf("JP addr\n");
            pc = n2 | n3 | n4;
            inc_pc = false;
            break;
        case 0x2000:
            // CALL addr
            break;
        case 0x3000:
            // SE Vx, byte
            break;
        case 0x4000:
            // SNE Vx, byte
            break;
        case 0x5000:
            // SE Vx, Vy
            break;
        case 0x6000:
            // LD Vx, byte
            printf("LD Vx, byte\n");
            v[n2 >> 8] = n3 | n4;
            break;
        case 0x7000:
            // ADD Vx, byter
            printf("ADD Vx, byte\n");
            v[n2 >> 8] += (n3 | n4);
            break;
        case 0x8000:
            switch (n4) {
                case 0x0000:
                    // LD Vx, Vy
                    break;
                case 0x0001:
                    // OR Vx, Vy
                    break;
                case 0x0002:
                    // AND Vx, Vy
                    break;
                case 0x0003:
                    // XOR Vx, Vy
                    break;
                case 0x0004:
                    // ADD Vx, Vy
                    break;
                case 0x0005:
                    // SUB Vx, Vy
                    break;
                case 0x0006:
                    // SHR Vx {, Vy}
                    break;
                case 0x0007:
                    // SUBN Vx, Vy
                    break;
                case 0x000E:
                    // SHL Vx {, Vy}
                    break;
                default:
                    printf("UNKNOWN INSTRUCTION: %.4x\n", instruction);
                    break;
            }
            break;
        case 0x9000:
            // SNE Vx, Vy
            break;
        case 0xa000:
            // LD I, addr
            printf("LD I, addr\n");
            I = n2 | n3 | n4;
            break;
        case 0xb000:
            // JP V0, addr
            break;
        case 0xc000:
            // RND Vx, byte
            break;
        case 0xd000:
            // DRW Vx, Vy, nibble
            printf("DRW Vx, Vy, nibble\n");

            /* i tracks the number of addresses to read from memory*/
            /*
             * j tracks each of the 8 bits of the current address (from left to
             * right)
             */

            // x = n2 >> 8;
            // y = n3 >> 4;
            // n = n4;
            for (int i = 0, row = v[n3 >> 4], address = I; i < n4;
                 i++, address++, row++) {
                /* wrap around */
                if (row >= CH8_SCREEN_ROWS) {
                    row -= CH8_SCREEN_ROWS;
                }
                for (int j = 0, col = v[n2 >> 8]; j < 8; col++, j++) {
                    /* wrap around */
                    if (col >= CH8_SCREEN_COLS) {
                        col -= CH8_SCREEN_COLS;
                    }

                    /*
                     * if an 'on' pixel is set to 'off', set VF to 1:
                     * i.e: if both are true
                     */

                    /* (memory[address] >> 7 - j) & 0x1 -> the single bit
                     * needed! */

                    if ((screen_data[row][col]) &&
                        (((memory[address] >> (7 - j)) & 0x1))) {
                        v[0xf] = 1;
                    }

                    /* set the data */
                    screen_data[row][col] ^=
                        (((memory[address] >> (7 - j)) & 0x1));
                }
            }

            // printf screen_data
            // for (int row = 0; row < CH8_SCREEN_ROWS; row++) {
            //     for (int col = 0; col < CH8_SCREEN_COLS; col++) {
            //         if (screen_data[row][col]) {
            //             printf("x");
            //         } else {
            //             printf(" ");
            //         }
            //     }
            //     printf("\n");
            // }

            // load the new screen
            CPU_draw_on_SDL_screen(ch8);
            SDL_RenderPresent(ch8->renderer);
            break;

        case 0xe000:
            switch (n3 | n4) {
                case 0x009e:
                    // SKP Vx
                    break;
                case 0x00A1:
                    // SKNP Vx
                    break;
                default:
                    printf("UNKNOWN INSTRUCTION: %.4x\n", instruction);
                    break;
            }
            break;
        case 0xf000:
            switch (n3 | n4) {
                case 0x0007:
                    // LD Vx, DT
                    break;
                case 0x000a:
                    // LD Vx, K
                    break;
                case 0x0015:
                    // LD DT, Vx
                    break;
                case 0x0018:
                    // LD ST, Vx
                    break;
                case 0x001e:
                    // ADD I, Vx
                    break;
                case 0x0029:
                    // LD F, Vx
                    break;
                case 0x0033:
                    // LD B, Vx
                    break;
                case 0x0055:
                    // LD [I], Vx
                    break;
                case 0x0065:
                    // LD, Vx, [I]
                    break;
                default:
                    printf("UNKNOWN INSTRUCTION: %.4x\n", instruction);
                    break;
            }
            break;
    }
}
