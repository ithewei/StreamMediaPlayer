// StreamMediaPlayer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "StreamMediaPlayer.h"
#include "PlayerThread.h"

AJB_SMP_API long AJB_SMP_GetHandle()
{
	PlayerThread* pObj = new PlayerThread;
	if (!pObj)
		return NULL;

	return (long)pObj;
}

AJB_SMP_API void AJB_SMP_CloseHandle(long handle)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return;

	delete pObj;
}

AJB_SMP_API int AJB_SMP_Play(long handle, const char* url, const char* username, const char* pswd, HWND hWnd)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return -1;

	int ret = pObj->Start(url,username,pswd,hWnd);
	if (ret == -3)
		return ERR_IPC_CONNECT;
	else if (ret == -4)
		return ERR_IPC_URL;
	else if (ret == -5)
		return ERR_IPC_AUTHORIZE;

	return 0;
}

AJB_SMP_API int AJB_SMP_Stop(long handle)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return -1;

	pObj->Stop();

	return 0;
}

AJB_SMP_API int AJB_SMP_SnapShot(long handle, const char* path)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return -1;

	pObj->SnapShot(path);

	return 0;
}

AJB_SMP_API int AJB_SMP_Record(long handle, bool bRecord, const char* pathname)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return -1;

	pObj->SetRecord(bRecord, pathname);

	return 0;
}

AJB_SMP_API int AJB_SMP_SetCallback_direction(int handle, CALLBACK_DIRECTION pfn, void* pParam)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return -1;

	pObj->SetCallback_direction(pfn, pParam);

	return 0;
}

AJB_SMP_API int AJB_SMP_SetDirArea(long handle, const RECT& rc)
{
	PlayerThread* pObj = (PlayerThread*)handle;
	if (!pObj)
		return -1;

	pObj->SetDirArea(rc);

	return 0;
}


