//

#include "stdafx.h"
#include "DMediaFileParser.h"

#include <assert.h>
//////////////////////////////////////////////////////////////////////////
DAutoLocker::DAutoLocker(CRITICAL_SECTION &cs)
: m_cs(cs)
{
	EnterCriticalSection(&m_cs);
}

DAutoLocker::~DAutoLocker(void)
{
	LeaveCriticalSection(&m_cs);
}
////////////////////////////////////////////////////////////////////////////

DMediaFileParser::DMediaFileParser(FILE* file)
	: m_file(file)
	, m_data_cache_read(NULL)
	, m_pts_base(-1)	
{
	memset(&m_frame_info, 0, sizeof(H264FrameInfo));
	m_frame_info.pData = (char*)malloc(MAXLEN_FRAME_SIZE_H264);
	m_data_cache_read = (unsigned char*)malloc(MAXLEN_FRAME_SIZE_H264);	

	InitializeCriticalSection(&m_csFile);
}

DMediaFileParser::~DMediaFileParser(void)
{
	if ( m_frame_info.pData != NULL )
	{
		free(m_frame_info.pData);
		m_frame_info.pData = NULL;
	}
	if ( m_data_cache_read != NULL )
	{
		free(m_data_cache_read);
		m_data_cache_read = NULL;
	}

	DeleteCriticalSection(&m_csFile);
}

int DMediaFileParser::reset()
{
	m_pts_base = -1;

	return 0;
}

int DMediaFileParser::read_frame(void **data, int& len, unsigned long long& pts, int& stream_idx, int& frame_type)
{
	DAutoLocker locker(m_csFile);

	if ( data == NULL )
		return -1;
	if ( m_file == NULL )
		return -2;
	if ( m_frame_info.pData == NULL )
		return -3;
	
	unsigned char ident[4] = {0};
	
	int offset = 0;
	int ret_bytes = 0;
	int type = -1;

	while ( true )
	{
		offset = 0;
		ret_bytes = 0;
		while ( (ret_bytes=fread(ident+offset, 1, 4-offset, m_file)) > 0 )
		{
			if ( ret_bytes <= 0 )
				return -5;

			offset = 3;

			if ( memcmp(ident, IDENTI_PS_START, 4) == 0 )
			{
				type = 0;
				break;
			}
			else if ( memcmp(ident, IDENTI_PES_START_V, 4) == 0 )
			{
				type = 1;
				break;
			}
			memcpy(ident, ident+1, 3);
			ident[3] = 0;
		}
		if ( type == -1 )
		{
			//dxh:2015-05-18test
// 			char chInfo[32] = {0};
// 			sprintf(chInfo, "pos:%08x\r\n", ftell(m_file));
// 			OutputDebugStringA(chInfo);

			return -5;
		}


		if ( type == 0 )		//0x000001ba 
		{
			fseek(m_file, 9, SEEK_CUR);
			ret_bytes = fread(ident, 1, 1, m_file);
			if ( ret_bytes <= 0 )
				return -5;
			offset = ident[0]&0x07;
			fseek(m_file, offset, SEEK_CUR);
			ret_bytes = fread(ident, 1, 4, m_file);
			if ( ret_bytes < 4 )
				return -5;
			if ( memcmp(ident, IDENTI_PES_START_V, 4) == 0 )
				type = 1;
			else if ( memcmp(ident, IDENTI_PES_START_A, 4) == 0 )
				type = 2;
			else
				type = -1;			
		}

		if ( type == 2 )	//a
			return 1;	//not support

		d_pes_time pes_time = {0};
		if ( type == 1 )	//v
		{
			if ( fread(ident, 1, 2, m_file) < 2 )
			{
				//dxh:2015-05-18test
// 				char chInfo[32] = {0};
// 				sprintf(chInfo, "pos:%08x\r\n", ftell(m_file));
// 				OutputDebugStringA(chInfo);

				return -5;
			}
			int pack_len = (ident[0]<<8) | ident[1];
			if ( pack_len == 0 )
			{
#ifdef _DEBUG
				char chInfo[32] = {0};
				sprintf(chInfo, "pos:%08x\r\n", ftell(m_file));
				OutputDebugStringA(chInfo);
#endif

				//return -6;
				seek_back(2);  //hewei:2015-9-24
				continue;
			}

			if ( fread(ident, 1, 3, m_file) < 3 )
				return -5;
			int head_data_len = ident[2]&0xff;		

			int pts_dts_flag = (ident[1]>>6) & 0x03;
			if ( pts_dts_flag == 2 || pts_dts_flag == 3 )
			{
				if ( m_frame_info.uLen != 0 )
				{
					len = m_frame_info.uLen;
					pts = m_frame_info.ullPts;
					stream_idx = 1;
					seek_back(9);
					memcpy(m_data_cache_read, m_frame_info.pData, m_frame_info.uLen);
					*data = m_data_cache_read;
					m_frame_info.uLen = 0;
					frame_type = m_frame_info.pData[4] & 0x1f;					
					
					return 0;
				}

				if ( fread(&pes_time, 1, 5, m_file) < sizeof(d_pes_time) )
					return -5;

				fseek(m_file, head_data_len-sizeof(d_pes_time), SEEK_CUR);

				
				m_frame_info.ullPts = (long long)pes_time.base3230<<30 |
					(long long)pes_time.base2922<<22 |
					(long long)pes_time.base2115<<15 |
					(long long)pes_time.base1407<<7 |
					(long long)pes_time.base0600;	
				if ( m_pts_base == -1 )
				{
					m_pts_base = m_frame_info.ullPts;					
				}
			}	
			else
				fseek(m_file, head_data_len, SEEK_CUR);

			pack_len -= head_data_len+3;

			ret_bytes = fread(m_data_cache_read, 1, pack_len, m_file);
			if ( ret_bytes < pack_len )
				return -5;	

			memcpy(m_frame_info.pData+m_frame_info.uLen, m_data_cache_read, ret_bytes);
			m_frame_info.uLen += ret_bytes;
		}//EndOf "if ( type == 1 )	//v"
	}//EndOf "while ( true )"

	return -5;
}

