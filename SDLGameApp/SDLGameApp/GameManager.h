#pragma once

#include "GameObjects.h"

#define GAMENAME "カモメマックスフライト"
#define TEXT_KEY1 "A"
#define TEXT_KEY2 "B"
#define TEXT_KEY3 "C"
#define TEXT_KEY4 "D"
#define TEXT_KEY5 "E"

#define DEFAULT_SPEED 100.0f


struct GameData {
    float timer = 0;    // タイマー
    float score = 0;    // スコア
    uINT ebiCount = 0;
    uINT stageNum = 0;  // ステージ番号

    int disp_timer = 0;
    int disp_score = 0;

    float elapsedTime = 0;

    float basescore = 100;

    void UpdateTime(float deltaTime) {
        timer += deltaTime;
        elapsedTime += deltaTime;
        if (elapsedTime >= 10.0f) {
            elapsedTime = 0;
            score += basescore;
        }
    }

    void UpdateScore(uINT x) {
        basescore = 100 + 0.1f * x;
        score += basescore;
    }

    bool CheckTimerChanged() {
        if (static_cast<int>(timer) != disp_timer) {
            disp_timer = static_cast<int>(timer);
            return true;
        }
        return false;
    }
    bool CheckScoreChanged() {
        if (static_cast<int>(score) != disp_score) {
            disp_score = static_cast<int>(score);
            return true;
        }
        return false;
    }
};


class TitleScene : public Scene {
private:
    Image BigTextImage;
    Image InfoTextImage;
    Animation InfoTextAnim;
    bool isVisible = true;
    uINT FASE = 0;

    MoveAnimationY BigTextAnim;

    SDL_Texture* cloudTex;
    SDL_Rect cloudRect[MAX_CLOUD];

public:
    TitleScene(SDL_Renderer* renderer)
        : InfoTextAnim(0,0.4f){

        Size size1 = Size();
        BigTextImage.texture = ResourceManager::LoadFontTexture(GAMENAME,TEXT_KEY1,COLOR_WHITE, &size1, renderer);
        size1.width *= 2;
        size1.height *= 2;

        BigTextImage.rect = { (int)((WINDOW_SIZE_X - size1.width) / 2),(int)(WINDOW_SIZE_Y * 1.1f),size1.width,size1.height};
        BigTextAnim = MoveAnimationY(&BigTextImage.rect,(float)((WINDOW_SIZE_Y - BigTextImage.rect.h) / 2 - 100), 200.0f);

        Size size2 = Size();
        InfoTextImage.texture = ResourceManager::LoadFontTexture("↑ or ↓ or ← or → を押してゲームスタート", TEXT_KEY2, COLOR_BLACK, &size2, renderer);
        InfoTextImage.rect = { (int)((WINDOW_SIZE_X - size2.width) / 2),(int)((WINDOW_SIZE_Y + size1.height) / 2) + 100,size2.width,size2.height };   
       
        InfoTextAnim.ToggleBlink(true);

        cloudTex = ResourceManager::LoadTexture("Assets/Images/cloudx32.png", renderer);
        int cloudSpacing = WINDOW_SIZE_X / MAX_CLOUD; // 雲同士の間隔を計算
        for (int i = 0; i < MAX_CLOUD; i++) {
            cloudRect[i].x = i * cloudSpacing + (rand() % (cloudSpacing / 2) - cloudSpacing / 4);
            cloudRect[i].y = (rand() % 500);
            cloudRect[i].w = 100;
            cloudRect[i].h = 100;
        }
    }
    void update(float deltaTime) override {

        switch(FASE) {
        case 0:
            BigTextAnim.Update(deltaTime);
            if (BigTextAnim.GetIsAnimEnd()) {
                FASE = 1;
            }
            break;
        case 1:
        {
            InfoTextAnim.UpdateBlinkCheck(deltaTime);

            const Uint8* keystates = SDL_GetKeyboardState(NULL);

            if (keystates[SDL_SCANCODE_UP]) {
                isVisible = false;
            }
            if (keystates[SDL_SCANCODE_DOWN]) {
                isVisible = false;
            }
            if (keystates[SDL_SCANCODE_LEFT]) {
                isVisible = false;
            }
            if (keystates[SDL_SCANCODE_RIGHT]) {
                isVisible = false;
            }

            if (!isVisible) {
                BigTextAnim = MoveAnimationY(&BigTextImage.rect, -100, 200.0f);
                FASE = 2;
            }
        }
            break;
        case 2:
            IsTransition = true;
            FASE = 0;
            /*
            BigTextAnim.Update(deltaTime);
            if (BigTextAnim.GetIsAnimEnd()) {
                FASE = 3;
            }*/
            break;
        case 3:

            //Unused
            IsTransition = true;
            FASE = 0;
            break;
        }
    }
    void render(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 86, 157, 225, 255);

