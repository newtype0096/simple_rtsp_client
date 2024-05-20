#pragma once
#include <string>

extern "C"
{
#pragma warning(disable:4819)
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#pragma warning(default:4819)
}

class FFmpegHelper
{
public:
	static void Init();
	static void ConfigureHWDecoder(AVHWDeviceType* HWtype);
	static AVPixelFormat GetHWPixelFormat(AVHWDeviceType HWtype);
};