int DMediaFileParser::get_total_play_len()
{
	DAutoLocker locker(m_csFile);

	if ( !m_file )
		return 0;

	__int64 pos_bak = ftell(m_file);

	if ( fseek(m_file, 0, SEEK_SET) < 0 )
		return 0;

	__int64 pos_v_start = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
	if ( pos_v_start < 0 )
	{
		fseek(m_file, pos_bak, SEEK_SET);
		return 0;
	}

	d_pes_time dpt;

	fseek(m_file, 9+pos_v_start, SEEK_SET);
	fread((unsigned char*)&dpt, 1, sizeof(dpt), m_file);
	__int64 pts_start =  (long long)dpt.base3230<<30 |
							(long long)dpt.base2922<<22 |
							(long long)dpt.base2115<<15 |
							(long long)dpt.base1407<<7 |
							(long long)dpt.base0600;	

	__int64 pos_v_stop = reverse_find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
	if ( pos_v_stop < 0 )
	{
		fseek(m_file, pos_bak, SEEK_SET);
		return 0;
	}
	fseek(m_file, 9+pos_v_stop, SEEK_SET);
	fread((unsigned char*)&dpt, 1, sizeof(dpt), m_file);
	__int64 pts_stop =  (long long)dpt.base3230<<30 |
		(long long)dpt.base2922<<22 |
		(long long)dpt.base2115<<15 |
		(long long)dpt.base1407<<7 |
		(long long)dpt.base0600;	

	
	__int64 span = pts_stop - pts_start;
	if ( span < 0 )
		span += (unsigned __int64)0xffffffffffffffff;

	int len = (int)(span / 90000);

#ifdef _DEBUG
	char chInfo[32] = {0};
	sprintf(chInfo, "Media file total len:%ds\r\n", len);
	OutputDebugStringA(chInfo);
#endif

	fseek(m_file, pos_bak, SEEK_SET);

	return len;
}

