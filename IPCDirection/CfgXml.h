//CfgXml.h:

#pragma once

#include <string>
#include <vector>
using namespace std;

#define XML_FILE_PATH ("CfgIpc.xml")

typedef struct  
{
	char ip[16];
	int  port;
	char url[128];
	char left[16];
	char right[16];
	char top[16];
	char bottom[16];
	char descibe[256];
}IPCINFO;

void CreateXml();
void InsertIPC(IPCINFO& info);
void DeleteIPC(const char* ip);
void UpdateIPC(const char* ip, IPCINFO& info);
bool SelectIPC(const char* ip, IPCINFO& info);
void SelectAllIPC(vector<IPCINFO>& vctIP);