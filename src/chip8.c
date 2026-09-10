#include <string.h>
#include "chip8.h"
/*
 PRIVATE FUNC FOR OPCODES
 */

//region System
void op_0x00E0(struct CHIP8* cpu){
    memset(cpu->memory, 0, sizeof(cpu->memory));
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
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    op_0x00E0(cpu);
                    //printf("System: 0x00E0 executed. \n");
                    break;
                case 0x00EE:
                    op_0x00EE(cpu);
                    //printf("System: 0x00EE executed. \n");
                    break;
                default:
                    
                    break;
            }
            printf("System: %02X%02X NOP\n", cpu->memory[cpu->PC], cpu->memory[cpu->PC +1]);
            break;
        case 0x1000:
            op_0x1NNN(cpu, opcode);
            printf("0x1NNN executed. \n");
            break;
        case 0x6000:
            op_0x6XNN(cpu, opcode);
            printf("0x6XNN executed. \n");
            break;
        case 0x7000:
            op_0x7XNN(cpu, opcode);
            printf("0x7XNN executed. \n");
            break;
        case 0xA000:
            op_0xANNN(cpu, opcode);
            printf("0xANNN executed. \n");
            break;
        default:
            printf("0xF000 NOP. \n");
            break;
    }
}
