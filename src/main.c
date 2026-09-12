#include <SDL2/SDL.h>
#include "chip8.h"


#define INSTRUCTIONS_PER_FRAME 11   
int main(int argc, char **argv){
    struct CHIP8 chip_8;
    printf("Starting..... \n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    const SDL_Scancode keymap[16] = {
     SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
     SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
     SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
     SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V
    };

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",           // Window title
        SDL_WINDOWPOS_CENTERED,          // X position
        SDL_WINDOWPOS_CENTERED,          // Y position
        800,                              // Width
        600,                              // Height
        SDL_WINDOW_SHOWN                  // Flags
    );

    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;

    initialize_chip8(&chip_8);

    load_rom("C:/Users/Fouzi/Chip8-c/test_roms/Pong.ch8", &chip_8);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);  
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                bool pressed = (event.type == SDL_KEYDOWN);
                SDL_Scancode scancode = event.key.keysym.scancode;
                for (int i = 0; i < 16; ++i) {
                    if (scancode == keymap[i]) {
                        chip_8.keys[i] = pressed;
                        printf("key %X = %d\n", i, pressed); 
                    }
                }
            }
        }
         

        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) emulate_cpu_cycle(&chip_8);

        if (chip_8.delay_timer > 0) --chip_8.delay_timer;
        if (chip_8.sound_timer > 0) {
            --chip_8.sound_timer;
            if (chip_8.sound_timer == 0) {
                printf("BEEP\n");
            }
        }

        uint32_t pixels[64 * 32];
        for (int i = 0; i < 64 * 32; i++)
            pixels[i] = chip_8.gfx[i] ? 0xFFFFFFFF : 0x000000FF;

        SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL); 
        SDL_RenderPresent(renderer);
        
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


}
