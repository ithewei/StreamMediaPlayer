//RtpUnPacker.h:

#pragma once

#include "Databuffer.h"

///////////////////////自定义结构体///////////////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)

typedef struct
{
	BYTE cc : 4;
	BYTE x  : 1;
	BYTE p  : 1;
	BYTE v  : 2;

	BYTE pt : 7;
	BYTE m  : 1;

	USHORT seq;
	ULONG  ts;
	ULONG  ssrc;
}RTP_HEADER;

typedef struct
{
	BYTE type : 5;
	BYTE nri  : 2;
	BYTE f    : 1;
}NALU_HEADER;

typedef struct
{
	BYTE type    : 5;
	BYTE reserve : 1;
	BYTE end     : 1;
	BYTE start   : 1;
}FUA_HEADER;

#pragma pack(pop)
/////////////////////////////////////////////////////////////////////////////////

const unsigned char NALU_START_CODE[4] = {0x00, 0x00, 0x00, 0x01};

//////////////////////////////////////////////////////////////////////////////////

class RtpUnpacker
{
public:
	RtpUnpacker();
	~RtpUnpacker();

public:
	int QueueStream(void* pBuf, unsigned int uLen, RtpBufferList* pList);
	int ParseRtpPacket(RtpBuffer* pRtp, FrameH264Buffer* pFrameH264);

private:
	RtpBuffer* m_pRtpRest;
	FrameH264Buffer* m_pFrameH264;

	bool m_bNeedSPS;
	bool m_bNeedPPS;

	WORD m_wSeq;
	DWORD m_dwSsrc;

public:
	DataBuffer* m_pSPSPPS;
};