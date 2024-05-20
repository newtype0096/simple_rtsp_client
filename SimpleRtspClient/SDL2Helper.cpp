#include "pch.h"
#include "SDL2Helper.h"

void SDL2Helper::Init()
{
	static bool isInit = false;
	if (isInit) return;

	assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);

	isInit = true;
}

void SDL2Helper::DeInit()
{
	SDL_Quit();
}
