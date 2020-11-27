#include "FFmpegTool.h"

FFmpegTool::FFmpegTool()
{
	Init();
}

FFmpegTool::~FFmpegTool()
{
	Release();
}

void FFmpegTool::Init()
{
}

void FFmpegTool::Release()
{
}

int FFmpegTool::bmp2jpg(uint8_t* in_buffer, AVPacket* avpkt)
{
	int retVal = 0;
	
	memcpy(&bmpFHeader, in_buffer, sizeof(BITMAPFILEHEADER));
	bmpHeadLen = bmpFHeader.bfOffBits - sizeof(BITMAPFILEHEADER);

	memcpy(&bmpIHeader, in_buffer + sizeof(BITMAPFILEHEADER), bmpHeadLen);
	nWidth = bmpIHeader.biWidth;
	nHeight = bmpIHeader.biHeight;

	int rgb_frame_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, nWidth, nHeight, 1);
	int yuv_frame_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, nWidth, nHeight, 1);
	uint8_t* rgb_buffer = (uint8_t*)av_malloc(rgb_frame_size);//[bmpFHeader.bfSize - bmpFHeader.bfOffBits];
	uint8_t* yuv_buffer = (uint8_t*)av_malloc(yuv_frame_size);

	AVFrame* m_pRGBFrame = NULL;
	AVFrame* m_pYUVFrame = NULL;
	m_pRGBFrame = av_frame_alloc();
	m_pYUVFrame = av_frame_alloc();

	bmp2rgb(in_buffer, rgb_buffer);

	retVal = rgb2yuv(rgb_buffer, yuv_buffer, m_pRGBFrame, m_pYUVFrame);
	if (retVal <= 0) 
	{
		av_free(rgb_buffer);
		av_free(yuv_buffer);
		av_frame_free(&m_pRGBFrame);
		av_frame_free(&m_pYUVFrame);
		return -1;
	}

	retVal = yuv2jpg(m_pYUVFrame, avpkt);
	if (retVal < 0)
	{
		av_free(rgb_buffer);
		av_free(yuv_buffer);
		av_frame_free(&m_pRGBFrame);
		av_frame_free(&m_pYUVFrame);
		return -1;
	}

	av_free(rgb_buffer);
	av_free(yuv_buffer);
	av_frame_free(&m_pRGBFrame);
	av_frame_free(&m_pYUVFrame);

	return 0;
}

void FFmpegTool::bmp2rgb(uint8_t* img_buffer, uint8_t* rgb_buffer)
{
	img_buffer += bmpFHeader.bfOffBits;
	int nDataLen = bmpFHeader.bfSize - bmpFHeader.bfOffBits;
	
	memcpy(rgb_buffer, img_buffer, nDataLen);
}

int FFmpegTool::rgb2yuv(uint8_t* rgb_buffer, uint8_t* yuv_buffer, AVFrame* m_pRGBFrame, AVFrame* m_pYUVFrame)
{
	int retVal = 0;

	av_image_fill_arrays((uint8_t**)&m_pRGBFrame->data, m_pRGBFrame->linesize, (uint8_t*)rgb_buffer, AV_PIX_FMT_BGR24, nWidth, nHeight, 1);
	av_image_fill_arrays((uint8_t**)&m_pYUVFrame->data, m_pYUVFrame->linesize, (uint8_t*)yuv_buffer, AV_PIX_FMT_YUV420P, nWidth, nHeight, 1);
	m_pYUVFrame->format = AV_PIX_FMT_YUV420P;
	m_pYUVFrame->width = nWidth;
	m_pYUVFrame->height = nHeight;
	m_pRGBFrame->format = AV_PIX_FMT_BGR24;
	m_pRGBFrame->width = nWidth;
	m_pRGBFrame->height = nHeight;

	m_pRGBFrame->data[0] += m_pRGBFrame->linesize[0] * (nHeight - 1);
	m_pRGBFrame->linesize[0] *= -1;
	m_pRGBFrame->data[1] += m_pRGBFrame->linesize[1] * (nHeight / 2 - 1);
	m_pRGBFrame->linesize[1] *= -1;
	m_pRGBFrame->data[2] += m_pRGBFrame->linesize[2] * (nHeight / 2 - 1);
	m_pRGBFrame->linesize[2] *= -1;

	SwsContext* scxt = sws_getContext(nWidth, nHeight, AV_PIX_FMT_BGR24, nWidth, nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	retVal = sws_scale(scxt, m_pRGBFrame->data, m_pRGBFrame->linesize, 0, nHeight, m_pYUVFrame->data, m_pYUVFrame->linesize);

	sws_freeContext(scxt);
	return retVal;
}

int FFmpegTool::yuv2jpg(AVFrame* m_pYUVFrame, AVPacket* avpkt)
{
	int retVal = 0;

	AVCodecContext* c = avcodec_alloc_context3(NULL);
	if (!c)
	{
		avcodec_close(c);
		av_free(c);
		return -1;
	}

	c->codec_id = AV_CODEC_ID_MJPEG;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->pix_fmt = AV_PIX_FMT_YUVJ420P;
	c->width = nWidth;
	c->height = nHeight;
	c->time_base.den = 30;
	c->time_base.num = 1;
	//c->bit_rate = 3000000;
	//c->gop_size = 10;
	//c->max_b_frames = 1;
	//c->thread_count = 1;

	AVCodec* pCodecJPG = avcodec_find_encoder(c->codec_id);
	if (!pCodecJPG)
	{
		avcodec_close(c);
		av_free(c);
		return -1;
	}

	retVal = avcodec_open2(c, pCodecJPG, NULL);
	if (retVal < 0)
	{
		avcodec_close(c);
		av_free(c);
		return retVal;
	}

	//retVal = avcodec_encode_video2(c, &avpkt, m_pYUVFrame, &got_packet_ptr);
	retVal = avcodec_send_frame(c, m_pYUVFrame);
	if (retVal < 0)
	{
		avcodec_close(c);
		av_free(c);
		return retVal;
	}

	retVal = avcodec_receive_packet(c, avpkt);
	if (retVal < 0)
	{
		avcodec_close(c);
		av_free(c);
		return retVal;
	}

	avcodec_close(c);
	av_free(c);
	return retVal;
}
