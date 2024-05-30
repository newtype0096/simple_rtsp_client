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

bool SDL2VideoRenderer::Create(HWND hWnd, int srcWidth, int srcHeight, int dstWidth, int dstHeight)
{
	if (m_window) Close();

	std::unique_lock<std::mutex> lock(m_cs);

	m_window = SDL_CreateWindowFrom(hWnd);
	if (!m_window) return false;

	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
	if (!m_renderer) return false;

	SDL_RenderSetLogicalSize(m_renderer, srcWidth, srcHeight);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING, dstWidth, dstHeight);
	if (!m_texture) return false;

	m_isCreated = true;
	return true;
}

void SDL2VideoRenderer::Close(void)
{
	std::unique_lock<std::mutex> lock(m_cs);

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
	std::unique_lock<std::mutex> lock(m_cs);

	if (!m_texture || !m_isCreated) return;

	SDL_UpdateTexture(m_texture, nullptr, yuy2Data, lineSize);
}

void SDL2VideoRenderer::Present(void)
{
	std::unique_lock<std::mutex> lock(m_cs);

	if (!m_renderer || !m_texture || !m_isCreated) return;

	SDL_RenderClear(m_renderer);
	SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	SDL_RenderPresent(m_renderer);
}

void SDL2VideoRenderer::Resize(int width, int height)
{
	std::unique_lock<std::mutex> lock(m_cs);

	if (!m_window || !m_renderer || !m_texture || !m_isCreated) return;

	SDL_SetWindowSize(m_window, width, height);

	SDL_RenderClear(m_renderer);
	SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	SDL_RenderPresent(m_renderer);
}