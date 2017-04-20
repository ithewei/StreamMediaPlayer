stdafx.h中加入如下宏定义以放FFMPEG使用出错

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif


库加载顺序
#ifdef __cplusplus  
extern "C" 
{  
#endif  
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
#include "libavutil\error.h"
#include "libswresample\swresample.h"

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
#pragma comment( lib, "ffmpeg/lib/libswresample.a")
