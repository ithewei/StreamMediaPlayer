#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////
class DAutoLocker
{
public:
	DAutoLocker(CRITICAL_SECTION &cs);
	~DAutoLocker(void);

private:
	CRITICAL_SECTION &m_cs;
};
/////////////////////////////////////////////////////////////////////////////////////////////////

struct H264FrameInfo 
{
	char *pData;
	unsigned uLen;
	unsigned long long ullPts;
	bool bFrameFinish;

	H264FrameInfo()
	{
		pData = NULL;
		uLen = 0;
		ullPts = 0;
		bFrameFinish = false;
	}
};

#pragma pack(push)
#pragma pack(1)

typedef struct tagD_PES_TIME 
{
	unsigned char mark0					:1;
	unsigned char base3230				:3;
	unsigned char reserved				:4;


	unsigned char base2922				:8;

	unsigned char mark1					:1;
	unsigned char base2115				:7;


	unsigned char base1407				:8;

	unsigned char mark2					:1;
	unsigned char base0600				:7;	
}d_pes_time;

#pragma pack(pop)

const unsigned char IDENTI_PS_START[] = {0x00, 0x00, 0x01, 0xba};
const unsigned char IDENTI_PES_START_V[] = {0x00, 0x00, 0x01, 0xe0};	//HIK-V
const unsigned char IDENTI_PES_START_A[] = {0x00, 0x00, 0x01, 0xc0}; //HIK-A
const unsigned char IDENTI_H264_NALU[] = {0x00, 0x00, 0x00, 0x01};

const int MAXLEN_FRAME_SIZE_H264 = 1<<20;
const int MAXLEN_FRAME_SIZE_YUV = 1920*1080*2;
/////////////////////////////////////////////////////////////////////////////////////////////////////

class DMediaFileParser
{
public:
	DMediaFileParser(FILE *file);
	~DMediaFileParser(void);

public:
	int reset();
	int read_frame(void **data, int& len,unsigned long long& pts, int& stream_idx, int& frame_type);	//stream_idx: 1-v 2-a
	int get_total_play_len();
	int seek(int seconds);
	int seek_to(int seconds);
	bool CanSeek()	{ return (m_pts_base==-1) ? false : true; }
	FILE* get_file_handle()	{ return m_file; }
	__int64 get_base_pts()		{ return m_pts_base; }

protected:
	__int64 find_identi_pos(const char* identi, int len_identi, int direction=0);	//查找标记 direction:0正向查找 1:反向查找
	__int64 reverse_find_identi_pos(const char* identi, int len_identi);
	int seek_back(__int64 len);
	__int64 get_file_size();

	
protected:

private:
	FILE *m_file;
	H264FrameInfo m_frame_info;
	unsigned char *m_data_cache_read;	
	__int64 m_pts_base;
	CRITICAL_SECTION m_csFile;
};
