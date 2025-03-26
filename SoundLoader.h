#pragma once
#include <string>
#include <unordered_map>
#include "SDL2/SDL_mixer.h"



class SoundLoader
{
public:
	static Mix_Chunk* loadSound(std::string filename);
	static void deallocateSounds();


private:
	static std::unordered_map<std::string, Mix_Chunk*> umapSoundsLoaded;
};
