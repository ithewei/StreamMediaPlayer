//PlayerThread.cpp:

#include "stdafx.h"
#include "PlayerThread.h"

PlayerThread::PlayerThread()
: m_bWorking(false)
, m_bWorking_getstream(false)
, m_bPlayStatusChanged(false)
, m_hThread_getstream(NULL)
, m_hThread_unpack(NULL)
, m_hThread_decode(NULL)
, m_hThread_render(NULL)
, m_pRtspClient(NULL)
, m_pRtpUnpacker(NULL)
, m_pH264Decoder(NULL)
, m_pD3DRenderer(NULL)
, m_bSnapShot(false)
, m_bRecord(false)
, m_fpRecord(NULL)
, m_bNeedIFrame(true)
, m_hDIR(NULL)
, m_pcb_direction(NULL)
, m_pParam_direction(NULL)
{
	m_pRtpList       = new RtpBufferList;
	m_pFrameH264List = new FrameH264BufferList;
	m_pFrameYUVList  = new FrameYUVBufferList;

	strcpy(m_szRecord,"F:\\Record"); //默认录像路径
	::CreateDirectoryA(m_szRecord,NULL);

	m_hDIR = AJB_Dir_InitSys();
}

PlayerThread::~PlayerThread()
{
	Stop();

	if (m_pRtpList)
	{
		delete m_pRtpList;
		m_pRtpList = NULL;
	}
	if (m_pFrameH264List)
	{
		delete m_pFrameH264List;
		m_pFrameH264List = NULL;
	}
	if (m_pFrameYUVList)
	{
		delete m_pFrameYUVList;
		m_pFrameYUVList = NULL;
	}

	if (m_hDIR)
	{
		AJB_Dir_ReleaseSys(m_hDIR);
		m_hDIR = NULL;
	}
}

int PlayerThread::Start(const char* url,const char* username,const char* password, HWND hRendererWnd)
{
	if (!url)
		return -1;

	if (!::IsWindow(hRendererWnd))
		return -2;

	Stop();

	m_pRtspClient = new CRtspClient;
	int ret = m_pRtspClient->ConnectSrv(url);
	if (ret < 0)
	{
		if (ret == -3)
		{
			OutputDebugStringA("IPC连接失败");
			return -3;
		}
		else
		{
			OutputDebugStringA("IPC资源路径有误");
			return -4;
		}
	}
	else if (ret == 1)
	{
		if (!m_pRtspClient->Authorize(username, password))
		{
			OutputDebugStringA("IPC认证失败");
			return -5;
		}
	}

	m_pRtpUnpacker = new RtpUnpacker;
	m_pH264Decoder = new H264Decoder;
	m_pD3DRenderer = new D3DRenderer;
	m_pD3DRenderer->SetRendererWnd(hRendererWnd);
	
	m_bWorking = true;
	m_bWorking_getstream = true;
	m_bPlayStatusChanged = true;
	m_hThread_getstream = (HANDLE)_beginthread(thread_getstream, 0, this);
	m_hThread_unpack = (HANDLE)_beginthread(thread_unpack, 0, this);
	m_hThread_decode = (HANDLE)_beginthread(thread_decode, 0, this);
	m_hThread_render = (HANDLE)_beginthread(thread_render, 0, this);

	return 0;
}

void PlayerThread::Stop()
{
	m_bWorking_getstream = false;
	WaitForSingleObject(m_hThread_getstream, INFINITE);

	m_bWorking = false;
	WaitForSingleObject(m_hThread_unpack, INFINITE);
	WaitForSingleObject(m_hThread_decode, INFINITE);
	WaitForSingleObject(m_hThread_render, INFINITE);

	if (m_pRtspClient)
	{
		delete m_pRtspClient;
		m_pRtspClient = NULL;
	}
	if (m_pRtpUnpacker)
	{
		delete m_pRtpUnpacker;
		m_pRtpUnpacker = NULL;
	}
	if (m_pH264Decoder)
	{
		delete m_pH264Decoder;
		m_pH264Decoder = NULL;
	}
	if (m_pD3DRenderer)
	{
		delete m_pD3DRenderer;
		m_pD3DRenderer = NULL;
	}
	if (m_fpRecord)
	{
		fclose(m_fpRecord);
		m_fpRecord = NULL;
	}

	m_pRtpList->RemoveAll();
	m_pFrameH264List->RemoveAll();
	m_pFrameYUVList->RemoveAll();
}

