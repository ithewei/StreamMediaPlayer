//RtpUnpacker.cpp:

#include "stdafx.h"
#include "RtpUnpacker.h"

RtpUnpacker::RtpUnpacker()
: m_pFrameH264(NULL)
, m_pRtpRest(NULL)
, m_bNeedSPS(true)
, m_bNeedPPS(true)
, m_wSeq(0)
, m_dwSsrc(0)
{
	m_pFrameH264 = new FrameH264Buffer;
	m_pRtpRest = new RtpBuffer;
	m_pSPSPPS = new DataBuffer(1024);
}

RtpUnpacker::~RtpUnpacker()
{
	if (m_pFrameH264)
	{
		delete m_pFrameH264;
		m_pFrameH264 = NULL;
	}
	if (m_pRtpRest)
	{
		delete m_pRtpRest;
		m_pRtpRest = NULL;
	}
	if (m_pSPSPPS)
	{
		delete m_pSPSPPS;
		m_pSPSPPS = NULL;
	}
}

int RtpUnpacker::QueueStream(void* pBuf, unsigned int uLen, RtpBufferList* pList)
{
	if ( !pBuf || uLen == 0 || !pList)
		return -1;

	unsigned long len_src = m_pRtpRest->GetLength() + uLen;
	unsigned char* pAdd = (unsigned char*)malloc(len_src);
	unsigned char* pSrc = pAdd;
	memcpy(pSrc, m_pRtpRest->GetData(), m_pRtpRest->GetLength());
	memcpy(pSrc+m_pRtpRest->GetLength(), pBuf, uLen);
	m_pRtpRest->Clear();

	unsigned char flags = 0;
	unsigned int len_packet = 0;
	unsigned int len_rest = len_src;
	RtpBuffer* pRtp = NULL;

	while (len_rest > 4)
	{
		//对RTSP回复和RTCP包不做处理，仅入队RTP包
		flags = *pSrc++;
		if (flags != 0x24)
		{
			len_rest--;
			continue;
		}

		flags = *pSrc++;
		if (flags != 0x00)
		{
			len_rest--;
			continue;
		}

		len_packet = *pSrc++;
		len_packet <<= 8;
		len_packet |= *pSrc++;

		len_rest -= 4;

		if (len_rest >= len_packet)
		{
			while ( !(pRtp = pList->FindFreeSpace()) )
			{
				Sleep(1);
			}
			pRtp->WriteData(pSrc, len_packet);
			pList->PushBack(pRtp);
			len_rest -= len_packet;
			pSrc += len_packet;
		}
		else
		{
			m_pRtpRest->WriteData(pSrc-4, len_rest+4);
			free(pAdd);
			return 0;
		}
	}

	if (len_rest != 0)
	{
		m_pRtpRest->WriteData(pSrc, len_rest);
	}

	free(pAdd);
	return 0;
}

int RtpUnpacker::ParseRtpPacket(RtpBuffer* pRtp, FrameH264Buffer* pFrameH264)
{
	if (!pRtp || !pFrameH264)
		return -1;

	unsigned char* pSrc = pRtp->GetData();
	unsigned long uLen = pRtp->GetLength();
	//...debug
	//FILE* fp = fopen("rtp.data", "ab");
	//fwrite(pSrc, 1, uLen, fp);
	//fclose(fp);

	RTP_HEADER rtpHdr;
	unsigned char* pTmp = (unsigned char*)&rtpHdr;

	pTmp[0] = *pSrc++;
	pTmp[1] = *pSrc++;

	rtpHdr.seq = *pSrc++;
	rtpHdr.seq <<= 8;
	rtpHdr.seq |= *pSrc++;

	rtpHdr.ts = *pSrc++;
	rtpHdr.ts <<= 8;
	rtpHdr.ts |= *pSrc++;
	rtpHdr.ts <<= 8;
	rtpHdr.ts |= *pSrc++;
	rtpHdr.ts <<= 8;
	rtpHdr.ts |= *pSrc++;

	rtpHdr.ssrc = *pSrc++;
	rtpHdr.ssrc <<= 8;
	rtpHdr.ssrc |= *pSrc++;
	rtpHdr.ssrc <<= 8;
	rtpHdr.ssrc |= *pSrc++;
	rtpHdr.ssrc <<= 8;
	rtpHdr.ssrc |= *pSrc++;

	if (rtpHdr.v != 2)
		return -2;

	if (rtpHdr.pt != 96)
		return -3;

	if (m_wSeq == 0 && m_dwSsrc == 0)
	{
		m_wSeq = rtpHdr.seq;
		m_dwSsrc = rtpHdr.ssrc;
	}
	else
	{
		m_wSeq++;
		if (rtpHdr.seq != m_wSeq)//丢包处理
		{
			m_wSeq = rtpHdr.seq;

			m_bNeedSPS = true;
			m_bNeedPPS = true;

			m_pFrameH264->Clear();
			m_pSPSPPS->Clear();

			return -4;
		}
			

		if (rtpHdr.ssrc != m_dwSsrc)
		{
			m_dwSsrc = rtpHdr.ssrc;
			return -5;
		}
	}

	if (rtpHdr.x == 1){
		// 16字节的扩展字段
		pSrc += 16;
		uLen -= 16;
	}

	NALU_HEADER naluHdr;
	pTmp = (unsigned char*)&naluHdr;
	pTmp[0] = *pSrc++;
	int type = naluHdr.type;

	//如果是FU_A类型的一些处理
	FUA_HEADER fuaHdr;
	unsigned int uLenData = uLen - sizeof(RTP_HEADER) - sizeof(NALU_HEADER);
	if (type == 28)
	{
		uLenData -= sizeof(FUA_HEADER);//数据长度-1
		pTmp = (unsigned char*)&fuaHdr;
		pTmp[0] = *pSrc++;
		naluHdr.type = fuaHdr.type;//替换nalu头的具体类型，方便后续将nalu头写入内存
	}

	//记录下SPS、PPS
	if (m_bNeedSPS || m_bNeedPPS)
	{
		if (naluHdr.type == 0x07)
		{
			m_pSPSPPS->WriteData(NALU_START_CODE, 4);
			m_pSPSPPS->WriteData(&naluHdr, 1);
			m_pSPSPPS->WriteData(pSrc, uLenData);
			m_bNeedSPS = false;
		}
		else if (naluHdr.type == 0x08)
		{
			m_pSPSPPS->WriteData(NALU_START_CODE, 4);
			m_pSPSPPS->WriteData(&naluHdr, 1);
			m_pSPSPPS->WriteData(pSrc, uLenData);
			m_bNeedPPS = false;
		}
	}

	//判断条件，添加起始码和NALU头
	if (type != 28 || fuaHdr.start == 1)
	{
		m_pFrameH264->WriteData(NALU_START_CODE, 4);
		m_pFrameH264->WriteData(&naluHdr, 1);
		m_pFrameH264->ullPts = rtpHdr.ts;
		m_pFrameH264->frame_type = naluHdr.type;
	}
	
	m_pFrameH264->WriteData(pSrc, uLenData);

	if (rtpHdr.m)
	{
		pFrameH264->Copy(m_pFrameH264);
		m_pFrameH264->Clear();
		return 0;
	}
	else
	{
		return 1;
	}
}