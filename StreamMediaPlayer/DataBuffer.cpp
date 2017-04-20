//DataBuffer.cpp:

#include "stdafx.h"
#include <assert.h>
#include "Databuffer.h"

DataBuffer::DataBuffer(length_t len)
: m_pData(NULL)
, m_Length(0)
, m_CurPos(0)
{
	m_pData = (unsigned char*)malloc(len);
	assert(m_pData != NULL);
	memset(m_pData, 0, len);
}

DataBuffer::~DataBuffer()
{
	if (m_pData)
	{
		free(m_pData);
		m_pData = NULL;
	}
}

length_t DataBuffer::ReadData(unsigned char* pData, length_t len)
{
	if (!pData || len == 0)
		return 0;

	memcpy(pData, m_pData + m_CurPos, len);

	return len;
}

length_t DataBuffer::WriteData(const void* pData, length_t len)
{
	if (!pData || len == 0)
		return 0;

	memcpy(m_pData + m_Length, pData, len);
	m_Length += len;

	return len;
}

int DataBuffer::Seek(int origin, int offset)
{
	if (origin == SEEK_SET)
		m_CurPos = offset;
	else if (origin == SEEK_CUR)
		m_CurPos += offset;
	else if (origin == SEEK_END)
		m_CurPos = m_Length - offset;
	
	return offset;
}

void DataBuffer::Clear()
{
	memset(m_pData, 0, m_Length);
	m_Length = 0;
}

