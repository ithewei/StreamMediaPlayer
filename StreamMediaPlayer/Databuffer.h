//DataBuffer.h:

#pragma once

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned long length_t;

class DataBuffer
{
public:
	DataBuffer(length_t len);
	virtual ~DataBuffer();

public:
	unsigned char* GetData() const { return m_pData; }
	length_t GetLength() const { return m_Length; }
	length_t ReadData(unsigned char* pData, length_t len);
	length_t WriteData(const void* pData, length_t len);
	int Seek(int origin, int offset);
	void Clear();
	bool IsEmpty();

private:
	unsigned char* m_pData;
	length_t m_Length;
	length_t m_CurPos;
};

class AutoLocker
{
public:
	AutoLocker(CRITICAL_SECTION& cs) : m_cs(cs) { EnterCriticalSection(&m_cs); }
	~AutoLocker() { LeaveCriticalSection(&m_cs); }

private:
	CRITICAL_SECTION& m_cs;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAXLEN_RTP				1500
#define MAXNUM_RTP				1000

#define MAXLEN_FRAMEH264		1<<20
#define MAXLEN_FRAMEYUV			1920*1280*3/2
#define MAXNUM_FRAMECACHE		10

class RtpBuffer : public DataBuffer
{
public:
	RtpBuffer() : DataBuffer(MAXLEN_RTP) {}
	~RtpBuffer() {}
};

class FrameH264Buffer : public DataBuffer
{
public:
	FrameH264Buffer() : DataBuffer(MAXLEN_FRAMEH264) { ullPts = 0; frame_type = 0; }
	virtual ~FrameH264Buffer() {}

	void Copy(FrameH264Buffer* pFrame);
	//length_t WriteData(const void* pData, length_t len){
	//	//...debug
	//	FILE* fp = fopen("1.h264", "ab");
	//	fwrite(pData, 1, len, fp);
	//	fclose(fp);
	//	return __super::WriteData(pData, len);
	//}

public:
	unsigned long long ullPts;
	int frame_type;
};

class FrameYUVBuffer : public DataBuffer
{
public:
	FrameYUVBuffer() : DataBuffer(MAXLEN_FRAMEYUV) { nWidth = 0; nHeight = 0; nPitch = 0; ullPts = 0; }
	virtual ~FrameYUVBuffer() {}

	void Copy(FrameYUVBuffer* pFrame);

public:
	int nWidth;
	int nHeight;
	int nPitch;
	unsigned long long ullPts;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RtpBufferList
{
public:
	RtpBufferList();
	virtual ~RtpBufferList();

public:
	RtpBuffer* FindFreeSpace();
	void PushBack(RtpBuffer* pData);
	RtpBuffer* GetHead();
	void RemoveHead();
	void RemoveAll();

private:
	RtpBuffer* m_pArrFrame;
	std::vector<RtpBuffer*> m_VctFrame;
	CRITICAL_SECTION m_cs;
};

class FrameH264BufferList
{
public:
	FrameH264BufferList();
	virtual ~FrameH264BufferList();

public:
	FrameH264Buffer* FindFreeSpace();
	void PushBack(FrameH264Buffer* pData);
	FrameH264Buffer* GetHead();
	void RemoveHead();
	void RemoveAll();

private:
	FrameH264Buffer* m_pArrFrame;
	std::vector<FrameH264Buffer*> m_VctFrame;
	CRITICAL_SECTION m_cs;
};

class FrameYUVBufferList
{
public:
	FrameYUVBufferList();
	virtual ~FrameYUVBufferList();

public:
	FrameYUVBuffer* FindFreeSpace();
	void PushBack(FrameYUVBuffer* pData);
	FrameYUVBuffer* GetHead();
	void RemoveHead();
	void RemoveAll();

private:
	FrameYUVBuffer* m_pArrFrame;
	std::vector<FrameYUVBuffer*> m_VctFrame;
	CRITICAL_SECTION m_cs;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////