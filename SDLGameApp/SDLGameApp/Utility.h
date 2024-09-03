#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <cmath>

//カラーテンプレ
#define COLOR_BLACK     SDL_Color{ 0, 0, 0, 255 }
#define COLOR_WHITE     SDL_Color{ 255, 255, 255, 255 }
#define COLOR_BLUE      SDL_Color{ 0, 0, 255, 255 }
#define COLOR_YELLOW    SDL_Color{ 255,255,0,0}
#define COLOR_LIGHTBLUE SDL_Color{ 86, 157, 225, 255 }

#define WINDOW_SIZE_X 800
#define WINDOW_SIZE_Y 600

constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
constexpr float DEFAULT_VECTOR = 90.0f;
constexpr int IMAGESIZE = 32;
using uINT = uint8_t;

//縦と横の大きさ
struct Size {
    int width;
    int height;

    Size() : width(0), height(0) {}
    Size(int w, int h) : width(w), height(h) {}
};

//細かい変化量を保つ為にfloatで保持
struct Point {
    float x;
    float y;

    Point() : x(0.0f),y(0.0f) {}
    Point(float x, float y) : x(x),y(y){}
};

//画像
struct Image{
    SDL_Rect rect;
    SDL_Texture* texture;
    Image() : rect({ 0,0,0,0 }),texture(nullptr) {}
    Image(SDL_Rect rect, SDL_Texture* texture) : rect(rect), texture(texture) {}
};

//スプライトシートを用いて描画する為のクラス
//Imageの上層に置いて使用
class Animation {
private:
    SDL_Rect srcrect = {0,0,IMAGESIZE,IMAGESIZE };//テクスチャ切り取り位置
    uINT MaxFrame = 0;
    uINT CurrentFrameIndex = 0;
    float frameCount = 0.0f;
    float animDuration = 0.0f;
    bool isAnim = true;
    bool isBlink = true;
    bool flg_blink = false;
    bool flg_onceAnim = false;
public:
    Animation(uINT maxframe,float duration) 
        : animDuration(duration),MaxFrame(maxframe) {
        CurrentFrameIndex = 0;
        frameCount = 0;
    }
    SDL_Rect* GetSrcRect() {
        return &srcrect;
    }
    void UpdateFrameCheck(float deltaTime) {
        srcrect.x = CurrentFrameIndex * IMAGESIZE;
        if (isAnim) {
            frameCount += deltaTime;
            if (frameCount > animDuration) {
                frameCount = 0;
                CurrentFrameIndex++;
                if (CurrentFrameIndex >= MaxFrame) {
                    CurrentFrameIndex = 0;

                    if (flg_onceAnim) {
                        flg_onceAnim = false;
                        ToggleAnim(false);
                    }
                }
                if (isBlink) {
                    flg_blink = !flg_blink;
                }
            }
        }
    }
    void ToggleAnim(bool Is) {
        isAnim = Is;
        CurrentFrameIndex = 0;
        frameCount = 0;
    }
    void ToggleBlink(bool Is) {
        isBlink = Is;
        flg_blink = Is;  //点滅オンでfalse　点滅オフでtrueを返す
        CurrentFrameIndex = 0;
        frameCount = 0;
    }
    void TriggerOnceAnim() {
        if (flg_onceAnim) {
            return;
        }
        ToggleAnim(true);
        flg_onceAnim = true;
    }
    bool GetBlink() {
        return flg_blink;
    }
    void UpdateBlinkCheck(float deltaTime) {
        if (isBlink) {
            frameCount += deltaTime;
            if (frameCount > animDuration) {
                frameCount = 0;
                flg_blink = !flg_blink;
            }
        }
    }
};

//ベクトル　回転の角度を保持する
struct Vector {
    float x = 0.0f, y = 0.0f;
    float angle = 0.0f;
    float magnitude = 0.0f;
    bool isMagnitudeDirty = true;

    float length() {
        if (isMagnitudeDirty) {
            magnitude = std::sqrt(x * x + y * y);
            isMagnitudeDirty = false;
        }
        return magnitude;
    }

    void normalize() {
        float len = length();
        if (len > 0) {
            x /= len;
            y /= len;
            magnitude = 1.0f;
            isMagnitudeDirty = false;
        }
    }

    void fromAngle(float angleInDegrees) {
        angle = angleInDegrees;
        float angleInRadians = angleInDegrees * DEG_TO_RAD;
        x = std::cos(angleInRadians);
        y = std::sin(angleInRadians);
        isMagnitudeDirty = true;
    }

    void set(float newX, float newY) {
        x = newX;
        y = newY;
        isMagnitudeDirty = true;
    }

