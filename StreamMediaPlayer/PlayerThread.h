//PlayerThread.h:

#pragma once

#include <process.h>
#include "Databuffer.h"
#include "rtsp.h"
#include "RtpUnpacker.h"
#include "H264Decoder.h"
#include "D3DRenderer.h"
#include "StreamMediaPlayer.h"
#include "DIR\CVDirAPI.h"
#pragma comment(lib, "DIR\\CVDirAPI.lib")

typedef struct
{
	int direction;
	void* pfn;
	void* pParam;
}THREAD_DIRECTION_PARAM;

class PlayerThread
{
public:
	PlayerThread();
	~PlayerThread();

public:
	int  Start(const char* url, const char* username, const char* password, HWND hRendererWnd);
	void Stop();
	void SetRecord(bool bRecord = true, const char* pathname = NULL);
	void SnapShot(const char* path);
	void SetCallback_direction(CALLBACK_DIRECTION pfn, void* pParam) { m_pcb_direction = pfn; m_pParam_direction = pParam; }
	void SetDirArea(const RECT& rc);

protected:
	friend void thread_getstream(void* pParam);
	friend void thread_unpack(void* pParam);
	friend void thread_decode(void* pParam);
	friend void thread_render(void* pParam);

	friend void thread_direction(void* pParam);

	void Record(FrameH264Buffer* pFrameH264);

private:
	bool m_bWorking;
	bool m_bWorking_getstream;
	bool m_bPlayStatusChanged;

	HANDLE m_hThread_getstream;
	HANDLE m_hThread_unpack;
	HANDLE m_hThread_decode;
	HANDLE m_hThread_render;

	CRtspClient* m_pRtspClient;
	RtpUnpacker* m_pRtpUnpacker;
	H264Decoder* m_pH264Decoder;
	D3DRenderer* m_pD3DRenderer;

	RtpBufferList*       m_pRtpList;
	FrameH264BufferList* m_pFrameH264List;
	FrameYUVBufferList*  m_pFrameYUVList;

	//抓拍
	bool m_bSnapShot;
	char m_szSnapShot[MAX_PATH];

	//录像
	bool m_bRecord;
	char m_szRecord[MAX_PATH];
	SYSTEMTIME m_TimeRecord;
	FILE* m_fpRecord;
	bool m_bNeedIFrame;

	AJB_DIR_Handle m_hDIR; // 方位判断句柄

	//方位回调函数
	void* m_pcb_direction;
	void* m_pParam_direction;
};