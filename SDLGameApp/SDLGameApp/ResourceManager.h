// ResourceManager.h
#pragma once

#include <map>
#include "Utility.h"

class ResourceManager {
public:
    static SDL_Texture* LoadTexture(const std::string& filePath, SDL_Renderer* renderer);
    static void LoadFont(const std::string& filePath);
    static SDL_Texture* LoadFontTexture(const std::string& text, SDL_Color textColor, Size* size, SDL_Renderer* renderer);
    static SDL_Texture* LoadFontTexture_Effect(const std::string& text, SDL_Color textColor, Size* size, SDL_Renderer* renderer);
    static void CleanUp();
private:
    static std::map<std::string, SDL_Texture*> textures;
    static TTF_Font* font;
    static const std::string key;
    static const std::string key2;
};