//rtsp.cpp:

#include "stdafx.h"
#include "rtsp.h"
#include "MD5/MD5.h"

CRtspClient::CRtspClient()
: m_pIntract(NULL)
, m_nSeq(1)
, m_bNeedAuth(false)
, m_bWorking(false)
, m_hThread_keepalive(NULL)
{
	m_pIntract = new CIntractSocket;

	memset(m_szName, 0, sizeof(m_szName));
	memset(m_szPswd, 0, sizeof(m_szPswd));
	memset(m_szRealm, 0, sizeof(m_szRealm));
	memset(m_szNonce, 0, sizeof(m_szNonce));
	memset(m_szTrack, 0, sizeof(m_szTrack));
	memset(m_szSession, 0, sizeof(m_szSession));
}

CRtspClient::~CRtspClient()
{
	if (m_pIntract)
	{
		delete m_pIntract;
		m_pIntract = NULL;
	}
}

int CRtspClient::ConnectSrv(const char* url)
{
	if (!url)
		return -1;

	if (ParseUrl(url, m_szIp, m_nPort) != 0)
		return -2;

	strcpy(m_szUrl, url);
	
	if (!m_pIntract->Connect(m_szIp, m_nPort))
		return -3;
	
	//发一次DESCRIBE请求判断是否需要认证
	if (SendRequest(RTSP_REQUEST_TYPE_DESCRIBE) != 0)
		return -4;

	if (RecvResponse(RTSP_REQUEST_TYPE_DESCRIBE) != 0)
		return -5;

	if (m_bNeedAuth)
		return 1; // 需要认证
	else
		return 0;
}

//bool CRtspClient::ConnectSrv(const char* ip, int port)
//{
//	if (!ip)
//		return false;
//
//	if (!m_pIntract->Connect(ip, port))
//		return false;
//
//	strcpy(m_szIp, ip);
//	m_nPort = port;
//	sprintf(m_szUrl, "rtsp://%s:%d/PSIA/Streaming/channels/101/", m_szIp, m_nPort);
//
//	return true;
//}

bool CRtspClient::Authorize(const char* username, const char* pswd)
{
	if (!username || !pswd)
		return false;

	strcpy(m_szName, username);
	strcpy(m_szPswd, pswd);

	if (SendRequest(RTSP_REQUEST_TYPE_DESCRIBE) != 0)
		return false;

	if (RecvResponse(RTSP_REQUEST_TYPE_DESCRIBE) != 0)
		return false;

	return true;
}

bool CRtspClient::BeginStream()
{
	if (SendRequest(RTSP_REQUEST_TYPE_SETUP) != 0)
		return false;

	if (RecvResponse(RTSP_REQUEST_TYPE_SETUP) != 0)
		return false;

	if (SendRequest(RTSP_REQUEST_TYPE_PLAY) != 0)
		return false;

	if (RecvResponse(RTSP_REQUEST_TYPE_PLAY) != 0)
		return false;

	m_bWorking = true;
	m_hThread_keepalive = (HANDLE)_beginthread(thread_keepalive, 0, this);

	return true;
}

bool CRtspClient::EndStream()
{
	m_bWorking = false;
	WaitForSingleObject(m_hThread_keepalive, INFINITE);

	if (SendRequest(RTSP_REQUEST_TYPE_TEARDOWN) != 0)
		return false;

	return true;
}

void thread_keepalive(void* pParam)
{
	CRtspClient* pObj = (CRtspClient*)pParam;

	int cnt = 0;

	while (pObj->m_bWorking)
	{
		if (cnt == 1000)
		{
			cnt = 0;
			pObj->SendRequest(RTSP_REQUEST_TYPE_OPTIONS);
		}

		Sleep(1);
		cnt++;
	}

	pObj->m_hThread_keepalive = 0;
}

int CRtspClient::GetStreamData(char* buf, int len)
{
	if (!buf || len == 0)
		return -1;

	int readbytes = m_pIntract->Read(buf, len);
	if (readbytes < 0)
		return -2;

	return readbytes;
}

