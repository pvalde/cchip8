#include "cpu.h"
#include "chip8.h"
#include "rom.h"
#include <SDL_events.h>
#include <SDL_render.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROM_ADDRESS 0x200
#define BEGIN_FONTS_ADDRESS 0x000
// #define END_FONTS_ADDRESS 0x050
#define CH8_SCREEN_COLS 64
#define CH8_SCREEN_ROWS 32

/*****************************************************************************
 * CPU internals
 ****************************************************************************/
static uint8_t memory[0xFFF] = {0};
static uint8_t v[0x10] = {0};
static uint16_t stack[0x10] = {0};
static uint16_t I = 0;
static uint8_t delay_timer = 0;
static uint8_t sound_timer = 0;
static uint16_t pc = 0;
static uint8_t sp = 0;
static bool screen_data[CH8_SCREEN_ROWS][CH8_SCREEN_COLS] = {false};
static bool font_loaded = false;
static bool rom_loaded = false;
static bool inc_pc = true;
static uint8_t random_number = 0;
static uint8_t key_pressed = 16;
static bool halt = false;
static uint8_t halt_key = 16;

/*****************************************************************************
 * static functions' declarations
 ****************************************************************************/
static void CPU_clear_state(void);
static void CPU_load_fonts(void);
static uint16_t CPU_get_instruction(void);
static void CPU_decode_execute_instruction(uint16_t instruction, Chip8 *ch8);
static void CPU_clear_screen_data(void);
static void CPU_draw_on_SDL_screen(Chip8 *ch8);
static uint8_t get_chip8_key(SDL_Event event);

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
    //      //printf("%.4x: %.2x\n", i, memory[i]);
    // }
}

void CPU_Execute_Instruction(Chip8 *ch8) {
    if (rom_loaded && font_loaded) {
        uint16_t instruction = CPU_get_instruction();
        CPU_decode_execute_instruction(instruction, ch8);
        if (!halt) {

            if (inc_pc) {
                pc += 2; // increments to next instruction
            } else {
                inc_pc = true;
            }
        }
        if (delay_timer) {
            delay_timer--;
        }
    } else {
        printf("no rom loaded");
        exit(1);
    }
}

static uint16_t CPU_get_instruction(void) {
    printf("addresses [0x%.2x-0x%.2x]: ", pc, pc + 1);
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

    for (int m_i = BEGIN_FONTS_ADDRESS, i = 0;
         i < (sizeof(fonts) / sizeof(fonts[0])); i++, m_i++) {
        memory[m_i] = fonts[i];
    }

    font_loaded = true;

    // for (int i = BEGIN_FONTS_ADDRESS; i <= END_FONTS_ADDRESS; i++) {
    //     //printf("%.2x: %.2x\n", i, memory[i]);
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
    delay_timer = 0;
    sound_timer = 0;

    // clear program counter
    pc = 0;

    // clear stack pointer
    sp = 0;
}

/**
 * If key pressed is mapped to chip8's keyboard, returns the value of the key
 * from 0 to F, else, returns 16.
 */
