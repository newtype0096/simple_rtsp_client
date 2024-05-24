#include "pch.h"
#include "SDL2VideoRenderer.h"

SDL2VideoRenderer::SDL2VideoRenderer()
	: m_window(nullptr)
	, m_renderer(nullptr)
	, m_texture(nullptr)
	, m_isCreated(false)
{
}

SDL2VideoRenderer::~SDL2VideoRenderer()
{
	Close();
}

bool SDL2VideoRenderer::Create(HWND hWnd, int srcWidth, int srcHeight)
{
	if (m_window) Close();

	m_window = SDL_CreateWindowFrom(hWnd);
	if (!m_window) return false;

	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
	if (!m_renderer) return false;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, srcWidth, srcHeight);
	if (!m_texture) return false;

	m_isCreated = true;
	return true;
}

void SDL2VideoRenderer::Close(void)
{
	m_isCreated = false;

	if (m_texture)
	{
		SDL_DestroyTexture(m_texture);
		m_texture = nullptr;
	}

	if (m_renderer)
	{
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}

	if (m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
}

void SDL2VideoRenderer::Update(unsigned char* yuy2Data, int lineSize)
{
	if (!m_texture || !m_isCreated) return;

	SDL_UpdateTexture(m_texture, nullptr, yuy2Data, lineSize);
}

void SDL2VideoRenderer::Present(void)
{
	if (!m_renderer || !m_texture || !m_isCreated) return;

	SDL_RenderClear(m_renderer);
	SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	SDL_RenderPresent(m_renderer);
}
