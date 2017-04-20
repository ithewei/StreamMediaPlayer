
#pragma once
#ifdef IA_API
#else
#define IA_API _declspec(dllimport)
#endif
#include <vector>

typedef struct m_mat
{
	unsigned char* data;
	int rows;
	int cols;
}m_mat;

typedef struct m_Point
{
	int x;
	int y;
}m_Point;

typedef struct m_Rect
{
	int x;
	int y;
	int width;
	int height;
}m_Rect;

typedef struct m_Line
{
	m_Point pt[2];
}m_Line;

/************************************
参数：
lDetectionHandle：背景信息
srcimg：输入的当前帧图片
m_Rect ROI：感兴趣区域
double mscale:下采样倍数
int &direction:返回目标移出的方向，返回1 为左边移出， 返回2为右边移出,返回3为上端移出，返回4为下端移出，返回-1表示没有目标移出现象。
*说明:判断视野中是否有目标运动，有目标运动时返回1，否则为0。

例子：
m_Rect ROI;
ROI.x = frame.cols/4;
ROI.y =frame.rows/4;
ROI.width =frame.cols*1/2;
ROI.height =frame.rows*1/2;
bool tmpflag=getmoveDirection(lDetectionHandle,  imgdata,ROI, 2,direction);
************************************/
extern "C" IA_API bool __stdcall getmoveDirection(long lDetectionHandle, m_mat srcmat,m_Rect ROI,double mscale,int &direction);
extern "C" IA_API long __stdcall StartgetmoveDirection();
extern "C" IA_API long __stdcall StopgetmoveDirection(long lDetectionHandle);







