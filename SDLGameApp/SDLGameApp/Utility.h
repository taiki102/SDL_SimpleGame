#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>

//カラーテンプレ
#define COLOR_BLACK   SDL_Color{ 0, 0, 0, 255 }
#define COLOR_WHITE   SDL_Color{ 255, 255, 255, 255 }
#define COLOR_BLUE     SDL_Color{ 255, 0, 0, 255 }

constexpr float DEG_TO_RAD = M_PI / 180.0f;
constexpr int IMAGESIZE = 32;
constexpr float DEF_VECTOR = 90.0f;
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
    Point(int x,int y) : x(x),y(y){}
};

//画像
struct Image{
    SDL_Rect rect = {0,0,0,0};
    SDL_Texture* texture = nullptr;
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
    bool DoAnim = true;
public:
    Animation(uINT maxframe,float duration) 
        : animDuration(duration),MaxFrame(maxframe) {
        CurrentFrameIndex = 0;
        frameCount = 0;
    }
    SDL_Rect* GetsrcRect(float deltaTime) {
        srcrect.x = CurrentFrameIndex * IMAGESIZE;
        if (DoAnim) {
            frameCount += deltaTime;
            if (frameCount > animDuration) {
                frameCount = 0;
                CurrentFrameIndex++;
                if (CurrentFrameIndex >= MaxFrame) {
                    CurrentFrameIndex = 0;
                }
            }
        }
        return &srcrect;
    }
    //切り替えが一度の処理で済むように
    void SetOffAnim() {
        DoAnim = false;
        CurrentFrameIndex = 0;
        frameCount = 0;
    }
    void SetOnAnim() {
        DoAnim = true;
    }
};

//ベクトル　回転の角度を保持する
struct Vector {
    float x = 0, y = 0;
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
        rect.x = (int)point.x;
        rect.y = (int)point.y;
    }
};

//矩形の当たり判定を持つオブジェクト
struct RectCollisionObject : public Object{
    RectCollisionObject() : Object(Point(), 0, 0, Size(), nullptr) {}
    RectCollisionObject(Point initPoint, float initSpeed, Size size, SDL_Texture* texture)
        : Object(initPoint, initSpeed, DEF_VECTOR, size, texture) {}
    SDL_Rect GetRect() {
        return rect;
    }
};

//円の当たり判定を持つオブジェクト
struct RadCollisionObject : public Object, public Animation{
    float col_radius = 0;
    RadCollisionObject() : Object(Point(), 0, 0, Size(), nullptr),Animation(0,0) {}
    RadCollisionObject(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture, int animFrame, float duration)
        : Object(initPoint, initSpeed, DEF_VECTOR, size, texture),
        Animation(animFrame, duration), col_radius(rad) {}
    float GetCollisionRadius() {
        return col_radius;
    }
};

//円の当たり判定を持つオブジェクト かつ アニメーションを持たない
struct RadCollisionObject_ex : public Object{
    float col_radius = 0;
    RadCollisionObject_ex() : Object(Point(), 0, 0, Size(), nullptr) {}
    RadCollisionObject_ex(Point initPoint, float initSpeed, float rad, Size size, SDL_Texture* texture)
        : Object(initPoint, initSpeed, DEF_VECTOR, size, texture),
        col_radius(rad) {}
    float GetCollisionRadius() {
        return col_radius;
    }
};

/*
struct Object {
private:

    SDL_Rect rect = { 0,0,0,0 };  //見た目rect
    SDL_Texture* texture = nullptr;  //テクスチャ
    float col_radius = 0;//判定の
    float speed = 0;

    Object(Point initPoint, float initSpeed, Size size, float rad, float duration,int animFrame, SDL_Texture* texture)
        : Animation(animFrame,duration), col_radius(rad), speed(initSpeed), point(initPoint) {
        this->texture = texture;
        rect = { static_cast<int>(initPoint.x), static_cast<int>(initPoint.y), size.width, size.height };
    }
    void initObject(Point initP, float initSpeed) {
        point = initP;
        rect = { static_cast<int>(initP.x), static_cast<int>(initP.y), rect.w, rect.h };
    }
    void Move(float deltaX, float deltaY) {
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

    void Rend(SDL_Renderer* renderer) {
        SDL_RenderCopyEx(
            renderer, 
            texture, 
            &srcRect, 
            &destRect, 
            angle, 
            &center, 
            SDL_FLIP_NONE);
    }
};*/