void thread_getstream(void* pParam)
{
	PlayerThread* pObj = (PlayerThread*)pParam;

	//开始获取流
	if (!pObj->m_pRtspClient->BeginStream())
	{
		OutputDebugStringA("获取流失败");
		return;
	}

	char* pData = (char*)malloc(1024*100);

	while (pObj->m_bWorking_getstream)
	{
		int getbytes = pObj->m_pRtspClient->GetStreamData(pData, 1024*100);
		if (getbytes <= 0)
		{
			continue;
		}
		pObj->m_pRtpUnpacker->QueueStream(pData, getbytes, pObj->m_pRtpList);//该函数中有对m_pRtpList的FindFreeSpace等待操作，结束各线程时，注意先结束该线程
	}

	//停止获取流
	pObj->m_pRtspClient->EndStream();

	if (pData)
	{
		free(pData);
		pData = NULL;
	}

	pObj->m_hThread_getstream = 0;
}

void thread_unpack(void* pParam)
{
	PlayerThread* pObj = (PlayerThread*)pParam;

	RtpBuffer* pRtp = NULL;
	FrameH264Buffer* pFrameH264 = NULL;
	int frame_type = 0;

	while (pObj->m_bWorking)
	{
		pRtp = pObj->m_pRtpList->GetHead();
		if (!pRtp)
		{
			Sleep(1);
			continue;
		}

		pFrameH264 = pObj->m_pFrameH264List->FindFreeSpace();
		if (!pFrameH264)
		{
			Sleep(1);
			continue;
		}
		
		if (pObj->m_pRtpUnpacker->ParseRtpPacket(pRtp, pFrameH264) == 0)
		{
			pObj->m_pFrameH264List->PushBack(pFrameH264);
			if (pObj->m_bRecord)
			{
				if ( !(pObj->m_bNeedIFrame && (pFrameH264->frame_type != 0x05)) )
				{
					pObj->Record(pFrameH264);
					pObj->m_bNeedIFrame = false;
				}
			}
		}

		pObj->m_pRtpList->RemoveHead();
	}

	pObj->m_hThread_unpack = 0;
}

void thread_decode(void* pParam)
{
	PlayerThread* pObj = (PlayerThread*)pParam;

	FrameH264Buffer* pFrameH264 = NULL;
	FrameYUVBuffer* pFrameYUV = NULL;

	while (pObj->m_bWorking)
	{
		pFrameH264 = pObj->m_pFrameH264List->GetHead();
		if (!pFrameH264)
		{
			Sleep(1);
			continue;
		}

		pFrameYUV = pObj->m_pFrameYUVList->FindFreeSpace();
		if (!pFrameYUV)
		{
			Sleep(1);
			continue;
		}

		if (pObj->m_pH264Decoder->Decode(pFrameH264, pFrameYUV))
		{
			pObj->m_pFrameYUVList->PushBack(pFrameYUV);
		}

		pObj->m_pFrameH264List->RemoveHead();
	}

	pObj->m_hThread_decode = 0;
}

