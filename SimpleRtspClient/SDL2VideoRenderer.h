#pragma once
#include "SDL2Helper.h"

class SDL2VideoRenderer
{
public:
	SDL2VideoRenderer();
	virtual ~SDL2VideoRenderer();

	bool Create(HWND hWnd, int srcWidth, int srcHeight);
	bool IsCreated(void) const { return m_isCreated; }

	void Close(void);

	void Update(unsigned char* data, int lineSize);

	void Present(void);

private:
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	SDL_Texture* m_texture;

	bool m_isCreated;
};
