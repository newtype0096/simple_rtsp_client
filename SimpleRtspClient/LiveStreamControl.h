#pragma once
#include "SDL2VideoRenderer.h"
#include "FFmpegRtspClient.h"
#include "FFmpegVideoFrameConverter.h"


// LiveStreamControl

class LiveStreamControl : public CWnd
{
	DECLARE_DYNAMIC(LiveStreamControl)

public:
	LiveStreamControl();
	virtual ~LiveStreamControl();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

public:
	bool Open(const char* rtspURL, bool useTcp);
	void Close(void);

	void Resize(int width, int height);

private:
	FFmpegRtspClient* m_ffmpegRtspClient;
	FFmpegVideoFrameConverter* m_videoFrameConverter;

	SDL2VideoRenderer* m_videoRenderer;

	static void ReceiveDecodedVideoFrame(void* clientData, int width, int height, AVFrame* frame);
};