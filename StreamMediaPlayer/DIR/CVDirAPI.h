
#pragma once
#pragma warning(disable: 4251)
#pragma warning(disable: 4273)

#define AJB_CV_DIR_API_DLL_LIBRARY

#ifdef AJB_CV_DIR_API_DLL_LIBRARY
#define AJB_IVS_API _declspec(dllexport)
#else
#define AJB_IVS_API _declspec(dllimport)
#endif

//////////////////////////////////////////////////////////////////////////////////////
enum emDIRECTION
{
	DIR_UNKNOWN		= 0,  
	DIR_LEFT	    = 1,
	DIR_RIGHT	    = 2,
	DIR_TOP			= 3,
	DIR_BOTTOM		=4
};

// Rect结构体
typedef struct stRect
{
	int left;
	int top;
	int right;
	int down;
	stRect()
	{
		left = top = right = down =0;
	}
	stRect(int _left, int _top, int _right, int _down)
	{
		left = _left;
		top = _top;
		right = _right;
		down = _down;
	}
	stRect operator=(const stRect& r)
	{
		left = r.left;
		right = r.right;
		top = r.top;
		down = r.down;
		return(*this);
	}
}stRect;


typedef void*  AJB_DIR_Handle;                   //算法系统全局句柄

/*****************************************************************************
*
*Description:
*
*		初始化算法系统。
*
*Parameters:
*
*		IN    areaParam  异常事件分析算法的参数。
*
*       IN    algoParam  运动目标检测、跟踪与分析算法的参数。
*
*Return:
*
*       算法系统全局句柄。
*
******************************************************************************/

AJB_DIR_Handle AJB_IVS_API AJB_Dir_InitSys();


/*****************************************************************************
*
*Description:
*
*		分析处理每一帧视频图像，输出处理结果。
*
*Parameters:
*
*		IN	  IVS_Handle	   算法系统全局句柄。
*
*		IN	  box			   输入目标区域。
*
******************************************************************************/
void AJB_IVS_API  AJB_Dir_InitParams(const AJB_DIR_Handle IVS_Handle, const stRect& box);


/*****************************************************************************
*
*Description:
*
*		分析处理每一帧视频图像，输出处理结果。
*
*Parameters:
*
*		IN	  IVS_Handle	   算法系统全局句柄。
*
*       IN    pColorImage      输入为24位BGR彩色图像数据，算法会利用彩色图像信息对背景进行建模，
*                              此输入图像数据内存的分辨率必须与算法初始化时（调用函数AVT_IVSInitSys）参数AnalyseParam中设置的输入视频图像的分辨率一致。
*		IN	  width			   图像宽度。
*
*		IN	  height		   图像高度。 
*
*Return:
*		目标消失方向
*       0 ===>> 忽略（目标在场景内）    1 ===>> 从左侧消失    2===>>从右侧消失
*       3 ===>> 从上端消失				4 ===>> 从下面消失
*
******************************************************************************/

int AJB_IVS_API  AJB_Dir_ProcessFrame(const AJB_DIR_Handle IVS_Handle, const BYTE* pColorImage, int width,  int height);


/*****************************************************************************
*
*Description:
*
*		释放算法系统的所有内存。
*
*Parameters:
*
*		IN	  IVS_Handle	       算法系统全局句柄。	
*
******************************************************************************/
void AJB_IVS_API  AJB_Dir_ReleaseSys(AJB_DIR_Handle IVS_Handle);