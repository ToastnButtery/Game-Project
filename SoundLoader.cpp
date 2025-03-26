#include "SoundLoader.h"


std::unordered_map<std::string, Mix_Chunk*> SoundLoader::umapSoundsLoaded;




Mix_Chunk* SoundLoader::loadSound(std::string filename) {
    if (filename != "") {
        auto found = umapSoundsLoaded.find(filename);

        if (found != umapSoundsLoaded.end()) {

            return found->second;
        }
        else {

            std::string filepath = "Data/Sounds/" + filename;


            Mix_Chunk* mix_Chunk = Mix_LoadWAV(filepath.c_str());
            if (mix_Chunk != nullptr) {


                umapSoundsLoaded[filename] = mix_Chunk;

                return mix_Chunk;
            }
        }
    }

    return nullptr;
}



void SoundLoader::deallocateSounds() {

    Mix_HaltChannel(-1);


    while (umapSoundsLoaded.empty() == false) {
        auto it = umapSoundsLoaded.begin();
        if (it->second != nullptr)
            Mix_FreeChunk(it->second);

        umapSoundsLoaded.erase(it);
    }
}
