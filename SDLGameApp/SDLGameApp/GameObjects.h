#pragma once
#define MAX_ENEMY 10
#define MAX_CLOUD 6

#include "ResourceManager.h"

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
    Player() : RadCollisionObject() {}
    Player(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : RadCollisionObject(initPoint, initSpeed, rad, size, texture, animFrame, duration) {
    }

    void Render(SDL_Renderer* renderer) {

        if (!GetBlink()) {
            SDL_RenderCopy(renderer, texture, GetSrcRect(), GetUpdateRect());
        }
    }

    void NormalRender(SDL_Renderer* renderer) {
        SDL_RenderCopy(renderer, texture, GetSrcRect(), &rect);
    }

    bool IsOutOfScrern() {
        return point.x < 0 || point.x > WINDOW_SIZE_X || point.y < 0 || point.y > WINDOW_SIZE_Y;
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
        : RadCollisionObject_ex(initPoint, initSpeed, rad, size, texture) {
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
    float timer; 
    float spawnInterval;
    bool isVisible = false;
public:
    Ebi() : RadCollisionObject(), timer(0), spawnInterval(0.0f) {}
    Ebi(float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : RadCollisionObject({0,0}, initSpeed, rad, size, texture, animFrame, duration),
          timer(0), spawnInterval(5.0f) {        
        ToggleAnim(true);
    }

    void Update(float deltaTime) {

        if (isVisible) {

            UpdateFrameCheck(deltaTime);
            MoveVector(deltaTime);

            if (point.y > WINDOW_SIZE_Y) {
                isVisible = false;
            }
        }
        else {
            timer += deltaTime;
            if (timer >= spawnInterval) {
                timer = 0;
                point = { (float)(rand() % (WINDOW_SIZE_X - rect.w)) , -((float)rect.h) };
                isVisible = true;
            }
        }
    }

    bool CheckCollision(Point midp, float rad) {
        if (isVisible && CheckCircleCollision(midp, rad)) {
            isVisible = false;
            return true;
        }
        return false;
    }

    void Render(SDL_Renderer* renderer) {
        if (isVisible) {
            SDL_RenderCopy(renderer, texture, GetSrcRect(), GetUpdateRect());
        }
    }

    void SetSpeed(float newSpeed) {
        speed = newSpeed;
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
    Size _Size;

public:
    Cloud() : RectCollisionObject() {}

    Cloud(float initSpeed, Size size, SDL_Texture* _texture)
        : RectCollisionObject({ 0, 0 }, initSpeed, size, _texture), _Size(size) {

        int cloudSpacingX = WINDOW_SIZE_X / MAX_CLOUD;

        for (int i = 0; i < MAX_CLOUD; i++) {
            float x = i * cloudSpacingX + (rand() % (cloudSpacingX / 2) - cloudSpacingX / 4);
            float y = rand() % (WINDOW_SIZE_Y - _Size.height) - WINDOW_SIZE_Y;
            objectPool[i] = RectCollisionObject({ x, y }, initSpeed, _Size, texture);
        }
    }

    void Update(float deltaTime) {
        for (auto& obj : objectPool) {
            obj.MoveVector(deltaTime);

            // 画面外に出たら再配置
            if (obj.point.y > WINDOW_SIZE_Y) {
                obj.point.y = -obj.rect.h;
                obj.point.x = rand() % (WINDOW_SIZE_X - obj.rect.w);
            }
        }
    }

    bool CheckCollision(Point p) {
        for (auto& obj : objectPool) {
            if (obj.CheckRectCollision(p)) {
                return true;
            }
        }
        return false;
    }

    void Render(SDL_Renderer* renderer) {
        for (auto& obj : objectPool) {
            SDL_RenderCopy(renderer, obj.texture, nullptr, obj.GetUpdateRect());
        }
    }

    void SetSpeed(float newSpeed) {
        for (auto& obj : objectPool) {
            obj.speed = newSpeed;  // RectCollisionObject で SetSpeed が必要
        }
    }
};