// int DMediaFileParser::seek(int milli_seconds)
// {
// 	if ( m_file == NULL )
// 		return -1;
// 
// 	
// 	__int64 pos_bak = ftell(m_file);
// 	__int64 identi_pos_cur = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
// 	__int64 identi_pos_next = 0;
// 	unsigned char flag[3] = {0};
// 
// 	while ( true )
// 	{
// 		if ( fseek(m_file, 6+identi_pos_cur, SEEK_SET) < 0 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 		if ( fread(flag, 1, 3, m_file) < 3 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 		if ( ((flag[1]>>6) & 0x03) == 2 )
// 			break;
// 
// 		identi_pos_cur = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
// 		if ( identi_pos_cur == -1 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 	}
// 	
// 	
// 	d_pes_time dpt;
// 
// // 	if (fseek(m_file, 9+identi_pos_cur, SEEK_SET) < 0 )
// // 	{
// // 		fseek(m_file, pos_bak, SEEK_SET);
// // 		return -2;
// // 	}
// 
// 	fread((unsigned char*)&dpt, 1, sizeof(dpt), m_file);
// 	__int64 pts_cur =  (long long)dpt.base3230<<30 |
// 		(long long)dpt.base2922<<22 |
// 		(long long)dpt.base2115<<15 |
// 		(long long)dpt.base1407<<7 |
// 		(long long)dpt.base0600;	
// 
// 	char chInfo[32] = {0};
// 
// 	if (fseek(m_file, 2048, SEEK_CUR) < 0 )
// 	{
// 		fseek(m_file, pos_bak, SEEK_SET);
// 		return -2;
// 	}
// 
// 	while ( true )
// 	{	
// 		identi_pos_next = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
// 		
// 		if ( identi_pos_next == -1 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 
// 		if ( fseek(m_file, 6+identi_pos_next, SEEK_SET) < 0 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 
// // 		unsigned char tmp[9];
// // 		seek_back(6);
// // 		fread(tmp, 1, 9, m_file);
// // 		seek_back(3);
// 		
// 		if ( fread(flag, 1, 3, m_file) < 3 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 		if ( ((flag[1]>>6) & 0x03) != 2 )
// 			continue;		
// 
// 		fread((unsigned char*)&dpt, 1, sizeof(dpt), m_file);
// 		__int64 pts =  (long long)dpt.base3230<<30 |
// 			(long long)dpt.base2922<<22 |
// 			(long long)dpt.base2115<<15 |
// 			(long long)dpt.base1407<<7 |
// 			(long long)dpt.base0600;	
// 		
// 		if ( fseek(m_file, flag[2]-sizeof(dpt), SEEK_CUR) < 0 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 		unsigned char data_nalu[5] = {0};
// 		if ( fread(data_nalu, 1, 5, m_file) < 5 )
// 		{
// 			fseek(m_file, pos_bak, SEEK_SET);
// 			return -2;
// 		}
// 		if ( memcmp(data_nalu, IDENTI_H264_NALU, 4) != 0 )
// 			continue;
// 		int type = data_nalu[4]&0x1f;
// 		
// 		if ( type != 7 && type != 8 && type != 5 )
// 			continue;
// 			
// 
// 		__int64 pts_span = pts - pts_cur;
// 		if ( pts_span < 0 )
// 			pts_span += (unsigned __int64)0xffffffffffffffff;
// 		pts_span /= 90;
// 
// 		if ( pts_span >= milli_seconds-2000 && pts_span <= milli_seconds+2000 )
// 		{
// 			fseek(m_file, identi_pos_next, SEEK_SET);
// 			break;		
// 		}			
// 	}
// 
// 
// 
// 
// 	return 0;
// 
// }