static uint8_t get_chip8_key(SDL_Event event) {
    switch (event.key.type) {
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

static void CPU_print_status(void) {
    // V registers:
    for (int i = 0; i < 16; i++) {
        if (i == 8) {
            printf("\n");
        } 
        printf("V%.1x: 0x%.2x ", i, v[i]);
    }
    printf("\n");
    printf("I: 0x%.3x ", I);
    printf("pc: 0x%.3x ", pc);
    printf("sp: 0x%.3x ", sp);
    printf("dt: 0x%.3x ", delay_timer);
    printf("st: 0x%.3x\n", sound_timer);
    printf("stack: ");
    for (int i = 0; i < sizeof(stack) / sizeof(stack[0]); i++) {
        if (i == 7) {
            printf("\n");
        } 
        printf("%.1x=0x%.3x ", i, stack[i]);
    }
    printf("\n");
    printf("-------\n");
}

static void CPU_decode_execute_instruction(uint16_t instruction, Chip8 *ch8) {
    uint16_t n1 = instruction & 0xf000;
    uint16_t n2 = instruction & 0x0f00;
    uint16_t n3 = instruction & 0x00f0;
    uint16_t n4 = instruction & 0x000f;

    printf("0x%.4x -> ", instruction);

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
                    printf("RET\n");
                    pc = stack[sp];
                    sp--;
                    /* inc_pc = false; */ /* --> this is commented out because
                                           * we don't want to repeat the
                                           * instruction we are returning to,
                                           * but to continue with the next after
                                           * it.
                                           */
                    break;
                default:
                    // SYS addr
                    printf("SYS addr=0x%.3x\n", n2 | n3 | n4);
                    pc = n2 | n3 | n4;
                    inc_pc = false;
                    break;
            }
            break;
        case 0x1000:
            // JP addr
            printf("JP addr=0x%.3x\n", n2 | n3 | n4);
            pc = n2 | n3 | n4;
            inc_pc = false;
            break;
        case 0x2000:
            // CALL addr
            printf("CALL addr=0x%.3x\n", n2 | n3 | n4);
            sp++;
            stack[sp] = pc;
            pc = n2 | n3 | n4;
            inc_pc = false;
            break;
        case 0x3000:
            // SE Vx, byte
            printf("SE V[x=0x%.2x]=0x%.2x, byte=0x%.2x\n", n2 >> 8, v[n2 >> 8],
                   n3 | n4);
            if ((v[n2 >> 8]) == (n3 | n4)) {
                pc += 2;
            }
            break;
        case 0x4000:
            // SNE Vx, byte
            printf("SNE V[x=0x%.2x]=0x%.2x, byte=0x%.2x\n", n2 >> 8, v[n2 >> 8],
                   n3 | n4);
            if ((v[n2 >> 8]) != (n3 | n4)) {
                pc += 2;
            }
            break;
        case 0x5000:
            // SE Vx, Vy
            printf("SE V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n", n2 >> 8,
                   v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
            if ((v[n2 >> 8]) == (v[n3 >> 4])) {
                pc += 2;
            }
            break;
        case 0x6000:
            // LD Vx, byte
            printf("LD V[x=0x%.2x]=0x%.2x, byte=0x%.2x\n", n2 >> 8, v[n2 >> 8],
                   n3 | n4);
            v[n2 >> 8] = n3 | n4;
            break;
        case 0x7000:
            // ADD Vx, byte
            printf("ADD V[x=0x%.2x]=0x%.2x, byte=0x%.2x\n", n2 >> 8, v[n2 >> 8],
                   n3 | n4);
            v[n2 >> 8] += (n3 | n4);
            break;
        case 0x8000:
            switch (n4) {
                case 0x0000:
                    // LD Vx, Vy
                    printf("LD V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    v[n2 >> 8] = v[n3 >> 4];
                    break;
                case 0x0001:
                    // OR Vx, Vy
                    printf("OR V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    v[n2 >> 8] |= v[n3 >> 4];
                    break;
                case 0x0002:
                    // AND Vx, Vy
                    printf("AND V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    v[n2 >> 8] &= v[n3 >> 4];
                    break;
                case 0x0003:
                    // XOR Vx, Vy
                    printf("XOR V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    v[n2 >> 8] ^= v[n3 >> 4];
                    break;
                case 0x0004:
                    // ADD Vx, Vy
                    printf("ADD V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    /* using uint16_t to test the overflow! */
                    if (((uint16_t)(v[n2 >> 8]) + v[n3 >> 4]) > 0xff) {
                        v[n2 >> 8] += v[n3 >> 4];
                        v[0xf] = 1;
                    } else {
                        v[n2 >> 8] += v[n3 >> 4];
                        v[0xf] = 0;
                    }
                    break;
                case 0x0005:
                    // SUB Vx, Vy
                    printf("SUB V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    {
                        uint8_t original_vx = v[n2 >> 8];

                        v[n2 >> 8] = v[n2 >> 8] - v[n3 >> 4];
                        (original_vx >= v[n3 >> 4]) ? (v[0xf] = 1)
                                                    : (v[0xf] = 0);
                    }
                    break;
                case 0x0006:
                    // SHR Vx {, Vy}
                    printf("SHR V[x=0x%.2x]=0x%.2x {, Vy}\n", n2 >> 8,
                           v[n2 >> 8]);
                    {
                        uint8_t original_vx = v[n2 >> 8];
                        v[n2 >> 8] /= 0x2;

                        (original_vx & 0x1) ? (v[0xf] = 1) : (v[0xf] = 0);
                    }
                    break;
                case 0x0007:
                    // SUBN Vx, Vy
                    printf("SUBN V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n",
                           n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
                    {
                        uint8_t original_vx = v[n2 >> 8];
                        v[n2 >> 8] = v[n3 >> 4] - v[n2 >> 8];
                        (v[n3 >> 4] >= original_vx) ? (v[0xf] = 1)
                                                    : (v[0xf] = 0);
                    }
                    break;
                case 0x000E:
                    // SHL Vx {, Vy}
                    printf("SHL V[x=0x%.2x]=0x%.2x {, Vy}\n", n2 >> 8,
                           v[n2 >> 8]);
                    {
                        uint8_t original_vx = v[n2 >> 8];
                        v[n2 >> 8] *= 0x2;
                        ((original_vx & 0x80) >> 7) ? (v[0xf] = 1)
                                                    : (v[0xf] = 0);
                    }
                    break;
                default:
                    printf("UNKNOWN INSTRUCTION: %.4x\n", instruction);
                    break;
            }
            break;
        case 0x9000:
            // SNE Vx, Vy
            printf("SNE V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x\n", n2 >> 8,
                   v[n2 >> 8], n3 >> 4, v[n3 >> 4]);
            if (v[n2 >> 8] != v[n3 >> 4]) {
                pc += 2;
            }
            break;
        case 0xa000:
            // LD I, addr
            printf("LD I, addr=0x%.3x\n", n2 | n3 | n4);
            I = n2 | n3 | n4;
            break;
        case 0xb000:
            // JP V0, addr
            printf("JP V0=0x%.2x, addr=0x%.3x\n", v[0], n2 | n3 | n4);
            pc = (n2 | n3 | n4) + v[0x0];
            inc_pc = false;
            break;
        case 0xc000:
            // RND Vx, byte
            printf("RND V[x=0x%.2x]=0x%.2x, byte=0x%.2x\n", n2 >> 8, v[n2 >> 8],
                   n3 | n4);
            srand(time(NULL));
            random_number = rand() % (0xff - 0x0 + 1) + 0x0;
            v[n2 >> 8] = random_number & (n3 | n4);
            break;
        case 0xd000:
            // DRW Vx, Vy, nibble
            printf(
                "DRW V[x=0x%.2x]=0x%.2x, V[y=0x%.2x]=0x%.2x, nibble=0x%.2x\n",
                n2 >> 8, v[n2 >> 8], n3 >> 4, v[n3 >> 4], n4);

            // i tracks the number of addresses to read from memory
            // j tracks each of the 8 bits of the current address (from left to
            // right)

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
                     * i.e: if both are true (in that case xor returns 0 when
                     * the current pixel was 1)
                     */
                    if ((screen_data[row][col]) &&
                        (((memory[address] >> (7 - j)) & 0x1))) {
                        /* (memory[address] >> 7 - j) & 0x1 -> the single bit
                         * needed! */
                        v[0xf] = 1;
                    }

                    /* set the data */
                    screen_data[row][col] ^=
                        (((memory[address] >> (7 - j)) & 0x1));
                }
            }

            // load the new screen
            CPU_draw_on_SDL_screen(ch8);
            SDL_RenderPresent(ch8->renderer);
            break;

        case 0xe000:
            switch (n3 | n4) {
                case 0x009e:
                    // SKP Vx
                    printf("SKP V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    if (ch8->keyboard_state[v[n2 >> 8]]) {
                        pc += 2;
                    }
                    break;
                case 0x00A1:
                    // SKNP Vx
                    printf("SKNP V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    if (!ch8->keyboard_state[v[n2 >> 8]]) {
                        pc += 2;
                    }
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
                    printf("LD V[x=0x%.2x]=0x%.2x, DT\n", n2 >> 8, v[n2 >> 8]);
                    v[n2 >> 8] = delay_timer;
                    break;
                case 0x000a:
                    // LD Vx, K
                    // TODO
                    // Delay timer must continue
                    // once the key is released the execution resumes.
                    printf("LD V[x=0x%.2x]=0x%.2x, K\n", n2 >> 8, v[n2 >> 8]);
                    if (halt && (halt_key < 16)) {
                        if (!ch8->keyboard_state[halt_key]) {
                            halt = false;
                            halt_key = 16;
                            return;
                        }
                    }
                    halt = true;
                    for (uint8_t i = 0; i < 16; i++) {
                        if (ch8->keyboard_state[i]) {
                            v[n2 >> 8] = i;
                            halt_key = i;
                        }
                    }
                    break;
                case 0x0015:
                    // LD DT, Vx
                    printf("LD DT, V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    delay_timer = v[n2 >> 8];
                    break;
                case 0x0018:
                    // LD ST, Vx
                    printf("LD ST, V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    sound_timer = v[n2 >> 8];
                    break;
                case 0x001e:
                    // ADD I, Vx
                    printf("ADD I, V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    I += v[n2 >> 8];
                    break;
                case 0x0029:
                    // LD F, Vx
                    printf("LD F, V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    I = BEGIN_FONTS_ADDRESS + (v[n2 >> 8] * 5);
                    break;
                case 0x0033:
                    // LD B, Vx
                    printf("LD B, V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    memory[I] = v[n2 >> 8] / 100;
                    memory[I + 1] = (v[n2 >> 8] % 100) / 10;
                    memory[I + 2] = v[n2 >> 8] % 10;

                    break;
                case 0x0055:
                    // LD [I], Vx
                    printf("LD [I], V[x=0x%.2x]=0x%.2x\n", n2 >> 8, v[n2 >> 8]);
                    for (uint16_t i = 0x00; i <= (n2 >> 8); i++) {
                        memory[I + i] = v[i];
                    }
                    break;
                case 0x0065:
                    // LD Vx, [I]
                    printf("LD V[x=0x%.2x]=0x%.2x, [I]\n", n2 >> 8, v[n2 >> 8]);
                    for (uint16_t i = 0x0; i <= (n2 >> 8); i++) {
                        v[i] = memory[I + i];
                    }
                    break;
                default:
                    printf("UNKNOWN INSTRUCTION: %.4x\n", instruction);
                    break;
            }
            break;
    }
    CPU_print_status();
}
