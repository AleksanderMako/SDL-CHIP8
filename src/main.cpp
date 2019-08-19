#include <iostream>
#include <fstream>
#include "chip8.h"
#include <string>
#include <SDL2/SDL.h>
#include "stdint.h"
#include <chrono>
#include <thread>
using namespace std;
// Keypad keymap
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};
int main(int argc, char **argv)
{
    // Command usage
    if (argc != 2)
    {
        cout << "Usage: chip8 <ROM file>" << endl;
        return 1;
    }

    Chip8 c8 = Chip8();
    //c8.Initialize();
    int w = 1024; // Window width
    int h = 512;  // Window height
    SDL_Window *window = NULL;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        w, h, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n",
               SDL_GetError());
        exit(2);
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, w, h);

    // Create texture that stores frame buffer
    SDL_Texture *sdlTexture = SDL_CreateTexture(renderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                64, 32);

    // Temporary pixel buffer
    uint32_t pixels[2048];

load:
    // attempt to load rom
    if (!c8.LoadRom(argv[1]))
        return 2;

    // game loop

    while (true)
    {
        c8.Emulate();

        SDL_Event ev;

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
                exit(0);

            if (ev.type == SDL_KEYDOWN)
            {
                cout << "key press detected "<<endl; 
                if (ev.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                if (ev.key.keysym.sym == SDLK_F1)
                    goto load;

                for (int i = 0; i < 16; ++i)
                {
                    if (ev.key.keysym.sym == keymap[i])
                    {
                        c8.keypad[i] = 1;
                    }
                }
            }

            if (ev.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (ev.key.keysym.sym == keymap[i])
                    {
                        c8.keypad[i] = 0;
                    }
                }
            }
        }
        if (c8.drawFlag)
        {
            c8.drawFlag = false;
            for (int i = 0; i < 2048; ++i)
            {
                uint8_t pixel = c8.gfx_buffer[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            // Update SDL texture
            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
            // Clear screen and render
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(2000));
    }
}