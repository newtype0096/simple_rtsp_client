#include "pch.h"
#include "FFmpegHelper.h"

#include <set>

void av_log_output(void* ptr, int level, const char* fmt, va_list vargs)
{
	if (level > av_log_get_level()) return;

	char lineBuffer[1024]{};
	int printPrefix = 1;
	av_log_format_line(ptr, level, fmt, vargs, lineBuffer, 1024, &printPrefix);
	printf(lineBuffer);
	OutputDebugString(lineBuffer);
}

void FFmpegHelper::Init()
{
	static bool isInit = false;
	if (isInit) return;

	avdevice_register_all();
	avformat_network_init();

	av_log_set_level(AV_LOG_VERBOSE);
	av_log_set_callback(av_log_output);

	isInit = true;
}

void FFmpegHelper::ConfigureHWDecoder(_Out_ AVHWDeviceType* HWtype)
{
	if (!HWtype) return;

	*HWtype = AV_HWDEVICE_TYPE_NONE;
	std::set<AVHWDeviceType> availableHWDecoders;

	auto type = AV_HWDEVICE_TYPE_NONE;
	auto number = 0;

	while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
	{
		availableHWDecoders.insert(type);
	}

	if (availableHWDecoders.empty())
	{
		*HWtype = AV_HWDEVICE_TYPE_NONE;
		return;
	}

	if (availableHWDecoders.find(AV_HWDEVICE_TYPE_DXVA2) != availableHWDecoders.end())
	{
		*HWtype = AV_HWDEVICE_TYPE_DXVA2;
		return;
	}

	*HWtype = *availableHWDecoders.begin();
}

AVPixelFormat FFmpegHelper::GetHWPixelFormat(AVHWDeviceType HWtype)
{
	switch (HWtype)
	{
	case AV_HWDEVICE_TYPE_VDPAU:
		return AV_PIX_FMT_VDPAU;
	case AV_HWDEVICE_TYPE_CUDA:
		return AV_PIX_FMT_CUDA;
	case AV_HWDEVICE_TYPE_VAAPI:
		return AV_PIX_FMT_VAAPI;
	case AV_HWDEVICE_TYPE_DXVA2:
		return AV_PIX_FMT_NV12;
	case AV_HWDEVICE_TYPE_QSV:
		return AV_PIX_FMT_QSV;
	case AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
		return AV_PIX_FMT_VIDEOTOOLBOX;
	case AV_HWDEVICE_TYPE_D3D11VA:
		return AV_PIX_FMT_NV12;
	case AV_HWDEVICE_TYPE_DRM:
		return AV_PIX_FMT_DRM_PRIME;
	case AV_HWDEVICE_TYPE_OPENCL:
		return AV_PIX_FMT_OPENCL;
	case AV_HWDEVICE_TYPE_MEDIACODEC:
		return AV_PIX_FMT_MEDIACODEC;
	case AV_HWDEVICE_TYPE_NONE:
	default:
		return AV_PIX_FMT_NONE;
	}
}