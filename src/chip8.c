#include <string.h>
#include "chip8.h"
/*
 PRIVATE FUNC FOR OPCODES
 */

//region System
void op_0x00E0(struct CHIP8* cpu){
    memset(cpu->gfx, 0, sizeof(cpu->gfx));
    cpu->draw_flag = true;
}

void op_0x00EE(struct CHIP8* cpu){
    cpu->SP--;
    cpu->PC = cpu->stack[cpu->SP];
}

//endregion

// region Register / constant ops
void op_0x6XNN(struct CHIP8* cpu, uint16_t opcode){
    cpu->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
}

void op_0xANNN(struct CHIP8* cpu, uint16_t opcode){
    cpu->I = opcode & 0x0FFF;
}

void op_0x7XNN(struct CHIP8* cpu, uint16_t opcode){
    cpu->V[(opcode & 0x0F00) >> 8] +=  opcode & 0x00FF;
}
//endregion

//region Flow Control
void op_0x1NNN(struct CHIP8* cpu, uint16_t opcode){
    cpu->PC = opcode & 0x0FFF;
}
void op_0xBNNN(struct CHIP8* cpu, uint16_t opcode) {
    cpu->PC = (opcode & 0x0FFF) + cpu->V[0];
}
void op_0xCXNN(struct CHIP8* cpu, uint16_t opcode) {
    uint8_t X = (opcode & 0x0F00) >> 8;
    cpu->V[X] = (rand() % 256) & (opcode & 0x00FF);
}
//endregion

//region GFX
void op_0xDXYN(struct CHIP8* cpu, uint16_t opcode) {
    uint8_t X = cpu->V[(opcode & 0x0F00) >> 8];
    uint8_t Y = cpu->V[(opcode & 0x00F0) >> 4];
    //How much rows
    uint8_t N = opcode & 0x000F;
    //Collision
    cpu->V[0xF] = 0;
    for (int row = 0; row <= N-1; row++) {
        uint8_t sprite_byte = cpu->memory[row + cpu->I];

        for (int col = 0; col <= 7; col++) {
            int sprite_bit = sprite_byte & (0x80 >> col);
            if (sprite_bit != 0) {
                int pos_x = (X + col) % 64;
                int pos_y = (Y + row) % 32;
                int index = pos_y * 64 + pos_x;

                if (cpu->gfx[index] == 1) {
                    cpu->V[0xF] = 1;
                }
                cpu->gfx[index] ^= 1;
            }
        }
        cpu->draw_flag = true;
    }
}
//endregion

//region Conditional skips
void op_0x3XNN(struct CHIP8* cpu, uint16_t opcode) {
    uint8_t NN = opcode & 0x00FF;
    uint8_t X = (opcode & 0x0F00) >> 8;
    if (cpu->V[X] == NN) cpu->PC += 2;
}
void op_0x4XNN(struct CHIP8* cpu, uint16_t opcode) {
    uint8_t NN = opcode & 0x00FF;
    uint8_t X = (opcode & 0x0F00) >> 8;
    if (cpu->V[X] != NN) cpu->PC += 2;
}
void op_0x5XY0(struct CHIP8* cpu, uint16_t opcode) {
    uint8_t X = (opcode & 0x0F00) >> 8;
    uint8_t Y = (opcode & 0x00F0) >> 4;

    if (cpu->V[X] == cpu->V[Y]) cpu->PC += 2;
}
void op_0x9XY0(struct CHIP8* cpu, uint16_t opcode) {
    uint8_t X = (opcode & 0x0F00) >> 8;
    uint8_t Y = (opcode & 0x00F0) >> 4;

    if (cpu->V[X] != cpu->V[Y]) cpu->PC += 2;
}
void op_0xEX9E(struct CHIP8* cpu, uint8_t key_index) {
    if (cpu->keys[key_index] == 1) {
        cpu->PC += 2;
    }
}
void op_0xEXA1(struct CHIP8* cpu, uint8_t key_index) {
    if (cpu->keys[key_index] == 0) {
        cpu->PC += 2;
    }
}   
//endregion

