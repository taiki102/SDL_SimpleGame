#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <time.h>

#define WINDOW_SIZE_X 800
#define WINDOW_SIZE_Y 600

#define MAX_CLOUD 6
#define MAX_EBI 5
#define MAX_ENEMY 10
#define MAX_SUBIMAGE 3

using uINT = unsigned __int8;


struct Size{
    int w, h = 0;
};

//当たり判定を取らない,個々に速度を持たないものをImageと定義する
struct Image {
    Point point;
    SDL_Rect rect;
};

//アニメーション用　テクスチャは参照のみを扱う
class Animation {
private :
    std::array<SDL_Texture*, MAX_SUBIMAGE> texturePool = {};
    uINT texturePoolCount = 0;
    uINT CurrentTexture = 0;
    float frameCount = 0;
    float animDuration = 0;
public :
    //配列初期化可能にする為に空のコンストラクタをおく
    Animation(){}
    Animation(float duration, const std::vector<SDL_Texture*>& textures) : animDuration(duration) {
        for (SDL_Texture* tex : textures) {
            AddAnim(tex);
        }
    }
    void InitAnimDuration(float duration) {
        animDuration = duration;
    }
    bool AddAnim(SDL_Texture* tex) {
        if (texturePoolCount < MAX_SUBIMAGE) {
            texturePool[texturePoolCount++] = tex;
            return true;
        }
        return false;
    }
    SDL_Texture* GetAnim(float deltaTime) {
        frameCount += deltaTime;
        if (frameCount > animDuration) {
            frameCount = 0;
            CurrentTexture++;
            if (CurrentTexture >= texturePoolCount) {
                CurrentTexture = 0;
            }
        }
        return texturePool[CurrentTexture];
    }
};

//ベクトル用　テクスチャの回転を行いたい
struct Vector {
    float x, y = 0;
    float length() const {
        return std::sqrt(x * x + y * y);
    }
    void normalize() const {
        float len = length();
        len > 0 ? x / len, y / len : x , y; 
    }
    void fromAngle(float angleInDegrees) {
        float angleInRadians = angleInDegrees * (M_PI / 180.0f);
        x = std::cos(angleInRadians), y = std::sin(angleInRadians);
    }
};

//当たり判定をとるものをObjectと定義する
struct Object : public Animation, public Image {
    float col_radius = 0;//判定のRect
    float speed = 0;
    //配列初期化可能にする為に空のコンストラクタをおく
    Object() {}
    Object(Point initPoint, float initSpeed,Size v_rect,float rad,float duration,const std::vector<SDL_Texture*>& textures)
        : Animation(duration,textures), Image(), col_radius(rad), speed(initSpeed) {
        point = initPoint;
        rect = { static_cast<int>(initPoint.x), static_cast<int>(initPoint.y), v_rect.w, v_rect.h };
    }
    void initObject(Point initP,float initSpeed) {
        point = initP;
        rect = { static_cast<int>(initP.x), static_cast<int>(initP.y), rect.w, rect.h };
    }
    void Move(float deltaX,float deltaY) {
        point.x += speed * deltaX;
        point.y += speed * deltaY;
        rect.x = (int)point.x;
        rect.y = (int)point.y;
    }
    bool CheckCollision(Object& collision) {
        Point midPoint;
        midPoint.x = collision.point.x - point.x;
        midPoint.y = collision.point.y - point.y;
        float distanceSq = (midPoint.x * midPoint.x) + (midPoint.y * midPoint.y);
        float collisionRadSq = (col_radius + collision.col_radius) * (col_radius + collision.col_radius);
        return distanceSq <= collisionRadSq;
    }
};

/// <summary>
/// プレイヤー　
/// ・移動　
/// ・アイテム獲得時に速度上昇
/// ・フェーズ変化時にスタミナ減少量変化
/// </summary>
class Player : public Object {
    Player(Point initPoint, float initSpeed, Size v_rect,float rad, float duration, const std::vector<SDL_Texture*>& textures)
        : Object(initPoint, initSpeed, v_rect, rad, duration, textures) {
    }

};

