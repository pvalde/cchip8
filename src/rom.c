#include "rom.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Read rom binary file into Rom object.
 */
int ROM_read_rom(char *file_name, Rom *rom) {
    FILE *file_ptr = NULL;
    file_ptr = fopen(file_name, "rb");

    if (file_ptr == NULL) {
        printf("Error opening file\n");
        fclose(file_ptr);
        return 1;
    }

    // determine file size
    fseek(file_ptr, 0, SEEK_END);
    long file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);

    printf("file size: %ld bytes\n", file_size);

    // Dynamically allocate memory
    rom->data = (unsigned char *)malloc(file_size);

    if (rom->data == NULL) {
        perror("Failed to allocate memory");
        fclose(file_ptr);
        return 1;
    }

    rom->size = file_size;

    // read the file into the allocated array
    size_t bytes_read = fread(rom->data, 1, file_size, file_ptr);
    if (bytes_read != file_size) {
        perror("Failed to read file");
        free(rom->data);
        rom->data = NULL;
        fclose(file_ptr);
        return 1;
    }


    fclose(file_ptr);

    return 0;
}

/*
 * Free dynamically allocated data in Rom.data
 */
void ROM_deallocate_rom_data(Rom *rom) {
    free(rom->data);
    rom->data = NULL;
    rom->size = 0;
}

void ROM_print_rom_bytes(Rom *rom) {
    printf("rom size: %zu\n", rom->size);
    int counter = 0;
    for (size_t i = 0; i < rom->size; i++) {
        counter += 1;
        printf("%.2x ", rom->data[i]);
        if (counter == 8) {
            counter = 0;
            printf("\n");
        }
    }
    printf("\n");
}