bool DataBuffer::IsEmpty()
{
	if (m_Length == 0)
		return true;
	else
		return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameH264Buffer::Copy(FrameH264Buffer* pFrame)
{
	if (!pFrame)
		return;

	Clear();
	WriteData(pFrame->GetData(), pFrame->GetLength());
	ullPts = pFrame->ullPts;
	frame_type = pFrame->frame_type;
}

void FrameYUVBuffer::Copy(FrameYUVBuffer* pFrame)
{
	if (!pFrame)
		return;

	Clear();
	WriteData(pFrame->GetData(), pFrame->GetLength());
	nWidth  = pFrame->nWidth;
	nHeight = pFrame->nHeight;
	nPitch  = pFrame->nPitch;
	ullPts   = pFrame->ullPts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
RtpBufferList::RtpBufferList()
{
	m_pArrFrame = new RtpBuffer[MAXNUM_RTP];
	InitializeCriticalSection(&m_cs);
}

RtpBufferList::~RtpBufferList()
{
	if (m_pArrFrame)
	{
		delete[] m_pArrFrame;
		m_pArrFrame = NULL;
	}
	m_VctFrame.clear();
	DeleteCriticalSection(&m_cs);
};

RtpBuffer* RtpBufferList::FindFreeSpace()
{
	for (unsigned int i = 0; i < MAXNUM_RTP; i++)
	{
		if (m_pArrFrame[i].IsEmpty())
			return &m_pArrFrame[i];
	}

	return NULL;
}

void RtpBufferList::PushBack(RtpBuffer* pFrame)
{
	if (!pFrame)
		return;

	AutoLocker lock(m_cs);

	m_VctFrame.push_back(pFrame);
}

RtpBuffer* RtpBufferList::GetHead()
{
	AutoLocker lock(m_cs);

	std::vector<RtpBuffer*>::iterator iter = m_VctFrame.begin();
	if (iter != m_VctFrame.end())
		return *iter;
	else
		return NULL;
}

void RtpBufferList::RemoveHead()
{
	AutoLocker lock(m_cs);

	std::vector<RtpBuffer*>::iterator iter = m_VctFrame.begin();
	if (iter != m_VctFrame.end())
	{
		(*iter)->Clear();
		m_VctFrame.erase(iter);
	}
}

void RtpBufferList::RemoveAll()
{
	AutoLocker lock(m_cs);

	std::vector<RtpBuffer*>::iterator iter = m_VctFrame.begin();
	while(iter != m_VctFrame.end())
	{
		(*iter)->Clear();
		iter++;
	}

	m_VctFrame.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameH264BufferList::FrameH264BufferList()
{
	m_pArrFrame = new FrameH264Buffer[MAXNUM_FRAMECACHE];
	InitializeCriticalSection(&m_cs);
}

FrameH264BufferList::~FrameH264BufferList()
{
	if (m_pArrFrame)
	{
		delete[] m_pArrFrame;
		m_pArrFrame = NULL;
	}
	m_VctFrame.clear();
	DeleteCriticalSection(&m_cs);
};

FrameH264Buffer* FrameH264BufferList::FindFreeSpace()
{
	for (unsigned int i = 0; i < MAXNUM_FRAMECACHE; i++)
	{
		if (m_pArrFrame[i].IsEmpty())
			return &m_pArrFrame[i];
	}

	return NULL;
}

void FrameH264BufferList::PushBack(FrameH264Buffer* pFrame)
{
	if (!pFrame)
		return;

	AutoLocker lock(m_cs);

	m_VctFrame.push_back(pFrame);
}

FrameH264Buffer* FrameH264BufferList::GetHead()
{
	AutoLocker lock(m_cs);

	std::vector<FrameH264Buffer*>::iterator iter = m_VctFrame.begin();
	if (iter != m_VctFrame.end())
		return *iter;
	else
		return NULL;
}

void FrameH264BufferList::RemoveHead()
{
	AutoLocker lock(m_cs);

	std::vector<FrameH264Buffer*>::iterator iter = m_VctFrame.begin();
	if (iter != m_VctFrame.end())
	{
		(*iter)->Clear();
		m_VctFrame.erase(iter);
	}
}

void FrameH264BufferList::RemoveAll()
{
	AutoLocker lock(m_cs);

	std::vector<FrameH264Buffer*>::iterator iter = m_VctFrame.begin();
	while(iter != m_VctFrame.end())
	{
		(*iter)->Clear();
		iter++;
	}

	m_VctFrame.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameYUVBufferList::FrameYUVBufferList()
{
	m_pArrFrame = new FrameYUVBuffer[MAXNUM_FRAMECACHE];

	InitializeCriticalSection(&m_cs);
}

FrameYUVBufferList::~FrameYUVBufferList()
{
	if (m_pArrFrame)
	{
		delete[] m_pArrFrame;
		m_pArrFrame = NULL;
	}
	m_VctFrame.clear();

	DeleteCriticalSection(&m_cs);
};

FrameYUVBuffer* FrameYUVBufferList::FindFreeSpace()
{
	for (unsigned int i = 0; i < MAXNUM_FRAMECACHE; i++)
	{
		if (m_pArrFrame[i].IsEmpty())
			return &m_pArrFrame[i];
	}

	return NULL;
}

void FrameYUVBufferList::PushBack(FrameYUVBuffer* pFrame)
{
	if (!pFrame)
		return;

	AutoLocker lock(m_cs);

	m_VctFrame.push_back(pFrame);
}

FrameYUVBuffer* FrameYUVBufferList::GetHead()
{
	AutoLocker lock(m_cs);

	std::vector<FrameYUVBuffer*>::iterator iter = m_VctFrame.begin();

	if (iter != m_VctFrame.end())
		return *iter;
	else
		return NULL;
}

void FrameYUVBufferList::RemoveHead()
{
	AutoLocker lock(m_cs);

	std::vector<FrameYUVBuffer*>::iterator iter = m_VctFrame.begin();
	if (iter != m_VctFrame.end())
	{
		(*iter)->Clear();
		m_VctFrame.erase(iter);
	}
}

void FrameYUVBufferList::RemoveAll()
{
	AutoLocker lock(m_cs);

	std::vector<FrameYUVBuffer*>::iterator iter = m_VctFrame.begin();
	while(iter != m_VctFrame.end())
	{
		(*iter)->Clear();
		iter++;
	}

	m_VctFrame.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////