int DMediaFileParser::seek(int seconds)
{
	DAutoLocker locker(m_csFile);

	if ( m_pts_base == -1 )
	{
#ifdef _DEBUG
		OutputDebugStringA("@@@@@@@@@@@@@@@@\r\n");
#endif
		return -99;
	}

	if ( m_file == NULL )
		return -1;

	if ( seconds == 0 )
		return 1;

	__int64 pos_bak = ftell(m_file);
	__int64 identi_pos_cur = 0;
	__int64 identi_pos_next = 0;
	unsigned char flag[3] = {0};

	identi_pos_cur = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));

	while ( true )
	{
		if ( fseek(m_file, 6+identi_pos_cur, SEEK_SET) < 0 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}
		if ( fread(flag, 1, 3, m_file) < 3 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}
		if ( ((flag[1]>>6) & 0x03) == 2 )
			break;

		identi_pos_cur = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
		if ( identi_pos_cur == -1 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}
 	}


	d_pes_time dpt;

	fread((unsigned char*)&dpt, 1, sizeof(dpt), m_file);
	__int64 pts_cur =  (long long)dpt.base3230<<30 |
		(long long)dpt.base2922<<22 |
		(long long)dpt.base2115<<15 |
		(long long)dpt.base1407<<7 |
		(long long)dpt.base0600;	


	while ( true )
	{	
// 		if ( seconds > 0 )
// 			identi_pos_next = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
// 		else
// 			identi_pos_next = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V), 1);

		if ( seconds > 0 )
			identi_pos_next = find_identi_pos((char*)IDENTI_PES_START_V, sizeof(IDENTI_PES_START_V));
		else
		{
			__int64 target_sec = (m_frame_info.ullPts - m_pts_base) / 90000 + seconds;
			if ( target_sec < 0 )
				target_sec = 0;
			fseek(m_file, 0, SEEK_SET);

			return seek(target_sec);
		}

		if ( identi_pos_next == -1 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}

		if ( fseek(m_file, 6+identi_pos_next, SEEK_SET) < 0 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}
	
// 		char tmp[16] = {0};
// 		fread(tmp, 1, 16, m_file);
// 		seek_back(16);
		
		if ( fread(flag, 1, 3, m_file) < 3 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}

		if ( ((flag[1]>>6) & 0x03) != 2 )
		{
			continue;	
		}

		fread((unsigned char*)&dpt, 1, sizeof(dpt), m_file);
		__int64 pts =  (long long)dpt.base3230<<30 |
			(long long)dpt.base2922<<22 |
			(long long)dpt.base2115<<15 |
			(long long)dpt.base1407<<7 |
			(long long)dpt.base0600;	
		

		if ( fseek(m_file, flag[2]-sizeof(dpt), SEEK_CUR) < 0 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}

		unsigned char data_nalu[5] = {0};
		if ( fread(data_nalu, 1, 5, m_file) < 5 )
		{
			fseek(m_file, pos_bak, SEEK_SET);
			return -2;
		}

		if ( memcmp(data_nalu, IDENTI_H264_NALU, 4) != 0 )
		{		
			continue;
		}
		int type = data_nalu[4]&0x1f;

		if ( type != 7 && type != 8 && type != 5 )
		{			
			continue;
		}


		__int64 pts_span = pts - pts_cur;
		if ( pts_span < 0 )
			pts_span += (unsigned __int64)0xffffffffffffffff;
		pts_span /= 90000;

		if ( pts_span >= seconds-2 && pts_span <= seconds+2 )
		{
			fseek(m_file, identi_pos_next, SEEK_SET);
			break;		
		}		
		else
		{
			__int64 nRemain = seconds - pts_span;
			if ( nRemain > 0 )
				fseek(m_file, identi_pos_next+(1<<17)*nRemain, SEEK_SET);		
		}
	}//EndOf "while ( true )"


	return 0;
}