/// <summary>
/// エネミー
/// ・プレイヤーに脅威となる存在
/// ・画面の外側四方から　直線移動 / 曲線移動
/// 　＊フェーズ終了間近に真下からくる
/// ・フェーズ変化時に速度上昇
/// </summary>
class Enemy : public Object{
private:
    std::array<Object, MAX_ENEMY> objectPool;
    uINT activeObjectCount = 0;
public:
    Enemy(Point initPoint, float initSpeed, Size v_rect,float rad, float duration, const std::vector<SDL_Texture*>& textures)
        : Object(initPoint, initSpeed, v_rect, rad,duration,textures){
        for (int i = 0; i < MAX_ENEMY; i++) {
            objectPool[i] = Object(initPoint, initSpeed, v_rect, rad, duration, textures);
        }
    }
    bool UseObject(Point initPoint, float initSpeed) {
        if (activeObjectCount < MAX_ENEMY) {
            objectPool[activeObjectCount].initObject(initPoint,initSpeed);
            activeObjectCount++;
            return true;
        }
        return false;
    }


};

/// <summary>
/// エビ
/// ・速度上昇アイテム
/// ・直線移動のみ
/// ・フェーズ変化時にスタミナ回復量増加
/// </summary>
class Ebi : public Object {
private:
    std::array<Object, MAX_EBI> objectPool;
    Ebi(Point initPoint, float initSpeed, Size v_rect, float rad, float duration, const std::vector<SDL_Texture*>& textures)
        : Object(initPoint, initSpeed, v_rect, rad, duration, textures) {
        for (int i = 0; i < MAX_ENEMY; i++) {
            objectPool[i] = Object(initPoint, initSpeed, v_rect, rad, duration, textures);
        }
    }


};

//オブジェクトを管理するクラス
class ObjectManager {
private:
    Player _player;
    Ebi _ebi;
    Enemy _enemy;

};

int getRandomX() {
    return rand() % WINDOW_SIZE_X - 100;
}

/*
void initPlatforms() {
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        platforms[i].x = (int)getRandomX();
        platforms[i].y = -(100 + (i * 120));
        platforms[i].rect = { (int)platforms[i].x , (int)platforms[i].y , 100, 100 };
    }
}

void initEbis() {
    for (int i = 0; i < MAX_EBI; i++) {
        ebis[i].x = (int)getRandomX();
        ebis[i].y = -(100 + (i * 600));
        ebis[i].rect = { (int)ebis[i].x ,(int)ebis[i].y ,50,50 };
        ebis[i].speed = 3000.0f;
    }
}

void initEnemies() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        enemies[i].x = (int)getRandomX();
        enemies[i].y = -(100 + (i * 600));
        enemies[i].rect = { (int)enemies[i].x ,(int)enemies[i].y ,50,50 };
        enemies[i].speed = 6000.0f;
    }
}

void drawPlatforms(SDL_Renderer* renderer, SDL_Texture* texture) {
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        SDL_RenderCopy(renderer, texture, NULL, &platforms[i].rect);
    }
}
*/
int drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color textColor, SDL_Texture** texture, SDL_Rect* textRect) {    
    if (*texture != nullptr) {
        SDL_DestroyTexture(*texture);
        *texture = nullptr;
    }
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (textSurface == NULL) {
        return 0;
    }
    *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (*texture == NULL) {
        SDL_FreeSurface(textSurface);
        return 0;
    }
    textRect->w = textSurface->w;
    textRect->h = textSurface->h;
    SDL_FreeSurface(textSurface);
    return 1;
}

typedef struct {
    SDL_Texture* characterTexture;
    SDL_Texture* cloudTexture;
    SDL_Texture* enemyTexture;
    SDL_Texture* ebiTexture;
    SDL_Texture* textTexture;
    SDL_Texture* gaugeBack;
    SDL_Texture* gaugeFront;
} Textures;