//region Timers & key
void op_0xFX07(struct CHIP8* cpu, uint8_t x) {
    cpu->V[x] = cpu->delay_timer;
}
void op_0xFX0A(struct CHIP8* cpu, uint8_t x) {
    bool key_pressed = false;

    for (int key_index = 0; key_index < 16; key_index++) {
        if (cpu->keys[key_index] == 1) {
            cpu->V[x] = key_index;
            key_pressed = true;
            break;      
        }
    }
    if (!key_pressed) cpu->PC -= 2;
}
void op_0xFX15(struct CHIP8* cpu, uint8_t x) {
    cpu->delay_timer = cpu->V[x];
}
void op_0xFX18(struct CHIP8* cpu, uint8_t x) {
    cpu->sound_timer = cpu->V[x];
}
void op_0xFX1E(struct CHIP8* cpu, uint8_t x) {
    cpu->I += cpu->V[x];
}
void op_0xFX29(struct CHIP8* cpu, uint8_t x) {
    cpu->I = 0x050 + (cpu->V[x] * 5);
}
void op_0xFX33(struct CHIP8* cpu, uint8_t x) {
    cpu->memory[cpu->I] = cpu->V[x] / 100;
    cpu->memory[cpu->I+1] = (cpu->V[x] / 10) % 10;
    cpu->memory[cpu->I+2] = cpu->V[x] % 10;
}

void op_0xFX55(struct CHIP8* cpu, uint8_t x) {
    for (int i = 0; i <= x; i++) {
        cpu->memory[cpu->I + i] = cpu->V[i];
    }
}

void op_0xFX65(struct CHIP8* cpu, uint8_t x) {
    for (int i = 0; i <= x; i++) {
        cpu->V[i] = cpu->memory[cpu->I + i];
    }
}
//endregion

//region subroutine
void op_0x2NNN(struct CHIP8* cpu, uint16_t opcode) {
    uint16_t NNN = opcode & 0x0FFF;
    cpu->stack[cpu->SP] = cpu->PC;
    cpu->SP++;
    cpu->PC = NNN;
}
//endregion

//region Arithmetic / logic (`8XY_`)
void op_0x8XY0(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    cpu->V[x] = cpu->V[y];
}
void op_0x8XY1(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    cpu->V[x] = cpu->V[x] | cpu->V[y];
}
void op_0x8XY2(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    cpu->V[x] = cpu->V[x] & cpu->V[y];
}
void op_0x8XY3(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    cpu->V[x] = cpu->V[x] ^ cpu->V[y];
}
void op_0x8XY4(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    uint16_t  sum = cpu->V[x] + cpu->V[y];
    cpu->V[x] = sum & 0xFF;
    cpu->V[0xF] = (sum > 255) ? 1 : 0;
}
void op_0x8XY5(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    uint8_t flag = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
    cpu->V[x] = cpu->V[x] - cpu->V[y];
    cpu->V[0xF] = flag;
}   
void op_0x8XY6(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    uint8_t flag = cpu->V[x] & 1;
    cpu->V[x] = cpu->V[x] >> 1;
    cpu->V[0xF] = flag;
}
void op_0x8XY7(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    uint8_t flag = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
    cpu->V[x] = cpu->V[y] - cpu->V[x];
    cpu->V[0xF] = flag;
}
void op_0x8XYE(struct CHIP8* cpu, uint8_t x, uint8_t y) {
    uint8_t flag = (cpu->V[x] >> 7) & 1;
    cpu->V[x] = cpu->V[x] << 1;
    cpu->V[0xF] = flag;
}
//endregion


void initialize_chip8(struct CHIP8* cpu){
    memset(cpu->gfx, 0, sizeof(cpu->gfx));
    memset(cpu->memory, 0, sizeof(cpu->memory));
    memset(cpu->V, 0, sizeof(cpu->V));
    memset(cpu->stack, 0, sizeof(cpu->stack));
    memset(cpu->keys, 0, sizeof(cpu->keys));
    cpu->I = 0;
    cpu->PC = 0x200;
    cpu->SP = 0;
    cpu->delay_timer = 0;
    cpu->sound_timer = 0;
    cpu->draw_flag = false;

    uint8_t fontset[80] = {
   0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
   0x20, 0x60, 0x20, 0x20, 0x70, // 1
   0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
   0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
   0x90, 0x90, 0xF0, 0x10, 0x10, // 4
   0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
   0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
   0xF0, 0x10, 0x20, 0x40, 0x40, // 7
   0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
   0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
   0xF0, 0x90, 0xF0, 0x90, 0x90, // A
   0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
   0xF0, 0x80, 0x80, 0x80, 0xF0, // C
   0xE0, 0x90, 0x90, 0x90, 0xE0, // D
   0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
   0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++) {
       cpu->memory[0x050 + i] = fontset[i];
    }
}

