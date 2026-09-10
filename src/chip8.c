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
#ifdef DEBUG
            printf("System: 0x00E0 executed. \n");
#endif
            break;
        case 0x00EE:
            op_0x00EE(cpu);

#ifdef DEBUG
            printf("System: 0x00EE executed. \n");
#endif
            break;
        default:

            break;
        }
        break;
        }
        case 0x1000:
            op_0x1NNN(cpu, opcode);
            
#ifdef DEBUG
            printf("0x1NNN executed. \n");
#endif
            break;
        case 0x6000:
            op_0x6XNN(cpu, opcode);          
#ifdef DEBUG
            printf("0x6XNN executed. \n");
#endif
            break;
        case 0x7000:
            op_0x7XNN(cpu, opcode);   
#ifdef DEBUG
            printf("0x7XNN executed. \n");
#endif
            break;
        case 0xA000:
            op_0xANNN(cpu, opcode);        
#ifdef DEBUG
            printf("0xANNN executed. \n");
#endif
            break;
        case 0xD000:
            op_0xDXYN(cpu, opcode);
#ifdef DEBUG
            printf("0xDXYN executed. \n");
#endif
            break;
        case 0xE000: {
            uint8_t key_index = cpu->V[(opcode & 0x0F00) >> 8];
            switch (opcode & 0x00FF) {
            case 0x9E:
                op_0xEX9E(cpu, key_index);
#ifdef DEBUG
                printf("0xEX9E executed. \n");
#endif
                break;
            case 0xA1:
                op_0xEXA1(cpu, key_index);
#ifdef DEBUG
                printf("0xEXA1 executed. \n");
#endif
                break;
            }
            break;
        }

        case 0xF000: {
            uint8_t X = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
            case 0x0A:
                op_0xFX0A(cpu, X);
#ifdef DEBUG
                printf("0xFX0A executed. \n");
#endif

                break;
            }
            break;
        }
        default:
            printf("UNKNOWN opcode: 0x%04X\n", opcode);
            break;
    }
}
