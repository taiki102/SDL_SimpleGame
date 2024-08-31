#pragma once

#include "ResourceManager.h"

#define GAMENAME "カモメマックスフライト"

#define MAX_ENEMY 10
#define MAX_EBI 5
#define MAX_CLOUD 6

/// <summary>
/// 
/// ＊プレイヤー＊　
/// ・移動　
/// ・アイテム獲得時に速度上昇
/// ・フェーズ変化時にスタミナ減少量変化
/// 
/// </summary>
class Player : public RadCollisionObject {
private:

public:
    Player() : RadCollisionObject(){}
    Player(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : RadCollisionObject(initPoint, initSpeed, rad, size, texture, animFrame, duration) {
    }
    bool CheckRectCollision(SDL_Rect* rect) const {
        bool insideX = point.x >= rect->x && point.x <= rect->x + rect->w;
        bool insideY = point.y >= rect->y && point.y <= rect->y + rect->h;
        return insideX && insideY;
    }
    bool CheckCircleCollision(Point p,float rad) {
        Point midPoint;
        midPoint.x = p.x - point.x;
        midPoint.y = p.y - point.y;
        float distanceSq = (midPoint.x * midPoint.x) + (midPoint.y * midPoint.y);
        float collisionRadSq = (col_radius + rad) * (col_radius + rad);
        return distanceSq <= collisionRadSq;
    }
};

/// <summary>
/// 
/// エネミー
/// ・プレイヤーに脅威となる存在
/// ・画面の外側四方から　直線移動 / 曲線移動
/// 　＊フェーズ終了間近に真下からくる
/// ・フェーズ変化時に速度上昇
/// 
/// </summary>
class Enemy : public RadCollisionObject_ex {
private:
    std::array<RadCollisionObject_ex, MAX_ENEMY> objectPool;
    uINT activeObjectCount = 0;
public:
    Enemy() : RadCollisionObject_ex() {}
    Enemy(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture)
        : RadCollisionObject_ex(initPoint, initSpeed, rad, size, texture){
        for (auto& obj : objectPool) {
            obj = RadCollisionObject_ex(initPoint, initSpeed, rad, size, texture);
        }
    }
    bool UseObject(Point initPoint, float initSpeed, float angle) {
        if (activeObjectCount < MAX_ENEMY) {
            objectPool[activeObjectCount].InitObject(initPoint, initSpeed, angle);
            activeObjectCount++;
            return true;
        }
        return false;
    }
};

/// <summary>
/// 
/// エビ
/// ・速度上昇アイテム
/// ・直線移動のみ
/// ・フェーズ変化時にスタミナ回復量増加
/// 
/// </summary>
class Ebi : public RadCollisionObject {
private:
    std::array<RadCollisionObject, MAX_EBI> objectPool;
    uINT activeObjectCount = 0;
public:
    Ebi() : RadCollisionObject() {}
    Ebi(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : RadCollisionObject(initPoint, initSpeed, rad, size, texture, animFrame, duration) {
        for (auto& obj : objectPool) {
            obj = RadCollisionObject(initPoint, initSpeed, rad, size, texture, animFrame, duration);
        }
    }
    bool UseObject(Point initPoint, float initSpeed, float angle) {
        if (activeObjectCount < MAX_EBI) {
            objectPool[activeObjectCount].InitObject(initPoint, initSpeed, angle);
            activeObjectCount++;
            return true;
        }
        return false;
    }
};

/// <summary>
/// 
/// 雲
/// ・通過するプレイヤーの速度をスローにする
/// 
/// </summary>
class Cloud : public RectCollisionObject {
private:
    std::array<RectCollisionObject, MAX_CLOUD> objectPool;
    uINT activeObjectCount = 0;
public:
    Cloud() : RectCollisionObject(){}
    Cloud(Point initPoint, float initSpeed, Size size, SDL_Texture* texture)
        : RectCollisionObject(initPoint, initSpeed, size, texture) {
        for (auto& obj : objectPool) {
            obj = RectCollisionObject(initPoint, initSpeed, size, texture);
        }
    }
    bool UseObject(Point initPoint, float initSpeed, float angle) {
        if (activeObjectCount < MAX_CLOUD) {
            objectPool[activeObjectCount].InitObject(initPoint, initSpeed, angle);
            activeObjectCount++;
            return true;
        }
        return false;
    }
};

class Gauge {
private:
    float recover_speed;  // 回復速度
    float degree_speed;  //減少値

