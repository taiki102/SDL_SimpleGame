#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <time.h>

#include "Game.h"

#define WINDOW_SIZE_X 800
#define WINDOW_SIZE_Y 600

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Quit();
        return -1;
    }
    if (TTF_Init() == -1) {
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(GAMENAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_SIZE_X, WINDOW_SIZE_Y, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* texture = ResourceManager::LoadTexture("Assets/Images/chara_anim.png",renderer);
    ResourceManager::LoadFont("Assets/Fonts/DotGothic16-Regular.ttf");


    bool isRunning = true;
    SDL_Event event;
    const float deltaTime = 1.0f / 60.0f;
    float accumulator = 0.0f;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 previousTime = currentTime;

    float startTime = 0;

    SDL_Texture* textTexture = nullptr;
    SDL_Rect textRect;
    Size textsize;

    /*--------------------------------------"Game Loop"-------------------------------------*/

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        currentTime = SDL_GetTicks();
        float frameTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;
        accumulator += frameTime;

        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 86, 157, 225, 255);  // 背景

        std::string displayText = "タイム: " + std::to_string((currentTime - startTime) / 1000);

        if (textTexture) {
            SDL_DestroyTexture(textTexture);
        }
        textTexture = ResourceManager::LoadFontTexture(displayText.c_str(), SDL_Color{ 255, 255, 255, 255 }, &textsize, renderer);
        textRect = {100,100,600,200};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect); 

        SDL_RenderPresent(renderer);
    }

    /*--------------------------------------"Game Loop"-------------------------------------*/

    ResourceManager::CleanUp();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
