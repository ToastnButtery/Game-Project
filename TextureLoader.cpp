#include "TextureLoader.h"


std::unordered_map<std::string, SDL_Texture*> TextureLoader::umapTexturesLoaded;




SDL_Texture* TextureLoader::loadTexture(SDL_Renderer* renderer, std::string filename) {
    if (filename != "") {
        auto found = umapTexturesLoaded.find(filename);

        if (found != umapTexturesLoaded.end()) {

            return found->second;
        }
        else {

            std::string filepath = "Data/Images/" + filename;


            SDL_Surface* surfaceTemp = SDL_LoadBMP(filepath.c_str());
            if (surfaceTemp != nullptr) {


                SDL_Texture* textureOutput = SDL_CreateTextureFromSurface(renderer, surfaceTemp);

                SDL_FreeSurface(surfaceTemp);

                if (textureOutput != nullptr) {

                    SDL_SetTextureBlendMode(textureOutput, SDL_BLENDMODE_BLEND);

                    umapTexturesLoaded[filename] = textureOutput;

                    return textureOutput;
                }
            }
        }
    }

    return nullptr;
}



void TextureLoader::deallocateTextures() {

    while (umapTexturesLoaded.empty() == false) {
        auto it = umapTexturesLoaded.begin();
        if (it->second != nullptr)
            SDL_DestroyTexture(it->second);

        umapTexturesLoaded.erase(it);
    }
}
