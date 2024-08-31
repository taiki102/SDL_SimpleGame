#pragma once

#include "ResourceManager.h"

#define GAMENAME "�J�����}�b�N�X�t���C�g"

#define MAX_ENEMY 10
#define MAX_EBI 5
#define MAX_CLOUD 6

/// <summary>
/// 
/// ���v���C���[���@
/// �E�ړ��@
/// �E�A�C�e���l�����ɑ��x�㏸
/// �E�t�F�[�Y�ω����ɃX�^�~�i�����ʕω�
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
/// �G�l�~�[
/// �E�v���C���[�ɋ��ЂƂȂ鑶��
/// �E��ʂ̊O���l������@�����ړ� / �Ȑ��ړ�
/// �@���t�F�[�Y�I���ԋ߂ɐ^�����炭��
/// �E�t�F�[�Y�ω����ɑ��x�㏸
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
/// �G�r
/// �E���x�㏸�A�C�e��
/// �E�����ړ��̂�
/// �E�t�F�[�Y�ω����ɃX�^�~�i�񕜗ʑ���
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
/// �_
/// �E�ʉ߂���v���C���[�̑��x���X���[�ɂ���
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
    float recover_speed;  // �񕜑��x
    float degree_speed;  //�����l

    float gauge;  // ���݂̃Q�[�W�̒l
    float MAXGAUGE;  // �ő�Q�[�W�̒l
    SDL_Texture* gaugeBack;   // �Q�[�W�̔w�i�e�N�X�`��
    SDL_Texture* gaugeFront;  // �Q�[�W�̑O�i�e�N�X�`��
    SDL_Rect gBRect;  // �w�i�e�N�X�`���̕`��̈�
    SDL_Rect gFRect;  // �O�i�e�N�X�`���̕`��̈�

public:
    Gauge(float recoverSpeed, float degreeSpeed, SDL_Texture* backTex, SDL_Texture* frontTex, SDL_Rect bRect, SDL_Rect fRect)
        : gauge(bRect.w), MAXGAUGE(bRect.w), recover_speed(recoverSpeed),degree_speed(degreeSpeed), gaugeBack(backTex), gaugeFront(frontTex), gBRect(bRect), gFRect(fRect) {
        gFRect.w = bRect.w; // �Q�[�W�̕���ݒ�
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
        SDL_RenderCopy(renderer, gaugeBack, NULL, &gBRect);  // �Q�[�W��
        SDL_RenderCopy(renderer, gaugeFront, NULL, &gFRect); // �Q�[�W�O
    }
};

class Title {
private:
    SDL_Texture* textTexture = nullptr;  // �e�L�X�g�̃e�N�X�`��
    SDL_Rect textRect;                   // �e�L�X�g�̕`��̈�
    SDL_Point screenSize;                // ��ʃT�C�Y
    int targetY;                         // �ڕWY���W�i�����j
    int speed = 2;                       // �ړ����x
    bool isMoving = true;                // �e�L�X�g���ړ������ǂ���
    bool isVisible = true;               // �e�L�X�g���\������Ă��邩�ǂ���

public:
    Title(const std::string& text, TTF_Font* font, SDL_Color color, SDL_Point screenSize, SDL_Renderer* renderer)
        : screenSize(screenSize) {

        textTexture = ResourceManager::LoadFontTexture();

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);

        // �e�L�X�g�̃T�C�Y���擾���A�����ʒu��ݒ肷��
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
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) { // Enter�L�[�������ꂽ�Ƃ�
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

//�Q�[�����Ǘ�����N���X
class GameManager {
private:
    float stamina = 0;  // �X�^�~�i�l
    float timer = 0;    // �^�C�}�[
    float score = 0;    //�X�R�A

    GameScene currentScene = GameScene::Title;

    Player _player;
    Ebi _ebi;
    Enemy _enemy;
    Cloud _cloud;
public:
    GameManager(SDL_Texture* pTex, SDL_Texture* ebiTex, SDL_Texture* enemyTex, SDL_Texture* cloudTex) {
        Point p = Point(-100.0f,-100.0f);
        Size s = Size(10,10);
        _player = Player(p, 100.0f, 1.0f, s, pTex, 4, 1.0f);  // rad��anim ������
        _ebi = Ebi(p, 100.0f, 1.0f, s, ebiTex, 4, 1.0f);  // rad��anim
        _enemy = Enemy(p, 100.0f, 1.0f, s, enemyTex);  // rad
        _cloud = Cloud(p, 100.0f, s, cloudTex);  // none
    }
    void Update(float deltaTime) {
        switch (currentScene) {
        case GameScene::Title:
            titleScreen.update();
            titleScreen.render(renderer);

            // �Ⴆ�΁AEnter�L�[�������ꂽ�烁�C���Q�[���ɐ؂�ւ���
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