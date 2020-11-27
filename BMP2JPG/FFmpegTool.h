#pragma once
extern "C"
{
#include "libavcodec\codec.h"
#include "libavcodec\packet.h"
#include "libavcodec\avcodec.h"
#include "libavutil\rational.h"
#include "libavutil\pixfmt.h"
#include "libavutil\frame.h"
#include "libswscale\swscale.h"
#include "libavutil\imgutils.h"
#include "libavfilter\avfilter.h"
#include <wtypes.h>
};

class FFmpegTool
{
private:
	BITMAPFILEHEADER bmpFHeader;
	BITMAPINFOHEADER bmpIHeader;
	int nWidth;
	int nHeight;
	int bmpHeadLen;
public:
	FFmpegTool();
	~FFmpegTool();
	void Init();
	void Release();
	int bmp2jpg(uint8_t* in_buffer, AVPacket* avpkt);

private:
	void bmp2rgb(uint8_t* img_buffer, uint8_t* rgb_buffer);
	int rgb2yuv(uint8_t* rgb_buffer, uint8_t* yuv_buffer, AVFrame* m_pRGBFrame, AVFrame* m_pYUVFrame);
	int yuv2jpg(AVFrame* m_pYUVFrame, AVPacket* avpkt);
};