void thread_render(void* pParam)
{
	PlayerThread* pObj = (PlayerThread*)pParam;

	FrameYUVBuffer* pFrameYUV = NULL;

	unsigned long long tick_base = 0;
	unsigned long long tick_cur = 0;
	unsigned long long tick_span = 0;

	unsigned long long pts_base = 0;
	unsigned long long pts_cur = 0;
	unsigned long long pts_span = 0;

	unsigned char* pDataBGR = (unsigned char*)malloc(1920*1280*3);
	int direction = 0;

	while (pObj->m_bWorking)
	{	
		pFrameYUV = pObj->m_pFrameYUVList->GetHead();
		if (!pFrameYUV)
		{
			Sleep(1);
			continue;
		}

		if (pObj->m_bPlayStatusChanged)
		{
			//设置基准时间
			tick_base = ::GetTickCount64();
			tick_cur = tick_base;

			pts_base = pFrameYUV->ullPts;
			pts_cur = pts_base;

			pObj->m_bPlayStatusChanged = false;
		}
		else
		{
			tick_cur = ::GetTickCount64();
			tick_span = tick_cur - tick_base;

			pts_cur = pFrameYUV->ullPts;
			if (pts_cur > pts_base)
			{
				pts_span = (pts_cur - pts_base)/90;

				//卡顿处理：丢掉延时的帧(确保实时播放)并刷新基准时间
				if (tick_span > pts_span)
				{
					pObj->m_pFrameYUVList->RemoveAll();
					pObj->m_bPlayStatusChanged = true;
					continue;
				}

				while (tick_span < pts_span)
				{
					Sleep(1);
					tick_cur = ::GetTickCount64();
					tick_span = tick_cur - tick_base;
				}			
			}
			else
			{
				//pts无效处理：刷新基准时间
				pObj->m_bPlayStatusChanged = true;
				continue;
			}
		}

		//渲染
		pObj->m_pD3DRenderer->RenderSample(pFrameYUV);

		//抓拍
		if (pObj->m_bSnapShot)
		{
			pObj->m_bSnapShot = false;
			SaveBMP(pFrameYUV, pObj->m_szSnapShot);
		}

		//方位判断
		YUV420PToBGR24(pFrameYUV, pDataBGR);
		direction = AJB_Dir_ProcessFrame(pObj->m_hDIR, pDataBGR, pFrameYUV->nWidth, pFrameYUV->nHeight);
		if (direction != 0)
		{
			//有目标移出区域处理
			if (pObj->m_pcb_direction)
			{
				//另起线程执行回调函数，防止阻塞
				THREAD_DIRECTION_PARAM* pParam = new THREAD_DIRECTION_PARAM;
				pParam->direction = direction;
				pParam->pfn = pObj->m_pcb_direction;
				pParam->pParam = pObj->m_pParam_direction;
				_beginthread(thread_direction, 0, pParam);
			}
		}

		pObj->m_pFrameYUVList->RemoveHead();
	}

	//停止播放后刷新窗口
	HWND hWndRender = pObj->m_pD3DRenderer->GetRendererWnd();
	if (::IsWindow(hWndRender))
	{
		RECT rect;
		::GetClientRect(hWndRender, &rect);
		//::RedrawWindow(hWndRender, &rect, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		::InvalidateRect(hWndRender, &rect, true);
	}

	if (pDataBGR)
	{
		free(pDataBGR);
		pDataBGR = NULL;
	}

	pObj->m_hThread_render = 0;
}

void thread_direction(void* pParam)
{
	THREAD_DIRECTION_PARAM* pTDP = (THREAD_DIRECTION_PARAM*)pParam;

	(CALLBACK_DIRECTION(pTDP->pfn))(pTDP->direction, pTDP->pParam);

	delete pTDP;
}


void PlayerThread::SnapShot(const char* path)
{
	if (!path || strlen(path) == 0)
		return;

	m_bSnapShot = true;
	strcpy(m_szSnapShot, path);
}

void PlayerThread::SetRecord(bool bRecord, const char* pathname)
{
	if (pathname)
	{
		strcpy(m_szRecord, pathname);
	}
	m_bRecord = bRecord;

	if (bRecord == false)
	{
		if (m_fpRecord)
		{
			fclose(m_fpRecord);
			m_fpRecord = NULL;
		}
	}
}

void PlayerThread::Record(FrameH264Buffer* pFrameH264)
{
	//一个小时新建录像文件（避免文件过大）
	SYSTEMTIME st;
	::GetLocalTime(&st);
	if ( st.wMinute == m_TimeRecord.wMinute && (st.wHour - m_TimeRecord.wHour) == 1 )
	{			
		if (m_fpRecord)
		{
			fclose(m_fpRecord);
			m_fpRecord = NULL;
		}
	}

	if ( !m_fpRecord )
	{
		char filepath[MAX_PATH];
		sprintf(filepath, "%s\\%04d%02d%02d_%02d%02d%02d.h264", m_szRecord, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		m_fpRecord = fopen(filepath, "wb");
		fwrite(m_pRtpUnpacker->m_pSPSPPS->GetData(), 1, m_pRtpUnpacker->m_pSPSPPS->GetLength(), m_fpRecord); //H264文件头加上SPS、PPS信息
		m_bNeedIFrame = true; // I帧开始

		memcpy(&m_TimeRecord, &st, sizeof(SYSTEMTIME)); // 记录下创建文件时间
	}

	fwrite(pFrameH264->GetData(), 1, pFrameH264->GetLength(), m_fpRecord);
}

void PlayerThread::SetDirArea(const RECT& rc)
{
	if (!m_pD3DRenderer)
		return;

	RECT rcDst;
	m_pD3DRenderer->TransRect(rc, rcDst);

	stRect area;
	area.left = rcDst.left;
	area.top = rcDst.top;
	area.right = rcDst.right;
	area.down = rcDst.bottom;
	AJB_Dir_InitParams(m_hDIR, area);
}