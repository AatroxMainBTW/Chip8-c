#pragma once

#define VIDEO_SCALE 10
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32

#include<stdint.h>
#include<stdio.h>
#include <stdbool.h>

struct CHIP8 {
    //Display:chip8 uses 64 * 32 pixels which is 2048
    uint8_t gfx[VIDEO_WIDTH * VIDEO_HEIGHT];
    uint8_t keys[16];
    //— general-purpose; games read it to time events.
    uint8_t delay_timer;
    //while > 0, the console beeps. That's the only sound.
    uint8_t sound_timer;
    bool draw_flag;
    //4KB of memory
    uint8_t memory[4096];
    //16 registers V0-VF all of them of 8bits General purpose. VF is the flag register (carry / borrow / collision) — don't use it for storage.
    uint8_t V[16];
    //Index register. Points into memory for draws, BCD, load/store.
    uint16_t I;
    //Program counter. Starts at 0x200.
    uint16_t PC;
    uint16_t stack[16];
    //Stack pointer.
    uint8_t SP;
};

//To load a rom of chip8 game
void load_rom(const char* filename, struct CHIP8* cpu);


//To Initialize the chip8 real values.
void initialize_chip8(struct CHIP8* cpu);


//To read opcodes

void emulate_cpu_cycle(struct CHIP8* cpu);