        for (int i = 0; i < MAX_CLOUD; i++) {
            SDL_RenderCopy(renderer,cloudTex, nullptr, &cloudRect[i]);
        }

        if (FASE == 1) {
            if (!InfoTextAnim.GetBlink()) {
                SDL_RenderCopy(renderer, InfoTextImage.texture, nullptr, &InfoTextImage.rect);
            }
        }
        SDL_RenderCopy(renderer, BigTextImage.texture, nullptr, &BigTextImage.rect);
    }
};

class InGameScene : public Scene {
private:

    //GameObjectごとのManager
    Player _player;
    Ebi _ebi;
    Enemy _enemy;
    Cloud _cloud;

    //UI
    GameData* currentData = nullptr;
    char buffer[50];
    Image InfoText1; 
    Image InfoText2;
    Image InfoText3;

    Image gBackImage;
    Image gFrontImage;
    Gauge gauge;

    //調節用
    uINT FASE = 0;
    bool isMoving = false;
    bool latest_isMoving = false;

    float elapsedTime = 0.0f;
    MoveAnimationY charaAnim;
    bool isFirstUpdate = false;
    bool isGetEbi = false;

    float debuff_speed = 1;
    float headwind_scale = 0.3f;

public:
    InGameScene(SDL_Renderer* renderer, GameData* CurrentData) : currentData(CurrentData) {

        /*--------------------------------- プレイヤー初期化 ---------------------------*/
        Size s = Size(50, 50);
        SDL_Texture* pTex = ResourceManager::LoadTexture("Assets/Images/kamome128x32.png",renderer);
        Point p = Point((WINDOW_SIZE_X - 50) / 2, WINDOW_SIZE_Y + 50);
        _player = Player(p, DEFAULT_SPEED, 15.0f, s, pTex, 4, 0.25f);  // rad anim

        charaAnim = MoveAnimationY(&_player.rect, (float)((WINDOW_SIZE_Y - _player.rect.h) / 2 + 100), 200.0f);
        _player.ToggleBlink(false);
        _player.TriggerOnceAnim();

        /*--------------------------------- エビ初期化 ---------------------------*/
        p = Point(-100.0f, -100.0f);
        SDL_Texture* ebiTex = ResourceManager::LoadTexture("Assets/Images/ebi64x32.png", renderer);
        _ebi = Ebi(DEFAULT_SPEED, 15.0f, s, ebiTex, 2, 0.5f);  // rad anim


        /*--------------------------------- エネミー初期化 ---------------------------*/
        SDL_Texture* enemyTex = ResourceManager::LoadTexture("Assets/Images/enemyx32.png", renderer);
        _enemy = Enemy(p, DEFAULT_SPEED, 1.0f, s, enemyTex);  // rad

        /*-------------------------------- クラウド初期化 -------------------------------*/
        s = Size(100, 90);
        SDL_Texture* cloudTex = ResourceManager::LoadTexture("Assets/Images/cloudx32.png", renderer);
        _cloud = Cloud(DEFAULT_SPEED, s, cloudTex);  // none

        Size size1 = Size();
        sprintf_s(buffer, "| Time %d |", currentData->disp_timer);
        InfoText1.texture = ResourceManager::LoadFontTexture(buffer,TEXT_KEY1,COLOR_WHITE, &size1, renderer);
        InfoText1.rect = { 10, 3 ,size1.width,size1.height };

        Size size2 = Size();
        sprintf_s(buffer, "| Score %d |", currentData->disp_score);
        InfoText2.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY2, COLOR_WHITE, &size2, renderer);
        InfoText2.rect = { (WINDOW_SIZE_X - size2.width) / 2 , 3 ,size2.width,size2.height };