    void setLength(float newLength) {
        if (newLength == 0) {
            x = 0;
            y = 0;
            magnitude = 0;
            isMagnitudeDirty = false;
        }
        else {
            normalize();
            x *= newLength;
            y *= newLength;
            magnitude = newLength;
            isMagnitudeDirty = false;
        }
    }

    void rotate(float angleDelta) {
        float angleInRadians = angleDelta * DEG_TO_RAD;
        float cosTheta = std::cos(angleInRadians);
        float sinTheta = std::sin(angleInRadians);

        float newX = x * cosTheta - y * sinTheta;
        float newY = x * sinTheta + y * cosTheta;

        x = newX;
        y = newY;
        angle += angleDelta;
        isMagnitudeDirty = true;
    }
};

//ベクトルとスピードをもつものをObjectと定義する
struct Object : public Image {
    Point point;
    float speed = 0;
    Vector vect;

    Object(Point initPoint, float initSpeed,float v_angle,Size size,SDL_Texture* texture) : point(initPoint),speed(initSpeed){
        rect = { static_cast<int>(initPoint.x), static_cast<int>(initPoint.y), size.width, size.height };
        this->texture = texture;
        vect.fromAngle(v_angle);
    }
    void InitObject(Point initPoint, float initSpeed, float angle) {
        point = initPoint;
        speed = initSpeed;
        vect.fromAngle(angle);
    }
    void Move(float deltaX, float deltaY) {
        point.x += speed * deltaX;
        point.y += speed * deltaY;
    }
    void MoveVector(float delta) {
        point.x += speed * vect.x * delta;
        point.y += speed * vect.y * delta;
    }
    SDL_Rect* GetUpdateRect() {
        rect.x = (int)point.x;
        rect.y = (int)point.y;
        return &rect;
    }

    Point GetMidPoint() const {
        return Point(rect.x + rect.w / 2, rect.y + rect.h / 2);
    }
};

//矩形と円の当たり判定を持つオブジェクト
/*
struct CollisionObject : public Object, public Animation {
    SDL_Rect col_rect = { 0,0,0,0 };
    float col_radius = 0;
    CollisionObject() : Object(Point(), 0, 0, Size(), nullptr), Animation(0, 0) {}
    CollisionObject(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : Object(initPoint, initSpeed, DEFAULT_VECTOR, size, texture),
        Animation(animFrame, duration), col_radius(rad) {}
};
*/

//矩形の当たり判定を持つオブジェクト
struct RectCollisionObject : public Object{
    RectCollisionObject() : Object(Point(), 0, 0, Size(), nullptr) {}
    RectCollisionObject(Point initPoint, float initSpeed, Size size, SDL_Texture* texture)
        : Object(initPoint, initSpeed, DEFAULT_VECTOR, size, texture) {}
    bool CheckRectCollision(const Point& point) const {
        bool insideX = point.x >= rect.x && point.x <= rect.x + rect.w;
        bool insideY = point.y >= rect.y && point.y <= rect.y + rect.h;
        return insideX && insideY;
    }
};

//円の当たり判定を持つオブジェクト
struct RadCollisionObject : public Object, public Animation{
    float col_radius = 0;
    RadCollisionObject() : Object(Point(), 0, 0, Size(), nullptr),Animation(0,0) {}
    RadCollisionObject(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : Object(initPoint, initSpeed, DEFAULT_VECTOR, size, texture),
        Animation(animFrame, duration), col_radius(rad) {}
    float GetCollisionRadius() {
        return col_radius;
    }
    bool CheckCircleCollision(Point midp, float rad) {
        Point mp = GetMidPoint();
        float dx = midp.x - mp.x;
        float dy = midp.y - mp.y;
        float distanceSq = (dx * dx) + (dy * dy);
        float collisionRadSq = (col_radius + rad) * (col_radius + rad);
        return distanceSq <= collisionRadSq;
    }
};

//円の当たり判定を持つオブジェクト かつ アニメーションを持たない
struct RadCollisionObject_ex : public Object{
    float col_radius = 0;
    RadCollisionObject_ex() : Object(Point(), 0, 0, Size(), nullptr) {}
    RadCollisionObject_ex(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture)
        : Object(initPoint, initSpeed, DEFAULT_VECTOR, size, texture),
        col_radius(rad) {}
    float GetCollisionRadius() {
        return col_radius;
    }
};

//シーン基底クラス
class Scene {
public:
    virtual ~Scene() {}
    virtual void update(float deltaTime) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
    bool isTransitioning() const {
        return IsTransition;
    }
protected:
    bool IsTransition = false;
};

//UI ゲージ
class Gauge : public Animation {
private:
    float recover_speed;  // 回復速度
    float degree_speed;  // 減少速度
    float gauge;  // 現在ゲージ
    float MAXGAUGE;  // 最大ゲージ
    Image* gaugeBack = nullptr; // ゲージのバック画像
    Image* gaugeFront = nullptr; // ゲージのフロント画像

