#ifndef ROM_H
#define ROM_H

#include <stddef.h>

typedef struct Rom {
    unsigned char *data;
    size_t size;
} Rom;

int ROM_read_rom(char *file_name, Rom *rom);

void ROM_deallocate_rom_data(Rom *rom);

void ROM_print_rom_bytes(Rom *rom);

#endif
