#include "pch.h"
#include "FFmpegRtspClient.h"

FFmpegRtspClient::FFmpegRtspClient()
	: m_formatContext(nullptr)
	, m_videoStreamId(-1)
	, m_videoCodec(nullptr)
	, m_videoCodecContext(nullptr)
	, m_videoFrame(nullptr)
	, m_videoSwFrame(nullptr)
	, m_audioStreamId(-1)
	, m_isOpened(false)
	, m_frameReceiveClientData(nullptr)
	, m_videoFrameReceiveCallback(nullptr)
	, m_audioFrameReceiveCallback(nullptr)
{
}

FFmpegRtspClient::~FFmpegRtspClient()
{
	Close();
}

void FFmpegRtspClient::SetFrameReceiveCallback(void* clientData, FrameReceiveCallback* video, FrameReceiveCallback* audio)
{
	m_frameReceiveClientData = clientData;
	m_videoFrameReceiveCallback = video;
	m_audioFrameReceiveCallback = audio;
}

bool FFmpegRtspClient::Open(const char* rtspURL, bool useTcp, int timeout)
{
	if (m_formatContext) Close();

	m_formatContext = avformat_alloc_context();
	if (!m_formatContext) return false;

	std::string usTimeout = "-1";
	if (timeout > 0) usTimeout = std::to_string(timeout * 1000 * 1000); // us -> s

	AVDictionary* options{};
	av_dict_set(&options, "stimeout", usTimeout.c_str(), 0);
	av_dict_set(&options, "rtsp_transport", useTcp ? "tcp" : "udp", 0);

	int err = avformat_open_input(&m_formatContext, rtspURL, nullptr, &options);
	av_dict_free(&options);
	if (err != 0) return false;

	err = avformat_find_stream_info(m_formatContext, nullptr);
	if (err < 0) return false;

	for (unsigned int i = 0; i < m_formatContext->nb_streams; i++)
	{
		auto codecpar = m_formatContext->streams[i]->codecpar;
		if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoStreamId = i;

			m_videoCodec = avcodec_find_decoder(codecpar->codec_id);
			if (!m_videoCodec) return false;

			m_videoCodecContext = avcodec_alloc_context3(m_videoCodec);
			if (!m_videoCodecContext) return false;

			AVHWDeviceType type{};
			FFmpegHelper::ConfigureHWDecoder(&type);
			if (type != AV_HWDEVICE_TYPE_NONE) av_hwdevice_ctx_create(&m_videoCodecContext->hw_device_ctx, type, nullptr, nullptr, 0);

			err = avcodec_parameters_to_context(m_videoCodecContext, codecpar);
			if (err < 0) return false;

			err = avcodec_open2(m_videoCodecContext, m_videoCodec, nullptr);
			if (err != 0) return false;

			m_videoFrame = av_frame_alloc();
			m_videoSwFrame = av_frame_alloc();
			if (!m_videoFrame || !m_videoSwFrame) return false;
		}
		else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_audioStreamId = i;
		}
	}

	m_frameGettingThread = std::thread(FrameGettingThreadProc, this);

	m_isOpened = true;
	return true;
}

void FFmpegRtspClient::Close()
{
	m_isOpened = false;

	if (m_frameGettingThread.joinable()) m_frameGettingThread.join();

	m_videoStreamId = -1;
	if (m_videoFrame)
	{
		av_frame_unref(m_videoFrame);
		av_freep(&m_videoFrame->data[0]);
		av_frame_free(&m_videoFrame);
		m_videoFrame = nullptr;
	}
	if (m_videoSwFrame)
	{
		av_frame_unref(m_videoSwFrame);
		av_freep(&m_videoSwFrame->data[0]);
		av_frame_free(&m_videoSwFrame);
		m_videoSwFrame = nullptr;
	}
	if (m_videoCodecContext)
	{
		avcodec_close(m_videoCodecContext);
		avcodec_free_context(&m_videoCodecContext);
		av_free(m_videoCodecContext);
		m_videoCodecContext = nullptr;
	}
	m_videoCodec = nullptr;
	if (m_formatContext)
	{
		avformat_close_input(&m_formatContext);
		avformat_free_context(m_formatContext);
		av_free(m_formatContext);
		m_formatContext = nullptr;
	}

	m_audioStreamId = -1;
}

int FFmpegRtspClient::GetVideoWidth(void)
{
	if (!m_isOpened || !m_videoCodecContext) return 0;
	return m_videoCodecContext->width;
}

int FFmpegRtspClient::GetVideoHeight(void)
{
	if (!m_isOpened || !m_videoCodecContext) return 0;
	return m_videoCodecContext->height;
}

AVPixelFormat FFmpegRtspClient::GetVideoPixelFormat(void)
{
	if (!m_isOpened || !m_videoCodecContext) return AV_PIX_FMT_NONE;

	AVHWDeviceType HWtype;
	FFmpegHelper::ConfigureHWDecoder(&HWtype);

	return HWtype == AV_HWDEVICE_TYPE_NONE ? 
		m_videoCodecContext->pix_fmt : FFmpegHelper::GetHWPixelFormat(HWtype);
}

void FFmpegRtspClient::FrameGettingThreadProc(void* param)
{
	FFmpegRtspClient* rtspClient = (FFmpegRtspClient*)param;
	rtspClient->FrameGettingFunc();
}

void FFmpegRtspClient::FrameGettingFunc(void)
{
	AVPacket pkt;
	AVFrame* tempVideoFrame = nullptr;

	int ret = 0, err = 0;
	while (m_isOpened)
	{
		ret = av_read_frame(m_formatContext, &pkt);
		if (ret == AVERROR(EAGAIN)) continue;

		if (pkt.stream_index == m_videoStreamId)
		{
			err = avcodec_send_packet(m_videoCodecContext, &pkt);
			if (err == 0)
			{
				err = avcodec_receive_frame(m_videoCodecContext, m_videoFrame);
				if (err == 0)
				{
					if (m_videoCodecContext->hw_device_ctx)
					{
						err = av_hwframe_transfer_data(m_videoSwFrame, m_videoFrame, 0);
						if (err == 0)
						{
							tempVideoFrame = m_videoSwFrame;
						}
						else
						{
							tempVideoFrame = m_videoFrame;
						}
					}
					else
					{
						tempVideoFrame = m_videoFrame;
					}

					if (m_videoFrameReceiveCallback)
					{
						m_videoFrameReceiveCallback(m_frameReceiveClientData, tempVideoFrame->width, tempVideoFrame->height, tempVideoFrame);
					}
				}
			}
		}
		else if (pkt.stream_index == m_audioStreamId)
		{
		}

		tempVideoFrame = nullptr;
		av_frame_unref(m_videoFrame);
		av_frame_unref(m_videoSwFrame);
		av_packet_unref(&pkt);
	}
}