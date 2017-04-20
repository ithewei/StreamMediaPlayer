//H264Decoder.cpp:

#include "stdafx.h"
#include <assert.h>
#include "H264Decoder.h"
 
////////////////////////////////////////////////////////////////////////////////////////

H264Decoder::H264Decoder()
: m_pCodecCtx(NULL)
{
	avcodec_register_all();
	bool hr = OpenDecoder();
	assert(hr==true);
}

H264Decoder::~H264Decoder()
{
	CloseDecoder();
}

bool H264Decoder::OpenDecoder()
{
	AVCodec* pCodec = avcodec_find_decoder(CODEC_ID_H264);
	if (!pCodec)
		return false;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!m_pCodecCtx)
		return false;

	if (avcodec_open2(m_pCodecCtx, pCodec, NULL) < 0)
		return false;

	return true;
}

void H264Decoder::CloseDecoder()
{
	if (m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		av_free(m_pCodecCtx);
	}
}

bool H264Decoder::Decode(FrameH264Buffer* pFrameH264, FrameYUVBuffer* pFrameYUV)
{
	if (!pFrameH264 || !pFrameYUV)
		return false;

	if (!m_pCodecCtx)
	{
		if (!OpenDecoder())
			return false;
	}

	AVFrame* pFrame = avcodec_alloc_frame();
	if (!pFrame)
		return false;

	AVPacket packet;
	av_init_packet(&packet);

	packet.size = pFrameH264->GetLength();
	packet.data = (uint8_t *)(pFrameH264->GetData());
	packet.pts = pFrameH264->ullPts;

	int got_picture = 0;

	if (avcodec_decode_video2(m_pCodecCtx, pFrame, &got_picture, &packet) < 0 || got_picture == 0)
	{
		av_free(pFrame);
		av_free_packet(&packet);
		return false;
	}
	
	pFrameYUV->WriteData(pFrame->data[0], pFrame->linesize[0]*pFrame->height);
	pFrameYUV->WriteData(pFrame->data[1], pFrame->linesize[1]*pFrame->height>>1);
	pFrameYUV->WriteData(pFrame->data[2], pFrame->linesize[2]*pFrame->height>>1);
	pFrameYUV->nWidth  = pFrame->width;
	pFrameYUV->nHeight = pFrame->height;
	pFrameYUV->nPitch  = pFrame->linesize[0];
	pFrameYUV->ullPts  = pFrame->pkt_pts;

	av_free(pFrame);
	av_free_packet(&packet);

	return true;
}

bool YUV420PToBGR24(FrameYUVBuffer* pFrameYUV, void* pDataBGR)
{
	if ( !pFrameYUV || !pDataBGR )
		return false;

	int w = pFrameYUV->nWidth;
	int h = pFrameYUV->nHeight;
	uint8_t* pDataYUV = (uint8_t*)pFrameYUV->GetData();

	//bmp图片内存从低到高需要排列B、G、R
	SwsContext* pSwsCtx = sws_getContext(w, h, PIX_FMT_YUV420P, w, h, PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	if ( !pSwsCtx )
		return false;

	AVPicture src,dst;
	src.linesize[0] = pFrameYUV->nPitch;
	src.linesize[1] = pFrameYUV->nPitch>>1;
	src.linesize[2] = src.linesize[1];

	src.data[0] = pDataYUV;
	src.data[1] = pDataYUV + pFrameYUV->nPitch * pFrameYUV->nHeight;
	src.data[2] = src.data[1] + pFrameYUV->nPitch * pFrameYUV->nHeight / 4;

	dst.data[0] = (uint8_t*)pDataBGR;
	dst.linesize[0] = w * 3;

	if ( sws_scale(pSwsCtx, src.data, src.linesize, 0, pFrameYUV->nHeight, dst.data, dst.linesize) < 0)
		return false;

	sws_freeContext(pSwsCtx);

	return true;
}

bool SaveBMP(FrameYUVBuffer* pFrameYUV, const char* path)
{
	if (!pFrameYUV || !path)
		return false;

	char* pDataBGR = (char*)malloc(1920*1280*3);
	YUV420PToBGR24(pFrameYUV, pDataBGR);

	int w = pFrameYUV->nWidth;
	int h = pFrameYUV->nHeight;
	int pitch = (w*3 + 3) >> 2 << 2;

	BITMAPFILEHEADER bmfHdr; 
	BITMAPINFOHEADER bmiHdr; 

	ZeroMemory( &bmfHdr, sizeof( bmfHdr ) );
	ZeroMemory( &bmiHdr, sizeof( bmiHdr ) );

	bmiHdr.biSize = sizeof(BITMAPINFOHEADER);
	bmiHdr.biWidth = w;
	bmiHdr.biHeight = -h;//图片以底部为0，故高度设为负值
	bmiHdr.biPlanes = 1;
	bmiHdr.biBitCount = 24;
	bmiHdr.biCompression = BI_RGB;
	bmiHdr.biSizeImage = w * h * 3;
	bmiHdr.biXPelsPerMeter = 0;
	bmiHdr.biYPelsPerMeter = 0;
	bmiHdr.biClrUsed = 0;
	bmiHdr.biClrImportant = 0;

	bmfHdr.bfType = ((WORD) ('M' << 8) | 'B');
	bmfHdr.bfSize=(DWORD)(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER) + bmiHdr.biSizeImage);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits=(DWORD)(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));

	FILE* fp = fopen(path,"wb");
	if (fp == NULL)
		return false;
	fwrite((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite((LPSTR)&bmiHdr,sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite(pDataBGR, 1, pitch * h, fp);
	fclose(fp);

	if (pDataBGR)
	{
		free(pDataBGR);
		pDataBGR = NULL;
	}

	return true;
}