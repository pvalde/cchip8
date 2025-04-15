#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "rom.h"
#include "chip8.h"

void CPU_load_rom(Rom *rom);
void CPU_Execute_Instruction(Chip8 *ch8);

#endif // !CPU_H
