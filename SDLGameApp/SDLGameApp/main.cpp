#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <time.h>

#include "GameManager.h"

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

    ResourceManager::LoadFont("Assets/Fonts/DotGothic16-Regular.ttf");

    bool isRunning = true;
    bool isPaused = false;
    SDL_Event event;

    const int frameDelay = 1000 / 60;
    Uint32 frameStart, frameTime, lastFrameTime = 0;
    float deltaTime;

    GameManager GM = GameManager(renderer);

    /*--------------------------------------"Game Loop"-------------------------------------*/

    while (isRunning) {

        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_FOCUS_LOST:
                case SDL_WINDOWEVENT_MINIMIZED:
                    isPaused = true;
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                case SDL_WINDOWEVENT_RESTORED:
                    isPaused = false;
                    break;
                }
            }
        }

        if (!isPaused) {

            deltaTime = (frameStart - lastFrameTime) / 1000.0f;
            lastFrameTime = frameStart;

            //シーン遷移をするときこのフレームは更新しない
            if (!GM.CheckTransision()) {

                //ゲームの更新
                GM.Update(deltaTime);
            };
                                           
            SDL_RenderClear(renderer);         

            //Render処理
            GM.Render();

            SDL_RenderPresent(renderer);
        }

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime < frameDelay) {
            SDL_Delay(frameDelay - frameTime);
        }
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
