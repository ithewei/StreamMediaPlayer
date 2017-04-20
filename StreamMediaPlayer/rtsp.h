//rtsp.h:

#pragma once

#include "socket.h"
#include <string>
#include <map>
#include <process.h>
using namespace std;

typedef enum
{
	RTSP_REQUEST_TYPE_OPTIONS = 0,
	RTSP_REQUEST_TYPE_DESCRIBE,
	RTSP_REQUEST_TYPE_SETUP,
	RTSP_REQUEST_TYPE_PLAY,
	RTSP_REQUEST_TYPE_PAUSE,
	RTSP_REQUEST_TYPE_TEARDOWN,
}RTSP_REQUEST_TYPE;

class CRtspClient
{
public:
	CRtspClient();
	~CRtspClient();

public:
	int ConnectSrv(const char* url);
	//bool ConnectSrv(const char* ip, int port);
	bool Authorize(const char* username, const char* pswd);
	bool BeginStream();
	bool EndStream();
	int GetStreamData(char* buf, int len);

protected:
	int SendRequest(RTSP_REQUEST_TYPE eType);
	int RecvResponse(RTSP_REQUEST_TYPE eType);
	int ParseHeader(const char* pHeader, map<string,string>& mapHeader);
	int ParseUrl(const char* url, char* ip, int& port);

	friend void thread_keepalive(void* pParam); //每1s发送一个心跳包

private:
	CIntractSocket* m_pIntract; //交互套接字

	int m_nSeq; //交互序列号
	bool m_bNeedAuth;//认证标志

	bool m_bWorking;
	HANDLE m_hThread_keepalive;

	//服务器IP地址、端口号、资源路径
	char m_szIp[16];
	int m_nPort; 
	char m_szUrl[128]; 

	//认证所需用户名、密码
	char m_szName[32];
	char m_szPswd[32];

	//DESCRIBE方法返回的随机数、域名，用于认证；通道号用于建立连接
	char m_szNonce[64];
	char m_szRealm[64];
	char m_szTrack[64];

	//SETUP方法返回的会话号
	char m_szSession[64];
};