int CRtspClient::SendRequest(RTSP_REQUEST_TYPE eType)
{
	string strRequest;

	char szMethod[16];

	switch (eType)
	{
	case RTSP_REQUEST_TYPE_OPTIONS:
		strcpy(szMethod, "OPTIONS");
		break;
	case RTSP_REQUEST_TYPE_DESCRIBE:
		strcpy(szMethod, "DESCRIBE");
		break;
	case RTSP_REQUEST_TYPE_SETUP:
		strcpy(szMethod, "SETUP");
		break;
	case RTSP_REQUEST_TYPE_PLAY:
		strcpy(szMethod, "PLAY");
		break;
	case RTSP_REQUEST_TYPE_PAUSE:
		strcpy(szMethod, "PAUSE");
		break;
	case RTSP_REQUEST_TYPE_TEARDOWN:
		strcpy(szMethod, "TEARDOWN");
		break;
	default: 
		break;
	}

	//方法
	strRequest = szMethod;
	strRequest += " ";
	strRequest += m_szUrl;
	if (eType == RTSP_REQUEST_TYPE_SETUP)
	{
		strRequest += "/";
		strRequest += m_szTrack;
	}
	strRequest += " ";
	strRequest += "RTSP/1.0";
	strRequest += "\r\n";

	//序列号
	strRequest += "CSeq: ";
	char szSeq[16];
	sprintf(szSeq, "%d", m_nSeq++);
	strRequest += szSeq;
	strRequest += "\r\n";

	//摘要认证
	if (m_bNeedAuth && eType != RTSP_REQUEST_TYPE_OPTIONS)
	{
		HASHHEX hash;

		getNewDigestResponse(m_szNonce, m_szName, m_szRealm, m_szPswd, szMethod, m_szUrl, hash);
		char szAuth[256];
		sprintf(szAuth, "Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"", m_szName, m_szRealm, m_szNonce, m_szUrl, hash);
		strRequest += szAuth;
		strRequest += "\r\n";
	}

	//其它
	switch (eType)
	{
	case RTSP_REQUEST_TYPE_DESCRIBE:
		strRequest += "Accept: application/sdp"; // 获取sdp描述
		strRequest += "\r\n";
		break;
	case RTSP_REQUEST_TYPE_SETUP:
		strRequest += "Transport: RTP/AVP/TCP;unicast;interleaved=0-1"; //TCP传输
		strRequest += "\r\n";
		break;
	case RTSP_REQUEST_TYPE_PLAY:
		strRequest += "Session: ";
		strRequest += m_szSession;
		strRequest += "\r\n";

		strRequest += "Range: npt=0.000-"; // 时长范围从0到无穷
		strRequest += "\r\n";
		break;
	case RTSP_REQUEST_TYPE_TEARDOWN:
		strRequest += "Session: ";
		strRequest += m_szSession;
		strRequest += "\r\n";
	default:
		break;
	}

	strRequest += "\r\n";

	int writebytes = m_pIntract->Write(strRequest.c_str(), strRequest.length());
	if (writebytes != strRequest.length())
	{
		return -1;
	}

	return 0;
}

int CRtspClient::RecvResponse(RTSP_REQUEST_TYPE eType)
{
	char pszResponse[2048] = {0};
	int readbytes = m_pIntract->Read(pszResponse, 2048);
	if (readbytes <= 0)
	{
		return -1;
	}

	map<string, string> mapHeader;
	ParseHeader(pszResponse, mapHeader);

	switch (eType)
	{
	case RTSP_REQUEST_TYPE_OPTIONS:
		{
			if (!mapHeader["Status"].empty())
			{
				if (mapHeader["Status"].find("200 OK") == string::npos)
				{
					free(pszResponse);
					return -2;
				}
			}
		}
		break;
	case RTSP_REQUEST_TYPE_DESCRIBE:
		{
			if (m_bNeedAuth)
			{
				if (mapHeader["Status"].find("200 OK") == string::npos)
				{
					//认证失败
					return -2;	
				}
			}
			else
			{
				if (!mapHeader["Status"].empty())
				{
					if (mapHeader["Status"].find("401 Unauthorized") != string::npos)
					{
						//需要认证、获取realm、nonce
						m_bNeedAuth = true;

						string strHeader(pszResponse);
						int pos = strHeader.find("realm");
						pos = strHeader.find('=',pos);
						int pos1 = strHeader.find('\"',pos);
						int pos2 = strHeader.find('\"',pos1+1);
						strcpy(m_szRealm, strHeader.substr(pos1+1,pos2-pos1-1).c_str());

						pos = strHeader.find("nonce");
						pos = strHeader.find('=',pos);
						pos1 = strHeader.find('\"',pos);
						pos2 = strHeader.find('\"',pos1+1);
						strcpy(m_szNonce, strHeader.substr(pos1+1,pos2-pos1-1).c_str());
					}
					else if (mapHeader["Status"].find("200 OK") != string::npos)
					{
						//无需认证
						m_bNeedAuth = false;
					}
					else
					{
						return -2;
					}
				}
			}

			if (mapHeader["Status"].find("200 OK") != string::npos)
			{
				// 接收SDP
				int readbytes = m_pIntract->Read(pszResponse, 2048);
				if (readbytes <= 0)
				{
					return -1;
				}

				//解析sdp、获取通道号
				string strHeader(pszResponse);
				int pos = strHeader.find("m=video");
				pos = strHeader.find("a=control",pos);
				pos = strHeader.find(':',pos);
				int pos2 = strHeader.find("\r\n",pos);
				string a = strHeader.substr(pos+1,pos2-pos-1);
				int pos3 = a.find(m_szUrl);
				if (pos3 == 0)
					a.erase(0,strlen(m_szUrl)+1);
				strcpy(m_szTrack,a.c_str());
			}
		}
		break;
	case RTSP_REQUEST_TYPE_SETUP:
		{
			if (!mapHeader["Status"].empty())
			{
				if (mapHeader["Status"].find("200 OK") == string::npos)
				{
					return -2;
				}
			}
			//获取会话号
			if (!mapHeader["Session"].empty())
			{
				strcpy(m_szSession, mapHeader["Session"].c_str());
			}
		}
		break;
	case RTSP_REQUEST_TYPE_PLAY:
		{
			if (!mapHeader["Status"].empty())
			{
				if (mapHeader["Status"].find("200 OK") == string::npos)
				{
					return -2;
				}
			}
		}
		break;
	case RTSP_REQUEST_TYPE_PAUSE:
		break;
	case RTSP_REQUEST_TYPE_TEARDOWN:
		{
			if (!mapHeader["Status"].empty())
			{
				if (mapHeader["Status"].find("200 OK") == string::npos)
				{
					return -2;
				}
			}
		}
		break;
	default:
		break;

	}

	return 0;
}

int CRtspClient::ParseHeader(const char* pHeader, map<string,string>& mapHeader)
{
	char* pEnd = NULL;
	pEnd = strstr((char*)pHeader, "\r\n\r\n");
	if (!pEnd)
		return -1;

	string strHeader;
	strHeader.append(pHeader, pEnd-pHeader+2);

	int pos1 = 0, pos2 = 0;

	if ( (pos2 = strHeader.find("\r\n")) != string::npos )
	{
		mapHeader["Status"] = strHeader.substr(0, pos2);
		strHeader.erase(0, pos2 + 2);
	}

	while ( (pos1 = strHeader.find(": ")) != string::npos )
	{
		if ( (pos2 = strHeader.find("\r\n")) != string::npos )
		{
			mapHeader[strHeader.substr(0, pos1)] = strHeader.substr(pos1 + 2, pos2 - pos1 - 2);
			strHeader.erase(0, pos2 + 2);
		}
	}

	return 0; 
}

int CRtspClient::ParseUrl(const char* url, char* ip, int& port)
{
	if (!url || strlen(url) == 0)
		return -1;

	string strUrl(url);

	int pos = strUrl.find("rtsp://");
	if (pos != 0)
		return -2;

	strUrl.erase(0, 7);

	pos = strUrl.find('/');
	if (pos != string::npos)
	{
		strUrl.erase(pos, strUrl.length() - pos);
	}

	pos = strUrl.find(':');
	if (pos != string::npos)
	{
		strcpy(ip, strUrl.substr(0, pos).c_str());
		port = atoi(strUrl.substr(pos+1, strUrl.length()-pos-1).c_str());
	}
	else
	{
		strcpy(ip, strUrl.c_str());
		port = 554; // RTSP默认端口554
	}

	return 0;
}