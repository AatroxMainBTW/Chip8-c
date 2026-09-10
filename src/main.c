#include "chip8.h"

int main(int argc, char **argv){
    struct CHIP8 chip_8;
    printf("Starting..... \n");
    initialize_chip8(&chip_8);
    
    load_rom("/Users/fouzi/C-Chip-8/test_roms/chip8-logo.ch8", &chip_8);
    
    while (true) {
        emulate_cpu_cycle(&chip_8);
    }
}
