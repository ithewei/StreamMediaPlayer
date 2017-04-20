//socket.cpp:

#include "stdafx.h"
#include "socket.h"

////////////////////////////////////////////////////////////////////////////////
class CWinSockInitializer
{
public:
	CWinSockInitializer()
	{
		WORD wVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		::WSAStartup(wVersion, &wsaData);
	}
	~CWinSockInitializer()
	{
		::WSACleanup();
	}
};
CWinSockInitializer WinSockInitializer;

////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
CTcpSelectSocket::CTcpSelectSocket()
{
	m_socket = OpenSocket();
}

CTcpSelectSocket::CTcpSelectSocket(SOCKET sock)
: m_socket(sock)
{
	
}

CTcpSelectSocket::~CTcpSelectSocket()
{
	CloseSocket();
}

bool CTcpSelectSocket::Connect(const char* ip, int port)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (::connect(m_socket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
		return true;
		
	return true;
}

bool CTcpSelectSocket::Bind(int port)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (::bind(m_socket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
		return false;

	if (::listen(m_socket, 64) == SOCKET_ERROR)
		return false;

	return true;
}

SOCKET CTcpSelectSocket::Accept(SOCKADDR_IN* addr)
{
	if (!IsOpen())
		return INVALID_SOCKET;

	int len = sizeof(SOCKADDR);

	return ::accept(m_socket, (SOCKADDR*)addr, &len);
}

int CTcpSelectSocket::Read(char* buf, int len)
{
	if (!buf || len == 0)
		return -1;

	if (!IsOpen())
		return -2;

	if (Select(SELECT_MODE_READ) != SELECT_OK)
		return -3;

	int readbytes = ::recv(m_socket, buf, len, 0);
	if (readbytes < 0)
		return -4;

	return readbytes;
}

int CTcpSelectSocket::Write(const char* buf, int len)
{
	if (!buf || len == 0)
		return -1;

	if (!IsOpen())
		return -2;

	if (Select(SELECT_MODE_WRITE) != SELECT_OK)
		return -3;

	int writebytes = ::send(m_socket, buf, len, 0);
	if (writebytes < 0)
		return -4;

	return writebytes;
}

SOCKET CTcpSelectSocket::OpenSocket()
{
	SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//unsigned long ul = 1;
	//ioctlsocket(sock, FIONBIO, &ul); //设置为非阻塞模式

	return sock;
}

void CTcpSelectSocket::CloseSocket()
{
	if (m_socket != INVALID_SOCKET)
	{
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

int CTcpSelectSocket::Select(SELECT_MODE mode, UINT nTimeout /* = 3000 */)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_socket, &fds);

	fd_set* readfds = NULL;
	fd_set* writefds = NULL;

	if (mode == SELECT_MODE_READ)
		readfds = &fds;
	else if (mode == SELECT_MODE_WRITE)
		writefds = &fds;

	timeval timeout;
	timeout.tv_sec = nTimeout / 1000;
	timeout.tv_usec = nTimeout % 1000 * 1000;

	int ret = ::select(m_socket + 1, readfds, writefds, NULL, &timeout);
	if (ret == SOCKET_ERROR)
		return SELECT_ERROR;
	else if (ret == 0)
		return SELECT_TIMEOUT;

	return SELECT_OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////
CIntractSocket::CIntractSocket()
{

}

CIntractSocket::CIntractSocket(SOCKET sock)
: CTcpSelectSocket(sock)
{

}

CIntractSocket::~CIntractSocket()
{

}

bool CIntractSocket::Connect(const char* ip, int port)
{
	return __super::Connect(ip, port);
}

int CIntractSocket::Read(char* buf, int len)
{
	return __super::Read(buf, len);
}

int CIntractSocket::Write(const char* buf, int len)
{
	return __super::Write(buf, len);
}
///////////////////////////////////////////////////////////////////////////////

CListenSocket::CListenSocket()
{

}

CListenSocket::~CListenSocket()
{

}

bool CListenSocket::Bind(int port)
{
	return __super::Bind(port);
}

CIntractSocket CListenSocket::Accept(SOCKADDR_IN* addr)
{
	return CIntractSocket(__super::Accept(addr));
}
//////////////////////////////////////////////////////////////////////////////////////