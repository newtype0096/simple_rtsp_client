#include "pch.h"
#include "FFmpegVideoFrameConverter.h"

FFmpegVideoFrameConverter::FFmpegVideoFrameConverter()
	: m_swsContext(nullptr)
	, m_dstFormat(AV_PIX_FMT_NONE)
	, m_dstBufferSize(-1)
	, m_dstBuffer(nullptr)
	, m_dstFrame(nullptr)
	, m_isCreated(false)
{
}

FFmpegVideoFrameConverter::~FFmpegVideoFrameConverter()
{
	Close();
}

bool FFmpegVideoFrameConverter::Create(int srcWidth, int srcHeight, AVPixelFormat srcFormat,
	int dstWidth, int dstHeight, AVPixelFormat dstFormat)
{
	if (m_swsContext) Close();

	m_dstFormat = dstFormat;

	m_swsContext = sws_getContext(srcWidth, srcHeight, srcFormat,
		dstWidth, dstHeight, dstFormat,
		SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
	if (!m_swsContext) return false;

	m_dstBufferSize = av_image_get_buffer_size(dstFormat, dstWidth, dstHeight, 1);
	if (m_dstBufferSize < 0) return false;

	m_dstBuffer = (unsigned char*)av_mallocz(m_dstBufferSize);
	if (!m_dstBuffer) return false;

	m_dstFrame = av_frame_alloc();
	if (!m_dstFrame) return false;

	if (av_image_fill_arrays(m_dstFrame->data, m_dstFrame->linesize, m_dstBuffer, dstFormat, dstWidth, dstHeight, 1) < 0) return false;

	m_isCreated = true;
	return true;
}

void FFmpegVideoFrameConverter::Close(void)
{
	m_isCreated = false;

	if (m_dstFrame)
	{
		av_frame_unref(m_dstFrame);
		av_freep(&m_dstFrame->data[0]);
		av_frame_free(&m_dstFrame);
		m_dstFrame = nullptr;
	}

	if (m_dstBuffer)
	{
		av_free(m_dstBuffer);
		m_dstBuffer = nullptr;
	}

	if (m_swsContext)
	{
		sws_freeContext(m_swsContext);
		m_swsContext = nullptr;
	}

	m_dstFormat = AV_PIX_FMT_NONE;
	m_dstBufferSize = -1;
}

AVFrame* FFmpegVideoFrameConverter::Convert(AVFrame* srcFrame)
{
	if (!m_isCreated || !m_swsContext) return nullptr;

	sws_scale(m_swsContext, srcFrame->data, srcFrame->linesize, 0, srcFrame->height, m_dstFrame->data, m_dstFrame->linesize);
	return m_dstFrame;
}
