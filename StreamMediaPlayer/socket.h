//socket.h:

#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

///////////////////////////////////////////////////////////////////////////
typedef enum
{
	SELECT_MODE_READ = 1,
	SELECT_MODE_WRITE = 2,
}SELECT_MODE;

#define SELECT_OK      1
#define SELECT_ERROR   -1
#define SELECT_TIMEOUT -2

class CTcpSelectSocket
{
protected:
	CTcpSelectSocket();
	CTcpSelectSocket(SOCKET sock);
	virtual ~CTcpSelectSocket();

protected:
	bool Connect(const char* ip, int port); // 客户端使用
	bool Bind(int port); // 服务端使用
	SOCKET Accept(SOCKADDR_IN* addr); // 服务端使用:返回服务器与客户端通信的套接字

	int Read(char* buf, int len);
	int Write(const char* buf, int len);

private:
	SOCKET OpenSocket();
	void   CloseSocket();
	bool   IsOpen() { return (m_socket != INVALID_SOCKET); }
	int    Select(SELECT_MODE mode, UINT nTimeout = 3000);

private:
	SOCKET m_socket;
};

/////////////////////////////////////////////////////////////////////////////////////////
class CIntractSocket : public CTcpSelectSocket
{
public:
	CIntractSocket();
	CIntractSocket(SOCKET sock);
	~CIntractSocket();

public:
	bool Connect(const char* ip, int port);
	int  Read(char* buf, int len);
	int  Write(const char* buf, int len);
};

class CListenSocket : public CTcpSelectSocket
{
public:
	CListenSocket();
	~CListenSocket();

public:
	bool Bind(int port);
	CIntractSocket Accept(SOCKADDR_IN* addr);
};
///////////////////////////////////////////////////////////////////////////////////////////