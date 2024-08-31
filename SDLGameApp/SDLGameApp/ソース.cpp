#include <SDL.h>
#include <iostream>

// グリッドのサイズ
const int GRID_ROWS = 10;
const int GRID_COLS = 10;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GRID_WIDTH = SCREEN_WIDTH / GRID_COLS;
const int GRID_HEIGHT = SCREEN_HEIGHT / GRID_ROWS;

int main2(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Grid Background Effect",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 描画モードを設定して、アルファ値を有効にする
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool running = true;
    SDL_Event event;

    // フェードイン用のアルファ値
    Uint8 alpha[GRID_ROWS][GRID_COLS] = { {0} };

    Uint32 lastTime = SDL_GetTicks(); // 時間管理

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // 秒単位
        lastTime = currentTime;

        // 背景をクリア
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // グリッドを描画
        for (int row = 0; row < GRID_ROWS; ++row) {
            for (int col = 0; col < GRID_COLS; ++col) {
                // フェードイン効果
                if (alpha[row][col] < 255) {
                    alpha[row][col] += static_cast<Uint8>(50 * deltaTime); // アルファ値を徐々に増加
                    if (alpha[row][col] > 255) {
                        alpha[row][col] = 255; // 上限を255に
                    }
                }

                // グリッドの色を設定
                SDL_SetRenderDrawColor(renderer, 100, 150, 200, alpha[row][col]);

                // グリッドの矩形を描画
                SDL_Rect gridRect = { col * GRID_WIDTH, row * GRID_HEIGHT, GRID_WIDTH, GRID_HEIGHT };
                SDL_RenderFillRect(renderer, &gridRect);
            }
        }

        // 画面更新
        SDL_RenderPresent(renderer);

        // フレームレート制御
        SDL_Delay(16); // 約60FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
