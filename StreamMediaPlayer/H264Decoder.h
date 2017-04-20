//H264Decoder.h:

#pragma once

#include "Databuffer.h"

///////////////////////////////ffmpeg¾²Ì¬¿âµ÷ÓÃ//////////////////////////////////////////////////////////////////////////////
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifdef __cplusplus  
extern "C" 
{  
#endif  

#include "libavcodec\avcodec.h"
#include "libswscale\swscale.h"
#include "libavformat\avformat.h"
#include "libavutil\error.h"

#ifdef __cplusplus  
}  
#endif  

#pragma comment( lib, "ffmpeg/lib/libgcc.a")  
#pragma comment( lib, "ffmpeg/lib/libmingwex.a")  
#pragma comment( lib, "ffmpeg/lib/libcoldname.a")  
#pragma comment( lib, "ffmpeg/lib/libavcodec.a")  
#pragma comment( lib, "ffmpeg/lib/libavformat.a")  
#pragma comment( lib, "ffmpeg/lib/libavutil.a")  
#pragma comment( lib, "ffmpeg/lib/libswscale.a")  
#pragma comment( lib, "ffmpeg/lib/libz.a")  
#pragma comment( lib, "ffmpeg/lib/libfaac.a")  
#pragma comment( lib, "ffmpeg/lib/libgsm.a")  
#pragma comment( lib, "ffmpeg/lib/libmp3lame.a")  
#pragma comment( lib, "ffmpeg/lib/libogg.a")  
#pragma comment( lib, "ffmpeg/lib/libspeex.a")  
#pragma comment( lib, "ffmpeg/lib/libtheora.a")  
#pragma comment( lib, "ffmpeg/lib/libvorbis.a")  
#pragma comment( lib, "ffmpeg/lib/libvorbisenc.a")  
#pragma comment( lib, "ffmpeg/lib/libx264.a")  
#pragma comment( lib, "ffmpeg/lib/xvidcore.a")
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class H264Decoder
{
public:
	H264Decoder();
	~H264Decoder();

public:
	bool Decode(FrameH264Buffer* pFrameH264, FrameYUVBuffer* pFrameYUV);

protected:
	bool OpenDecoder();
	void CloseDecoder();

private:
	AVCodecContext* m_pCodecCtx;
	AVFrame* pFrame;
};

bool YUV420PToBGR24(FrameYUVBuffer* pFrameYUV, void* pDataBGR);
bool SaveBMP(FrameYUVBuffer* pFrameYUV, const char* path);