#include <SDL.h>
#include <iostream>

// �O���b�h�̃T�C�Y
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

    // �`�惂�[�h��ݒ肵�āA�A���t�@�l��L���ɂ���
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool running = true;
    SDL_Event event;

    // �t�F�[�h�C���p�̃A���t�@�l
    Uint8 alpha[GRID_ROWS][GRID_COLS] = { {0} };

    Uint32 lastTime = SDL_GetTicks(); // ���ԊǗ�

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // �b�P��
        lastTime = currentTime;

        // �w�i���N���A
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // �O���b�h��`��
        for (int row = 0; row < GRID_ROWS; ++row) {
            for (int col = 0; col < GRID_COLS; ++col) {
                // �t�F�[�h�C������
                if (alpha[row][col] < 255) {
                    alpha[row][col] += static_cast<Uint8>(50 * deltaTime); // �A���t�@�l�����X�ɑ���
                    if (alpha[row][col] > 255) {
                        alpha[row][col] = 255; // �����255��
                    }
                }

                // �O���b�h�̐F��ݒ�
                SDL_SetRenderDrawColor(renderer, 100, 150, 200, alpha[row][col]);

                // �O���b�h�̋�`��`��
                SDL_Rect gridRect = { col * GRID_WIDTH, row * GRID_HEIGHT, GRID_WIDTH, GRID_HEIGHT };
                SDL_RenderFillRect(renderer, &gridRect);
            }
        }

        // ��ʍX�V
        SDL_RenderPresent(renderer);

        // �t���[�����[�g����
        SDL_Delay(16); // ��60FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
