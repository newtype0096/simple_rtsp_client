// LiveStreamControl.cpp: 구현 파일
//

#include "pch.h"
#include "SimpleRtspClient.h"
#include "LiveStreamControl.h"

// LiveStreamControl

IMPLEMENT_DYNAMIC(LiveStreamControl, CWnd)

LiveStreamControl::LiveStreamControl()
	: m_ffmpegRtspClient(nullptr)
	, m_videoFrameConverter(nullptr)
	, m_videoRenderer(nullptr)
{
}

LiveStreamControl::~LiveStreamControl()
{
	Close();
}

BEGIN_MESSAGE_MAP(LiveStreamControl, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// LiveStreamControl 메시지 처리기

int LiveStreamControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	return 0;
}

void LiveStreamControl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

void LiveStreamControl::OnDestroy()
{
	CWnd::OnDestroy();

	Close();
}

bool LiveStreamControl::Open(const char* rtspURL, bool useTcp)
{
	if (m_ffmpegRtspClient) Close();

	m_ffmpegRtspClient = new FFmpegRtspClient;
	m_ffmpegRtspClient->SetFrameReceiveCallback(this, ReceiveDecodedVideoFrame, nullptr);
	if (!m_ffmpegRtspClient->Open(rtspURL, useTcp)) return false;

	CRect rect;
	GetClientRect(rect);

	m_videoFrameConverter = new FFmpegVideoFrameConverter;
	if (!m_videoFrameConverter->Create(m_ffmpegRtspClient->GetVideoWidth(),
		m_ffmpegRtspClient->GetVideoHeight(),
		m_ffmpegRtspClient->GetVideoPixelFormat(),
		rect.Width(),
		rect.Height(),
		AV_PIX_FMT_YUYV422)) return false;

	m_videoRenderer = new SDL2VideoRenderer();
	if (!m_videoRenderer->Create(GetSafeHwnd(),
		m_ffmpegRtspClient->GetVideoWidth(),
		m_ffmpegRtspClient->GetVideoHeight(),
		rect.Width(),
		rect.Height())) return false;

	return false;
}

void LiveStreamControl::Close(void)
{
	if (m_ffmpegRtspClient)
	{
		delete m_ffmpegRtspClient;
		m_ffmpegRtspClient = nullptr;
	}

	if (m_videoFrameConverter)
	{
		delete m_videoFrameConverter;
		m_videoFrameConverter = nullptr;
	}

	if (m_videoRenderer)
	{
		delete m_videoRenderer;
		m_videoRenderer = nullptr;
	}
}

void LiveStreamControl::Resize(int width, int height)
{
	std::unique_lock<std::mutex> lock(m_cs);
	if (!m_videoRenderer || !m_ffmpegRtspClient) return;

	SetWindowPos(nullptr, -1, -1, width, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);

	m_videoRenderer->Create(GetSafeHwnd(),
		m_ffmpegRtspClient->GetVideoWidth(),
		m_ffmpegRtspClient->GetVideoHeight(),
		width, height);
	m_videoFrameConverter->Create(m_ffmpegRtspClient->GetVideoWidth(),
		m_ffmpegRtspClient->GetVideoHeight(),
		m_ffmpegRtspClient->GetVideoPixelFormat(),
		width, height,
		AV_PIX_FMT_YUYV422);
}

void LiveStreamControl::ReceiveDecodedVideoFrame(void* clientData, int width, int height, AVFrame* frame)
{
	LiveStreamControl* object = (LiveStreamControl*)clientData;

	std::unique_lock<std::mutex> lock(object->m_cs);
	if (!object->m_videoFrameConverter || !object->m_videoRenderer) return;

	auto convertedFrame = object->m_videoFrameConverter->Convert(frame);
	object->m_videoRenderer->Update(convertedFrame->data[0], convertedFrame->linesize[0]);
	object->m_videoRenderer->Present();
}