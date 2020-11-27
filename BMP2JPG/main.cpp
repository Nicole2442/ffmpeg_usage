#include <iostream>
#include <fstream>
#include <afx.h>
#include <afunix.h>
#include "stdafx.h"
#include "FFmpegTool.h"
using namespace std;

int main()
{
	int retVal = 0;
	int IMAGE_BYTE_SIZE = 6220854;

	FFmpegTool* ffmpeg_tool = new FFmpegTool;

	int img_size = IMAGE_BYTE_SIZE;
	BYTE* buffer = NULL;
	buffer = new BYTE[img_size];

	readFromFile(buffer, img_size)

	AVPacket* avpkt;
	avpkt = av_packet_alloc();
	av_init_packet(avpkt);

	ffmpeg_tool->bmp2jpg(buffer, avpkt);

	writeToFile(avpkt->data, avpkt->size);

	system("pause");
	delete[]buffer;
	av_packet_unref(avpkt);

	return true;
}

void writeToFile(uint8_t* data, int data_size)
{
	FILE* f = NULL;
	fopen_s(&f, "myData.jpg", "wb");
	fwrite(data, 1, data_size, f);
	fclose(f);
}

void readFromFile(uint8_t* data, int data_size)
{
	FILE* f = NULL;
	fopen_s(&f, "test.bmp", "rb");
	fread(data, 1, data_size, f);
	fclose(f);
}



