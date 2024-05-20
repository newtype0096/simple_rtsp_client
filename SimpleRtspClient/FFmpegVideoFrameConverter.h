#pragma once
#include "FFmpegHelper.h"

class FFmpegVideoFrameConverter
{
public:
	FFmpegVideoFrameConverter();
	virtual ~FFmpegVideoFrameConverter();

	bool Create(int srcWidth, int srcHeight, AVPixelFormat srcFormat,
		int dstWidth, int dstHeight, AVPixelFormat dstFormat);
	bool IsCreated(void) const { return m_isCreated; }

	void Close(void);

	AVFrame* Convert(AVFrame* srcFrame);

private:
	SwsContext* m_swsContext;

	AVPixelFormat m_dstFormat;
	int m_dstBufferSize;
	unsigned char* m_dstBuffer;
	AVFrame* m_dstFrame;

	bool m_isCreated;
};