        Size size3 = Size();
        sprintf_s(buffer, "| Stage %d |", currentData->stageNum);
        InfoText3.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY3, COLOR_WHITE, &size3, renderer);
        InfoText3.rect = { (WINDOW_SIZE_X - size2.width) - 10 , 3 ,size3.width,size2.height };

        gBackImage.texture = ResourceManager::LoadTexture("Assets/Images/gaugeBack.png", renderer);
        gFrontImage.texture = ResourceManager::LoadTexture("Assets/Images/gaugeFront.png", renderer);

        gBackImage.rect = { (int)(WINDOW_SIZE_X / 2 - 110 ),(int)(WINDOW_SIZE_Y - 170),300 , 250 };
        gFrontImage.rect = { gBackImage.rect.x, gBackImage.rect.y,gBackImage.rect.w ,gBackImage.rect.h };

        gauge = Gauge(100.0f, 15.0f, 22.5f, &gBackImage, &gFrontImage);
        gauge.ToggleBlink(true);
    }

    void ChangeSpeed(uINT x) {
        float newSpeed = DEFAULT_SPEED * (1 + 0.2f * x);
        _player.speed = newSpeed;
        _ebi.SetSpeed(newSpeed);
        _cloud.SetSpeed(newSpeed);
    }

    void update(float deltaTime) override {

        switch (FASE) {
            case 0:
                _player.UpdateFrameCheck(deltaTime);
                charaAnim.Update(deltaTime);
                if (charaAnim.GetIsAnimEnd()) {
                    FASE = 1;
                }
                break;
            case 1:
                elapsedTime += deltaTime;
                gauge.UpdateBlinkCheck(deltaTime);
                if (elapsedTime >= 1.0f) {
                    _player.point = Point(_player.rect.x, _player.rect.y);
                    gauge.ToggleBlink(false);
                    isFirstUpdate = true;
                    elapsedTime = 0;
                    FASE = 2;
                }
                break;
            case 2:/*----------------------------------ゲーム中--------------------------------*/
            {
                if (gauge.CheckGaugeZero()) {
                    FASE = 3;
                    _player.ToggleBlink(true);
                    return;
                }

                currentData->UpdateTime(deltaTime);

                headwind_scale = 0.4 + 0.01f * currentData->ebiCount;
                debuff_speed = 1.0f;
                
                if (_ebi.CheckCollision(_player.GetMidPoint(), _player.col_radius)) {
                    isGetEbi = true;
                }

                if (_player.IsOutOfScrern()) {
                    gauge.GetDotDamageEX();
                }

                if (_cloud.CheckCollision(_player.GetMidPoint())) {
                    debuff_speed = 0.5f;
                    headwind_scale *= 1.1f;
                    gauge.GetDotDamage();
                }

                _player.Move(0, deltaTime * headwind_scale);

                isMoving = false;
                const Uint8* keystates = SDL_GetKeyboardState(NULL);
                if (keystates[SDL_SCANCODE_UP]) {
                    isMoving = true;
                    _player.Move(0, -deltaTime * debuff_speed);
                }
                if (keystates[SDL_SCANCODE_DOWN]) {
                    isMoving = true;
                    _player.Move(0, deltaTime * debuff_speed);
                }
                if (keystates[SDL_SCANCODE_LEFT]) {
                    isMoving = true;
                    _player.Move(-deltaTime * debuff_speed, 0);
                }
                if (keystates[SDL_SCANCODE_RIGHT]) {
                    isMoving = true;
                    _player.Move(deltaTime * debuff_speed, 0);
                }
        
                if (isMoving) {             
                    if (!latest_isMoving) {
                        gauge.GetStartSpeedDamage();
                        latest_isMoving = true;
                    }                 
                    _player.TriggerOnceAnim();
                }
                else {
                    latest_isMoving = false;
                }

                gauge.UpdateGauge(deltaTime, isMoving);
                _player.UpdateFrameCheck(deltaTime);

                switch (currentData->stageNum)
                {
                    case 0:
                    {
                        if (isFirstUpdate) {

                            isFirstUpdate = false;
                        }
                        
                        if (isGetEbi) {
                            currentData->ebiCount++;
                            isGetEbi = false;
                            gauge.ChangeDotDamage(currentData->ebiCount);
                            gauge.GetHeal(currentData->ebiCount);
                            ChangeSpeed(currentData->ebiCount);
                            currentData->UpdateScore(currentData->ebiCount);
                        }

                        _cloud.Update(deltaTime);
                        _ebi.Update(deltaTime);
                    }
                    break;
                    case 1:

                        break;
                    case 3:

                        break;
                    case 4:

                        break;
                }
            }
                break;
            case 3:
                _player.UpdateBlinkCheck(deltaTime);
                elapsedTime += deltaTime;
                if (elapsedTime >= 3.0f) {
                    _player.ToggleBlink(false);
                    FASE = 4;
                }
                break;
            case 4:
                IsTransition = true;
                FASE = 0;
                break;
        }       
    }

    void render(SDL_Renderer* renderer) override {

        if (currentData->CheckTimerChanged()) {
            Size size1 = Size();
            sprintf_s(buffer, "| Time %d |", currentData->disp_timer);
            InfoText1.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY1, COLOR_WHITE, &size1, renderer);
            InfoText1.rect = { 10, 3 ,size1.width,size1.height };
        }

        if (currentData->CheckScoreChanged()) {
            Size size2 = Size();
            sprintf_s(buffer, "| Score %d |", currentData->disp_score);
            InfoText2.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY2, COLOR_WHITE, &size2, renderer);
            InfoText2.rect = { (WINDOW_SIZE_X - size2.width) / 2 , 3 ,size2.width,size2.height };
        }

        /*
        SDL_Rect rect = { 0, 0, 800, 55 };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);*/
        SDL_SetRenderDrawColor(renderer, 86, 157, 225, 255);

        switch (FASE) {
        case 0:
            _player.NormalRender(renderer);
            break;
        case 1:
            _player.NormalRender(renderer);
            gauge.RenderGauge(renderer);
            break;
        case 2:/*----------------------------------ゲーム中--------------------------------*/
            _player.Render(renderer);
            _cloud.Render(renderer);
            _ebi.Render(renderer);

            gauge.RenderGauge(renderer);
            break;
        case 3:
            _player.Render(renderer);
            _cloud.Render(renderer);
            _ebi.Render(renderer);

            gauge.RenderGauge(renderer);
            break;
        case 4:

            break;
        }

        SDL_RenderCopy(renderer, InfoText1.texture, nullptr, &InfoText1.rect);
        SDL_RenderCopy(renderer, InfoText2.texture, nullptr, &InfoText2.rect);
        SDL_RenderCopy(renderer, InfoText3.texture, nullptr, &InfoText3.rect);
    }
};