int DMediaFileParser::seek_to(int seconds)
{
//	DAutoLocker locker(m_csFile);

	if ( m_file == NULL )
		return -1;

	if ( m_pts_base == -1 )
		return -99;

	if ( seconds == 0 )
	{
		fseek(m_file, 0, SEEK_SET);
		return 0;
	}

	int relativeSeconds = seconds - int((m_frame_info.ullPts - m_pts_base) / 90000);
	return seek(relativeSeconds);

//	return seek(seconds);	
}

__int64 DMediaFileParser::find_identi_pos(const char* identi, int len_identi, int direction/*=0*/)
{
	if ( identi == NULL || m_file == NULL || len_identi <= 0 )
		return -1;
	
	char *cache = (char*)malloc(len_identi);
	assert(cache != NULL);
	int offset = 0;
	__int64 pos_bak = ftell(m_file);
	int ret = 0;

	if ( direction == 0 )
	{
		while ( (ret = fread(cache+offset, 1, len_identi-offset, m_file)) == len_identi-offset )
		{			
			if ( memcmp(cache, identi, len_identi) == 0 )
			{
				__int64 pos_cur = ftell(m_file) - len_identi;
				fseek(m_file, pos_bak, SEEK_SET);				
				free(cache);
				return pos_cur;
			}
			memcpy(cache, cache+1, len_identi-1);
			*(cache+len_identi-1) = 0;
			offset = len_identi - 1;
		}
	}
	else
	{
		while ( (ret=fread(cache, 1, len_identi-offset, m_file)) == len_identi-offset )
		{			
			if ( memcmp(cache, identi, len_identi) == 0 )
			{
				__int64 pos_cur = 0;
				if ( direction == 0 )
					pos_cur = ftell(m_file) - len_identi;
				else
					pos_cur = ftell(m_file)-1;

				fseek(m_file, pos_bak, SEEK_SET);
				free(cache);
				return pos_cur;
			}
			memcpy(cache+1, cache, len_identi-1);
			*cache = 0;

			ret = (offset == 0 ? seek_back(len_identi+1) : seek_back(2));
			if ( ret < 0 )
			{
				free(cache);
				return -1;
			}				

			offset = len_identi - 1;
		}
	}

	free(cache);
	cache = NULL;

	fseek(m_file, pos_bak, SEEK_SET);

	return -1;
}

__int64 DMediaFileParser::reverse_find_identi_pos(const char* identi, int len_identi)
{
	if ( identi == NULL || m_file == NULL || len_identi <= 0 )
		return -1;

	char *cache = (char*)malloc(len_identi);
	assert(cache != NULL);
	__int64 pos_bak = ftell(m_file);
	int ret = 0;
	__int64 len_file = 0;
	__int64 pos_cur = 0;

	fseek(m_file, 0, SEEK_END);
	len_file = ftell(m_file);
	if ( len_file < len_identi+5+sizeof(d_pes_time) )
	{
		free(cache);
		cache = NULL;
		return -1;
	}

	ret = fseek(m_file, len_file-(len_identi+5+sizeof(d_pes_time)), SEEK_SET);

	while ( (ret = fread(cache, 1, len_identi, m_file)) == len_identi )
	{
		if ( memcmp(cache, identi, len_identi) == 0 )
		{
			pos_cur = ftell(m_file) - len_identi;
			fseek(m_file, pos_bak, SEEK_SET);
			free(cache);
			cache = NULL;
			return pos_cur;
		}		

		if ( seek_back(len_identi+1) < 0 )
		{
			free(cache);
			cache = NULL;
			return -1;
		}
	}
	

	free(cache);
	cache = NULL;

	return -1;
}

int DMediaFileParser::seek_back(__int64 len)
{
	if ( m_file == NULL )
		return -1;

	__int64 pos = ftell(m_file);
	pos -= len;	
	
	return fseek(m_file, pos, SEEK_SET);
}

__int64 DMediaFileParser::get_file_size()
{
	__int64 pos_bak = ftell(m_file);
	fseek(m_file, 0, SEEK_END);
	__int64 len = ftell(m_file);
	fseek(m_file, pos_bak, SEEK_SET);

	return len;	
}