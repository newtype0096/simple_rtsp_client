#pragma once
#include <string>
#include <thread>
#include "FFmpegHelper.h"

class FFmpegRtspClient
{
public:
	FFmpegRtspClient();
	virtual ~FFmpegRtspClient();

	typedef void (FrameReceiveCallback)(void* clientData, int width, int height, AVFrame* frame);
	void SetFrameReceiveCallback(void* clientData, FrameReceiveCallback* video, FrameReceiveCallback*);

	bool Open(const char* rtspURL, bool useTcp, int timeout = 3);
	bool IsOpened(void) const { return m_isOpened; }

	void Close();

	int GetVideoWidth(void);
	int GetVideoHeight(void);
	AVPixelFormat GetVideoPixelFormat(void);

private:
	AVFormatContext* m_formatContext;

	int m_videoStreamId;
	const AVCodec* m_videoCodec;
	AVCodecContext* m_videoCodecContext;
	AVFrame* m_videoFrame;
	AVFrame* m_videoSwFrame;

	int m_audioStreamId;

	bool m_isOpened;

	std::thread m_frameGettingThread;
	static void FrameGettingThreadProc(void* param);
	void FrameGettingFunc(void);

	void* m_frameReceiveClientData;
	FrameReceiveCallback* m_videoFrameReceiveCallback;
	FrameReceiveCallback* m_audioFrameReceiveCallback;
};
