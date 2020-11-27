# ffmpeg_usage

# Convert BMP image data to JPG image data

We use ffmpeg lib to conver BMP image data to JPG image data

1. BMP→BGR

The BMP image has a **bitmap file header** and **bitmap information header**. And the rest is **BGR data**. Remove the two headers and abstract the BGR data. Note that bitmaps are encoded in BGR, not RGB.

```cpp
int img_size = 6220854;
BYTE* buffer = NULL;
bmp_buffer = new BYTE[img_size]; // read a bmp image and store the data into bmp_buffer

BITMAPFILEHEADER bmpFHeader;
BITMAPINFOHEADER bmpIHeader;
int nWidth;
int nHeight;
int bmpHeadLen;

int bgr_frame_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, nWidth, nHeight, 1);
uint8_t* bgr_buffer = (uint8_t*)av_malloc(bgr_frame_size);

memcpy(&bmpFHeader, bmp_buffer, sizeof(BITMAPFILEHEADER));
bmpHeadLen = bmpFHeader.bfOffBits - sizeof(BITMAPFILEHEADER);

memcpy(&bmpIHeader, bmp_buffer + sizeof(BITMAPFILEHEADER), bmpHeadLen);
nWidth = bmpIHeader.biWidth;
nHeight = bmpIHeader.biHeight;

bmp_buffer += bmpFHeader.bfOffBits;
int nDataLen = bmpFHeader.bfSize - bmpFHeader.bfOffBits;
	
memcpy(bgr_buffer, bmp_buffer, nDataLen);
```

2. BRG→YUV

If we want to compress the image data, we need to convert BGR to YUV. Note that the BMP image store BGR data up and down due to some historical reasons. We need to flip the BGR data first and then convert it into YUV format.

```cpp
int yuv_frame_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, nWidth, nHeight, 1);
uint8_t* yuv_buffer = (uint8_t*)av_malloc(yuv_frame_size);

AVFrame* m_pBGRFrame = NULL;
AVFrame* m_pYUVFrame = NULL;
m_pBGRFrame = av_frame_alloc();
m_pYUVFrame = av_frame_alloc();

av_image_fill_arrays((uint8_t**)&m_pBGRFrame ->data, m_pBGRFrame ->linesize, (uint8_t*)bgr_buffer, AV_PIX_FMT_BGR24, nWidth, nHeight, 1);
av_image_fill_arrays((uint8_t**)&m_pYUVFrame->data, m_pYUVFrame->linesize, (uint8_t*)yuv_buffer, AV_PIX_FMT_YUV420P, nWidth, nHeight, 1);
m_pYUVFrame->format = AV_PIX_FMT_YUV420P;
m_pYUVFrame->width = nWidth;
m_pYUVFrame->height = nHeight;
m_pBGRFrame ->format = AV_PIX_FMT_BGR24;
m_pBGRFrame ->width = nWidth;
m_pBGRFrame ->height = nHeight;

// flip the bgr data
m_pBGRFrame ->data[0] += m_pRGBFrame->linesize[0] * (nHeight - 1);
m_pBGRFrame ->linesize[0] *= -1;
m_pBGRFrame ->data[1] += m_pRGBFrame->linesize[1] * (nHeight / 2 - 1);
m_pBGRFrame ->linesize[1] *= -1;
m_pBGRFrame ->data[2] += m_pRGBFrame->linesize[2] * (nHeight / 2 - 1);
m_pBGRFrame ->linesize[2] *= -1;

SwsContext* scxt = sws_getContext(nWidth, nHeight, AV_PIX_FMT_BGR24, nWidth, nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
int retVal = sws_scale(scxt, m_pBGRFrame ->data, m_pBGRFrame ->linesize, 0, nHeight, m_pYUVFrame->data, m_pYUVFrame->linesize);

sws_freeContext(scxt);
```

3. YUV→JPG

Encode the YUV into JPG format. Here we store the jpg data into a file the check if the result is correct.

```cpp
int retVal = 0;

AVPacket* avpkt;
avpkt = av_packet_alloc();
av_init_packet(avpkt);

AVCodecContext* c = avcodec_alloc_context3(NULL);
if (!c)
{
	avcodec_close(c);
	av_free(c);
}

c->codec_id = AV_CODEC_ID_MJPEG;
c->codec_type = AVMEDIA_TYPE_VIDEO;
c->pix_fmt = AV_PIX_FMT_YUVJ420P;
c->width = nWidth;
c->height = nHeight;
c->time_base.den = 30;
c->time_base.num = 1;

AVCodec* pCodecJPG = avcodec_find_encoder(c->codec_id);
if (!pCodecJPG)
{
	avcodec_close(c);
	av_free(c);
}

retVal = avcodec_open2(c, pCodecJPG, NULL);
if (retVal < 0)
{
	avcodec_close(c);
	av_free(c);
}

//avcodec_encode_video2 deprecated, use avcodec_send_frame()/avcodec_receive_packet() instead
//retVal = avcodec_encode_video2(c, &avpkt, m_pYUVFrame, &got_packet_ptr);
retVal = avcodec_send_frame(c, m_pYUVFrame);
if (retVal < 0)
{
	avcodec_close(c);
	av_free(c);

}

retVal = avcodec_receive_packet(c, avpkt);
if (retVal < 0)
{
	avcodec_close(c);
	av_free(c);
}

FILE* f = NULL;
fopen_s(&f, "test.jpg", "wb");
fwrite(avpkt->data, 1, avpkt->size, f);
fclose(f);
```

The reference code is here: 
[Nicole2442/ffmpeg_usage](https://github.com/Nicole2442/ffmpeg_usage)