void load_rom(const char* filename, struct CHIP8* cpu){
    FILE *input_file = fopen(filename, "rb");
    if(input_file == NULL){
        return;
    }
    fseek(input_file, 0L, SEEK_END);
    size_t sz = ftell(input_file);
    fseek(input_file, 0L, SEEK_SET);
    fread(&cpu->memory[0x200], 1, sz, input_file);
    fclose(input_file);
}

void emulate_cpu_cycle(struct CHIP8* cpu){
    uint16_t opcode = cpu->memory[cpu->PC] << 8 | cpu->memory[cpu->PC +1];
    cpu ->PC+=2;
    switch (opcode & 0xF000) {
    case 0x0000: {
        switch (opcode & 0x00FF) {
        case 0x00E0:
            op_0x00E0(cpu);
            break;
        case 0x00EE:
            op_0x00EE(cpu);
            break;
        default:

            break;
        }
        break;
        }
        case 0x1000:
            op_0x1NNN(cpu, opcode);
            break;
        case 0x2000:
            op_0x2NNN(cpu, opcode);
            break;
        case 0x3000:
            op_0x3XNN(cpu, opcode);
            break;
        case 0x4000:
            op_0x4XNN(cpu, opcode);
            break;
        case 0x5000:
            op_0x5XY0(cpu, opcode);
            break;
        case 0x6000:
            op_0x6XNN(cpu, opcode);          
            break;
        case 0x7000:
            op_0x7XNN(cpu, opcode);   
            break;

        case 0x8000: {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            uint8_t digit = (opcode & 0x000F);
            switch (digit) {
            case 0x0:
                op_0x8XY0(cpu, X, Y);
                break;
            case 0x1:
                op_0x8XY1(cpu, X, Y);
                break;
            case 0x2:
                op_0x8XY2(cpu, X, Y);
                break;
            case 0x3:
                op_0x8XY3(cpu, X, Y);
                break;
            case 0x4:
                op_0x8XY4(cpu, X, Y);
                break;
            case 0x5:
                op_0x8XY5(cpu, X, Y);
                break;
            case 0x6:
                op_0x8XY6(cpu, X, Y);
                break;
            case 0x7:
                op_0x8XY7(cpu, X, Y);
                break;
            case 0xE:
                op_0x8XYE(cpu, X, Y);
                break;
            }
                break;
        }  
        case 0x9000:
            op_0x9XY0(cpu, opcode);
            break;
        case 0xA000:
            op_0xANNN(cpu, opcode);        
            break;
        case 0xB000:
            op_0xBNNN(cpu, opcode);
            break;
        case 0xC000:
            op_0xCXNN(cpu, opcode);
            break;
        case 0xD000:
            op_0xDXYN(cpu, opcode);
            break;
        case 0xE000: {
            uint8_t key_index = cpu->V[(opcode & 0x0F00) >> 8];
            switch (opcode & 0x00FF) {
            case 0x9E:
                op_0xEX9E(cpu, key_index);
                break;
            case 0xA1:
                op_0xEXA1(cpu, key_index);
                break;
            }
            break;
        }

        case 0xF000: {
            uint8_t X = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
            case 0x07:
                op_0xFX07(cpu, X);
                break;
            case 0x0A:
                op_0xFX0A(cpu, X);
                break;
            case 0x15:
                op_0xFX15(cpu, X);
                break;
            case 0x18:
                op_0xFX18(cpu, X);
                break;
            case 0x1E:
                op_0xFX1E(cpu, X);
                break;
            case 0x29:
                op_0xFX29(cpu, X);
                break;
            case 0x33:
                op_0xFX33(cpu, X);
                break;
            case 0x55:
                op_0xFX55(cpu, X);
                break;
            case 0x65:
                op_0xFX65(cpu, X);
                break;
            }
            break;
        }
        default:
            printf("UNKNOWN opcode: 0x%04X\n", opcode);
            break;
    }
}