    bool isDotDamage = false;
    bool flg_blink = false;
    float elapsedTime = 0;
    float dotDamage = 0.1f;

    bool end = false;
public:
    Gauge() : recover_speed(0), degree_speed(0), gauge(0), MAXGAUGE(0), Animation(0, 0) {}
    Gauge(float maxGauge, float recoverSpeed, float degreeSpeed, Image* backImage, Image* frontImage)
        : gauge(maxGauge), MAXGAUGE(maxGauge), recover_speed(recoverSpeed), degree_speed(degreeSpeed),
        gaugeBack(backImage), gaugeFront(frontImage), Animation(0, 0.1f) {
        gaugeFront->rect.w = gaugeBack->rect.w;
    }

    void GetStartSpeedDamage() {
        gauge -= MAXGAUGE * 0.02f;
    }

    void GetHeal(uINT x) {
        gauge += MAXGAUGE;//* (0.1f + 0.1f * x)
        if (gauge > MAXGAUGE) {
            gauge = MAXGAUGE;
        }
    }

    void ChangeDotDamage(uINT x) {
        recover_speed = 15.0f * (1.0 + 0.2f * x);
        dotDamage = 0.1f + 0.1f * x;
        if (dotDamage >= 0.6f) {
            dotDamage = 0.6f;
        }
    }

    void GetDotDamageEX() {
        if (isDotDamage) return;
        isDotDamage = true;
        gauge -= MAXGAUGE * dotDamage;
    }

    void GetDotDamage() {
        if (isDotDamage) return;
        isDotDamage = true;
        gauge -= MAXGAUGE * dotDamage;
    }

    bool CheckGaugeZero() {
        return end;
    }

    void UpdateGauge(float delta, bool isMoving) {

        if (gauge < 0.0001f) {
            end = true;
            return;
        }


        if (isDotDamage) {
            elapsedTime += delta;
            float damageInterval = 0.6f;
            if (elapsedTime >= damageInterval) {
                elapsedTime = 0;
                isDotDamage = false;
            }
        }
        else {
            if (isMoving) {
                // 減少処理
                gauge -= degree_speed * delta;
                if (gauge < 0) {
                    gauge = 0;
                }
            }
            else {
                // 回復処理
                if (gauge < MAXGAUGE) {
                    gauge += recover_speed * delta;
                    if (gauge > MAXGAUGE) {
                        gauge = MAXGAUGE;
                    }
                }
            }
        }

        if (!flg_blink && gauge < MAXGAUGE * 0.2f) {
            flg_blink = true;
            ToggleBlink(true);
        }
        else if(flg_blink && gauge > MAXGAUGE * 0.2f){
            flg_blink = false;
            ToggleBlink(false);
        }      
        UpdateBlinkCheck(delta);
    }

    void RenderGauge(SDL_Renderer* renderer) {

        if (!GetBlink()) {
            gaugeFront->rect.w = static_cast<int>((gauge / MAXGAUGE) * gaugeBack->rect.w);
            SDL_RenderCopy(renderer, gaugeBack->texture, nullptr, &gaugeBack->rect);  // ゲージ後
            SDL_RenderCopy(renderer, gaugeFront->texture, nullptr, &gaugeFront->rect); // ゲージ前
        }
    }

    void InitSpeed(float recoverSpeed, float degreeSpeed) {
        recover_speed = recoverSpeed;
        degree_speed = degreeSpeed;
    }

    void InitMaxGauge(float maxGauge) {
        MAXGAUGE = maxGauge;
    }
};

//Y軸移動アニメーション
class MoveAnimationY {
private:
    SDL_Rect* rect = nullptr;
    Point point;
    float targetY;
    float moveSpeed;
    bool IsAnimEnd = false;
public:
    MoveAnimationY() : targetY(0), moveSpeed(0) {}
    MoveAnimationY(SDL_Rect* rect, float targetY, float moveSpeed)
        : rect(rect), targetY(targetY), moveSpeed(moveSpeed),
        point(Point((float)(rect->x), (float)(rect->y))) {}

    void Update(float deltaTime) {
        if (rect->y > targetY) {
            point.y -= moveSpeed * deltaTime;
            rect->y = (int)(point.y);
        }
        else {
            rect->y = (int)targetY;
            IsAnimEnd = true;
        }
    }

    bool GetIsAnimEnd() { return IsAnimEnd; }

    SDL_Rect* GetCurrentRect() { return rect; }
};