    float gauge;  // 現在のゲージの値
    float MAXGAUGE;  // 最大ゲージの値
    SDL_Texture* gaugeBack;   // ゲージの背景テクスチャ
    SDL_Texture* gaugeFront;  // ゲージの前景テクスチャ
    SDL_Rect gBRect;  // 背景テクスチャの描画領域
    SDL_Rect gFRect;  // 前景テクスチャの描画領域

public:
    Gauge(float recoverSpeed, float degreeSpeed, SDL_Texture* backTex, SDL_Texture* frontTex, SDL_Rect bRect, SDL_Rect fRect)
        : gauge(bRect.w), MAXGAUGE(bRect.w), recover_speed(recoverSpeed),degree_speed(degreeSpeed), gaugeBack(backTex), gaugeFront(frontTex), gBRect(bRect), gFRect(fRect) {
        gFRect.w = bRect.w; // ゲージの幅を設定
    }
    void Update_Gauge(float moveValue, float frameTime) {
        gauge -= moveValue + degree_speed * frameTime;
        if (gauge < 0) {
            gauge = 0;
        }
        else if (gauge < MAXGAUGE) {
            gauge += recover_speed * frameTime;
            if (gauge > MAXGAUGE) {
                gauge = MAXGAUGE;
            }
        }
        gFRect.w = static_cast<int>(gauge);
    }
    void Render_Gauge(SDL_Renderer* renderer) {
        SDL_RenderCopy(renderer, gaugeBack, NULL, &gBRect);  // ゲージ後
        SDL_RenderCopy(renderer, gaugeFront, NULL, &gFRect); // ゲージ前
    }
};

class Title {
private:
    SDL_Texture* textTexture = nullptr;  // テキストのテクスチャ
    SDL_Rect textRect;                   // テキストの描画領域
    SDL_Point screenSize;                // 画面サイズ
    int targetY;                         // 目標Y座標（中央）
    int speed = 2;                       // 移動速度
    bool isMoving = true;                // テキストが移動中かどうか
    bool isVisible = true;               // テキストが表示されているかどうか

public:
    Title(const std::string& text, TTF_Font* font, SDL_Color color, SDL_Point screenSize, SDL_Renderer* renderer)
        : screenSize(screenSize) {

        textTexture = ResourceManager::LoadFontTexture();

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);

        // テキストのサイズを取得し、初期位置を設定する
        int textWidth, textHeight;
        SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);
        textRect = { (screenSize.x - textWidth) / 2, screenSize.y, textWidth, textHeight };
        targetY = (screenSize.y - textHeight) / 2;
    }

    void update() {
        if (isMoving) {
            if (textRect.y > targetY) {
                textRect.y -= speed;
            }
            else {
                textRect.y = targetY;
                isMoving = false;
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        if (isVisible) {
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        }
    }

    void handleEvent(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) { // Enterキーが押されたとき
            isVisible = false;
        }
    }

    bool isTextVisible() const {
        return isVisible;
    }
};

enum GameScene
{
    Title,
    InGame,
    Result
};

//ゲームを管理するクラス
class GameManager {
private:
    float stamina = 0;  // スタミナ値
    float timer = 0;    // タイマー
    float score = 0;    //スコア

    GameScene currentScene = GameScene::Title;

    Player _player;
    Ebi _ebi;
    Enemy _enemy;
    Cloud _cloud;
public:
    GameManager(SDL_Texture* pTex, SDL_Texture* ebiTex, SDL_Texture* enemyTex, SDL_Texture* cloudTex) {
        Point p = Point(-100.0f,-100.0f);
        Size s = Size(10,10);
        _player = Player(p, 100.0f, 1.0f, s, pTex, 4, 1.0f);  // radとanim を持つ
        _ebi = Ebi(p, 100.0f, 1.0f, s, ebiTex, 4, 1.0f);  // radとanim
        _enemy = Enemy(p, 100.0f, 1.0f, s, enemyTex);  // rad
        _cloud = Cloud(p, 100.0f, s, cloudTex);  // none
    }
    void Update(float deltaTime) {
        switch (currentScene) {
        case GameScene::Title:
            titleScreen.update();
            titleScreen.render(renderer);

            // 例えば、Enterキーが押されたらメインゲームに切り替える
            if (!titleScreen.isTextVisible()) {
                currentScene = GameScene::MainGame;
            }
            break;
        case GameScene::InGame:
            break;
        default:
            break;
        }
    }
    void Render() {
        
    }
};