void Release(SDL_Renderer* _rend, SDL_Window* _wind, TTF_Font* _font, Textures* _tex) {
    if (_tex->characterTexture != nullptr) {
        SDL_DestroyTexture(_tex->characterTexture);
        _tex->characterTexture = nullptr;
    }
    if (_tex->cloudTexture != nullptr) {
        SDL_DestroyTexture(_tex->cloudTexture);
        _tex->cloudTexture = nullptr;
    }
    if (_tex->enemyTexture != nullptr) {
        SDL_DestroyTexture(_tex->enemyTexture);
        _tex->enemyTexture = nullptr;
    }
    if (_tex->ebiTexture != nullptr) {
        SDL_DestroyTexture(_tex->ebiTexture);
        _tex->gaugeFront = nullptr;
    }
    if (_font != nullptr) {
        TTF_CloseFont(_font);
    }
    if (_rend != nullptr) {
        SDL_DestroyRenderer(_rend);
    }
    if (_wind != nullptr) {
        SDL_DestroyWindow(_wind);
    }
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void loadTexture(SDL_Renderer* renderer, const char* filePath, SDL_Texture** texture);

/*
Charactor initPlayer() {
    Charactor p;
    p.x = 400.0f;
    p.y = 300.0f;
    p.rect = {(int)p.x,(int)p.y,50,50};
    p.speed = 1000.0f;
    return p;
}

void updateCharaRect(float x, float y,Charactor* _c) {
    _c->rect = { (int)_c->x, (int)_c->y, _c->rect.w, _c->rect.h };
}*/

typedef struct {
    float speed;
    float time;
    int score;
    int waveCount;
    float stamina;
} GameData;

typedef struct {
    float MAXGAUGE;
    float gauge;
    float recover_speed;
}Gauge;

int main2(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Simple Action Game",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_SIZE_X, WINDOW_SIZE_Y, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glEnable(GL_TEXTURE_2D);

    /*--------------------------------------"Load Texture"-------------------------------------*/
    
    Textures _tex = { nullptr, nullptr, nullptr, nullptr, nullptr };

    TTF_Font* font = TTF_OpenFont("Fonts/DotGothic16-Regular.ttf", 28);
    if (font == NULL) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        Release(renderer, window, font, &_tex);
        return -1;
    }

    loadTexture(renderer, "Images/chara1.png", &_tex.characterTexture);
    if (!_tex.characterTexture) {
        printf("Failed to load characterTexture!\n");
        Release(renderer, window, font, &_tex);
        return -1;
    }

    loadTexture(renderer, "Images/cloud.png", &_tex.cloudTexture);
    if (!_tex.cloudTexture) {
        printf("Failed to load cloudTexture!\n");
        Release(renderer, window, font, &_tex);
        return -1;
    }

    loadTexture(renderer, "Images/enemy.png", &_tex.enemyTexture);
    if (!_tex.enemyTexture) {
        printf("Failed to load enemyTexture!\n");
        Release(renderer, window, font, &_tex);
        return -1;
    }

    loadTexture(renderer, "Images/ebi.png", &_tex.ebiTexture);
    if (!_tex.ebiTexture) {
        printf("Failed to load ebiTexture!\n");
        Release(renderer, window, font, &_tex);
        return -1;
    }

    loadTexture(renderer, "Images/gaugeBack.png", &_tex.gaugeBack);
    if (!_tex.gaugeBack) {
        printf("Failed to load enemyTexture!\n");
        Release(renderer, window, font, &_tex);
        return -1;
    }

    loadTexture(renderer, "Images/gaugeFront.png", &_tex.gaugeFront);
    if (!_tex.gaugeFront) {
        printf("Failed to load ebiTexture!\n");
        Release(renderer, window, font, &_tex);
        return -1;
    }

    SDL_Rect gFRect = { 510, -82, 350, 220 };
    SDL_Rect gBRect = { 510, -82, 350, 220 };

    /*--------------------------------------"Load Texture"-------------------------------------*/
    /*--------------------------------------"Texts Renderer"-------------------------------------*/

    SDL_Color textColor = { 255, 255, 255, 255 };

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Hello, SDL_ttf!", textColor);
    if (textSurface == nullptr) {
        printf("Unable to render text surface! TTF_Error: %s\n", TTF_GetError());
        Release(renderer, window, font, &_tex);
        return -1;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (textTexture == nullptr) {
        printf("Unable to create texture from rendered text! SDL_Error: %s\n", SDL_GetError());
        Release(renderer, window, font, &_tex);
        return -1;
    }

    SDL_Rect textRect = { 10, 3, textSurface->w, textSurface->h };

    /*--------------------------------------"Renderer Texts"-------------------------------------*/

    char displayText[100];
    Uint32 startTime = SDL_GetTicks();
    
    /*
    Charactor _p = initPlayer();
    Gauge gauge = { gBRect.w, gBRect.w, 1000.0f };
    initPlatforms();
    initEbis();
    initEnemies();*/

    bool isRunning = true;
    SDL_Event event;

    const float deltaTime = 1.0f / 60.0f;
    float accumulator = 0.0f;

    Uint32 currentTime = SDL_GetTicks();
    Uint32 previousTime = currentTime;

    const int screenWidth = WINDOW_SIZE_X;
    const int screenHeight = WINDOW_SIZE_Y;

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

        /*

        while (accumulator >= deltaTime) {
            const Uint8* keystates = SDL_GetKeyboardState(NULL);
            float moveValue = 0.0f;
            if (keystates[SDL_SCANCODE_UP]) {
                moveValue = _p.speed * frameTime;
                _p.y -= moveValue;
            }
            if (keystates[SDL_SCANCODE_DOWN]) {
                moveValue = _p.speed * frameTime;
                _p.y += moveValue;
            }
            if (keystates[SDL_SCANCODE_LEFT]) {
                moveValue = _p.speed * frameTime;
                _p.x -= moveValue;
            }
            if (keystates[SDL_SCANCODE_RIGHT]) {
                moveValue = _p.speed * frameTime;
                _p.x += moveValue;
            }

            if (_p.x < 0) {
                _p.x = 0;
            }
            if (_p.x + _p.rect.w > screenWidth) {
                _p.x = screenWidth - _p.rect.w;
            }
            if (_p.y < 0) {
                _p.y = 0;
            }
            if (_p.y + _p.rect.h > screenHeight) {
                _p.y = screenHeight - _p.rect.h;
            }

            for (int i = 0; i < PLATFORM_COUNT; i++) {
                platforms[i].y += 1000.0f * frameTime;
                platforms[i].rect.y = (int)platforms[i].y;

                if (platforms[i].rect.y > screenHeight) {
                    platforms[i].x = getRandomX();
                    platforms[i].y = -200;
                    platforms[i].rect.x = (int)platforms[i].x;
                    platforms[i].rect.y = (int)platforms[i].y;
                }
            }

            for (int i = 0; i < MAX_EBI; i++) {
                ebis[i].y += ebis[i].speed * frameTime;
                ebis[i].rect.y = (int)ebis[i].y;
                if (ebis[i].rect.y > screenHeight) {
                    ebis[i].x = getRandomX();
                    ebis[i].y = -200;
                    updateCharaRect(getRandomX(),-200,&ebis[i]);
                }
            }

            for (int i = 0; i < MAX_ENEMY; i++) {
                enemies[i].y += enemies[i].speed * frameTime;
                enemies[i].rect.y = (int)enemies[i].y;
                if (enemies[i].rect.y > screenHeight) {
                    enemies[i].x = getRandomX();
                    enemies[i].y = -200;
                    updateCharaRect(getRandomX(), -200, &enemies[i]);
                }
            }

            accumulator -= deltaTime;

            gauge.gauge -= moveValue * 2;

            if (gauge.gauge < 0) {
                gauge.gauge = 0;
            }else if (gauge.MAXGAUGE > gauge.gauge) {
                gauge.gauge +=  gauge.recover_speed  * frameTime;
                if (gauge.gauge > gauge.MAXGAUGE) {
                    gauge.gauge = gauge.MAXGAUGE;
                }
            }
            
        }
        */

        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 86, 157, 225, 255);  // 背景

        /*

        drawPlatforms(renderer, _tex.cloudTexture);  // 雲

        for (int i = 0; i < MAX_EBI; i++) {
            SDL_RenderCopy(renderer, _tex.ebiTexture, NULL, &ebis[i].rect);  // エビ
        }

        for (int i = 0; i < MAX_ENEMY; i++) {
            SDL_RenderCopy(renderer, _tex.enemyTexture, NULL, &enemies[i].rect);  // エビ
        }

        updateCharaRect(_p.x,_p.y,&_p);
        */

        //SDL_RenderCopy(renderer, _tex.characterTexture, NULL, &_p.rect);  // キャラ

        snprintf(displayText, sizeof(displayText), "Time: %u", Uint32((SDL_GetTicks() - startTime) / 1000.0f));
        drawText(renderer, font, displayText, textColor, &textTexture, &textRect);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);  // テキスト表示


        SDL_RenderCopy(renderer, _tex.gaugeBack, NULL, &gBRect);  // ゲージ後ろ
        //gFRect.w = gauge.gauge;
        SDL_RenderCopy(renderer, _tex.gaugeFront, NULL, &gFRect);  // ゲージ前     

        SDL_RenderPresent(renderer);
    }

    /*--------------------------------------"Game Loop"-------------------------------------*/

    Release(renderer, window, font, &_tex);

    return 0;
}

void loadTexture(SDL_Renderer* renderer, const char* filePath, SDL_Texture** texture) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        printf("Failed to load image: %s, %s\n", filePath, IMG_GetError());
        return;
    }
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!*texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
    }
}
