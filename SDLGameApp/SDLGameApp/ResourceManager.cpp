#include "ResourceManager.h"
#include <iostream>

std::map<std::string, SDL_Texture*> ResourceManager::textures;
TTF_Font* ResourceManager::font = nullptr;

SDL_Texture* ResourceManager::LoadTexture(const std::string& filePath, SDL_Renderer* renderer) {
    if (textures.find(filePath) == textures.end()) {
        SDL_Surface* surface = IMG_Load(filePath.c_str());
        if (!surface) {
            return nullptr;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            return nullptr;
        }
        textures[filePath] = texture;
    }
    return textures[filePath];
}

void ResourceManager::LoadFont(const std::string& filePath) {
    if (font) {
        TTF_CloseFont(font);
    }
    font = TTF_OpenFont(filePath.c_str(), 28);
}

SDL_Texture* ResourceManager::LoadFontTexture(const std::string& text,const std::string key, SDL_Color textColor,Size* size, SDL_Renderer* renderer) {
    if (textures.find(key) != textures.end()) {
        SDL_DestroyTexture(textures[key]);
        textures.erase(key);
    }
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, text.c_str(), textColor);
    if (!textSurface) {
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (!texture) {
        return nullptr;
    }
    textures[key] = texture;
    SDL_QueryTexture(texture, nullptr, nullptr, &size->width, &size->height);
    return texture;
}

void ResourceManager::CleanUp() {
    for (auto& texturePair : textures) {
        SDL_DestroyTexture(texturePair.second);
    }
    textures.clear();

    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
}