class ResultScene : public Scene {
private:
    Image InfoText1;
    Image InfoText2;
    Image InfoText3;
    Image InfoText4;
    Image InfoText5;

    int dispCount = 1;
    int maxDisp = 5;

    float elapsedTime = 0;

    char buffer[50];
public:
    ResultScene(SDL_Renderer* renderer, GameData* currentData)
    {
        int x = (int)(WINDOW_SIZE_X / 2);

        Size size1 = Size();
        sprintf_s(buffer, "-------- GameOver!! -------");
        InfoText1.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY1, COLOR_WHITE, &size1, renderer);
        InfoText1.rect = { x - size1.width / 2, 50, size1.width, size1.height };

        Size size2 = Size();
        sprintf_s(buffer, "Time %d", currentData->disp_timer);
        InfoText2.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY2, COLOR_WHITE, &size2, renderer);
        InfoText2.rect = { x - size2.width / 2, 150, size2.width, size2.height };

        Size size3 = Size();
        sprintf_s(buffer, "Stage %d", currentData->stageNum);
        InfoText3.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY3, COLOR_WHITE, &size3, renderer);
        InfoText3.rect = { x - size3.width / 2,InfoText2.rect.y + InfoText2.rect.h + 50, size3.width, size3.height };

        Size size4 = Size();
        sprintf_s(buffer, "# Score %d #", currentData->disp_score);
        InfoText4.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY4, COLOR_WHITE, &size4, renderer);
        InfoText4.rect = { x - size4.width / 2, InfoText3.rect.y + InfoText3.rect.h + 50, size4.width, size4.height };

        Size size5 = Size();
        sprintf_s(buffer, "Thanks for Playing!");
        InfoText5.texture = ResourceManager::LoadFontTexture(buffer, TEXT_KEY5, COLOR_WHITE, &size5, renderer);
        InfoText5.rect = { x - size5.width / 2, InfoText4.rect.y + InfoText4.rect.h + 100, size5.width, size5.height };
    }

    void update(float deltaTime) override {

        if ((dispCount < maxDisp)) {
            elapsedTime += deltaTime;
            if (elapsedTime >= 0.5f) {
                elapsedTime = 0;
                dispCount++;
                if (dispCount > maxDisp) {
                    dispCount = maxDisp;
                }
            }
        }
        else {
            const Uint8* keystates = SDL_GetKeyboardState(NULL);

            if (keystates[SDL_SCANCODE_UP]) {
                IsTransition = true;
            }
            if (keystates[SDL_SCANCODE_DOWN]) {
                IsTransition = true;
            }
            if (keystates[SDL_SCANCODE_LEFT]) {
                IsTransition = true;
            }
            if (keystates[SDL_SCANCODE_RIGHT]) {
                IsTransition = true;
            }
        }
    }

    void render(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        if (dispCount >= 1) {
            SDL_RenderCopy(renderer, InfoText1.texture, nullptr, &InfoText1.rect);
        }

        if (dispCount >= 2) {
            SDL_RenderCopy(renderer, InfoText2.texture, nullptr, &InfoText2.rect);
        }

        if (dispCount >= 3) {
            SDL_RenderCopy(renderer, InfoText3.texture, nullptr, &InfoText3.rect);
        }

        if (dispCount >= 4) {
            SDL_RenderCopy(renderer, InfoText4.texture, nullptr, &InfoText4.rect);
        }

        if (dispCount >= 5) {
            SDL_RenderCopy(renderer, InfoText5.texture, nullptr, &InfoText5.rect);
        }
    }
};

enum GameScene
{
    Title,  // タイトル
    InGame,  // ゲーム 
    Result  // 結果表示
};

//ゲームを管理するクラス
class GameManager {
private:
    GameData myData;

    GameScene currentScene;
    std::unique_ptr<Scene> scene;

    SDL_Renderer* renderer;

public:
    GameManager(SDL_Renderer* _rend) : renderer(_rend){
        scene = std::make_unique<TitleScene>(renderer);
        currentScene = GameScene::Title;
    }

    bool CheckTransision() {
        if (scene->isTransitioning()) {
            switch (currentScene)
            {
            case Title:
                currentScene = GameScene::InGame;
                scene = std::make_unique<InGameScene>(renderer, &myData);
                break;
            case InGame:
                currentScene = GameScene::Result;
                scene = std::make_unique<ResultScene>(renderer,&myData);
                break;
            case Result:
                currentScene = GameScene::InGame;
                myData = GameData();
                scene = std::make_unique<InGameScene>(renderer,&myData);
                break;
            }
            return true;
        }
        return false;
    }

    void Update(float deltaTime) {
        scene->update(deltaTime);
    }

    void Render() {
        scene->render(renderer);
    }
};