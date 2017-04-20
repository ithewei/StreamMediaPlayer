//CfgXml.cpp:

#include "stdafx.h"
#include "tinyxml/tinyxml.h"
#include "CfgXml.h"

void CreateXml()
{
	FILE* fp = fopen(XML_FILE_PATH,"rb");
	if (fp)
	{
		fclose(fp);
		return;
	}

	TiXmlDocument doc;

	TiXmlDeclaration* pDec = new TiXmlDeclaration("1.0","ANSI","yes");
	doc.LinkEndChild(pDec);

	TiXmlElement* pElemCfg = new TiXmlElement("Config");
	doc.LinkEndChild(pElemCfg);

	doc.SaveFile(XML_FILE_PATH);
}

void InsertIPC(IPCINFO& info)
{	
	IPCINFO selectinfo;
	if (SelectIPC(info.ip, selectinfo))
		return;

	TiXmlElement* pElemIpc = new TiXmlElement("IPC");
	pElemIpc->SetAttribute("ip", info.ip);
	pElemIpc->SetAttribute("port", info.port);
	pElemIpc->SetAttribute("url", info.url);
	pElemIpc->SetAttribute("left", info.left);
	pElemIpc->SetAttribute("right", info.right);
	pElemIpc->SetAttribute("top", info.top);
	pElemIpc->SetAttribute("bottom", info.bottom);
	pElemIpc->SetAttribute("describe", info.descibe);

	TiXmlDocument doc;
	doc.LoadFile(XML_FILE_PATH);
	TiXmlElement* pElemCfg = doc.FirstChildElement("Config");
	pElemCfg->LinkEndChild(pElemIpc);
	doc.SaveFile();
}

void DeleteIPC(const char* ip)
{
	TiXmlDocument doc;
	doc.LoadFile(XML_FILE_PATH);
	TiXmlElement* pElemCfg = doc.FirstChildElement("Config");
	
	TiXmlElement* pElemIpc = pElemCfg->FirstChildElement("IPC");
	if (!pElemIpc)
		return;

	do 
	{
		if (strcmp(pElemIpc->Attribute("ip"),ip) == 0)
		{
			pElemCfg->RemoveChild(pElemIpc);
			break;
		}
	}while ((pElemIpc = pElemIpc->NextSiblingElement()));

	doc.SaveFile();
}

void UpdateIPC(const char* ip, IPCINFO& info)
{
	TiXmlDocument doc;
	doc.LoadFile(XML_FILE_PATH);
	TiXmlElement* pElemCfg = doc.FirstChildElement("Config");

	TiXmlElement* pElemIpc = pElemCfg->FirstChildElement("IPC");
	if (!pElemIpc)
		return;

	do 
	{
		if (strcmp(pElemIpc->Attribute("ip"),ip) == 0)
		{
			pElemIpc->SetAttribute("ip", info.ip);
			pElemIpc->SetAttribute("port", info.port);
			pElemIpc->SetAttribute("url", info.url);
			pElemIpc->SetAttribute("left", info.left);
			pElemIpc->SetAttribute("right", info.right);
			pElemIpc->SetAttribute("top", info.top);
			pElemIpc->SetAttribute("bottom", info.bottom);
			pElemIpc->SetAttribute("describe", info.descibe);
			break;
		}
	}while ((pElemIpc = pElemIpc->NextSiblingElement()));

	doc.SaveFile();
}

bool SelectIPC(const char* ip, IPCINFO& info)
{
	TiXmlDocument doc;
	doc.LoadFile(XML_FILE_PATH);
	TiXmlElement* pElemCfg = doc.FirstChildElement("Config");

	TiXmlElement* pElemIpc = pElemCfg->FirstChildElement("IPC");
	if (!pElemIpc)
		return false;

	do 
	{
		if (strcmp(pElemIpc->Attribute("ip"),ip) == 0)
		{
			strcpy(info.ip, pElemIpc->Attribute("ip"));
			pElemIpc->Attribute("port",&info.port);
			strcpy(info.url, pElemIpc->Attribute("url"));
			strcpy(info.left, pElemIpc->Attribute("left"));
			strcpy(info.right, pElemIpc->Attribute("right"));
			strcpy(info.top, pElemIpc->Attribute("top"));
			strcpy(info.bottom, pElemIpc->Attribute("bottom"));
			strcpy(info.descibe, pElemIpc->Attribute("describe"));
			return true;
		}
	}while ((pElemIpc = pElemIpc->NextSiblingElement()));

	return false;
}

void SelectAllIPC(vector<IPCINFO>& vctIP)
{
	TiXmlDocument doc;
	doc.LoadFile(XML_FILE_PATH);
	TiXmlElement* pElemCfg = doc.FirstChildElement("Config");

	TiXmlElement* pElemIpc = pElemCfg->FirstChildElement("IPC");
	if (!pElemIpc)
		return;

	do 
	{
		IPCINFO info;
		strcpy(info.ip, pElemIpc->Attribute("ip"));
		pElemIpc->Attribute("port",&info.port);
		strcpy(info.url, pElemIpc->Attribute("url"));
		strcpy(info.left, pElemIpc->Attribute("left"));
		strcpy(info.right, pElemIpc->Attribute("right"));
		strcpy(info.top, pElemIpc->Attribute("top"));
		strcpy(info.bottom, pElemIpc->Attribute("bottom"));
		strcpy(info.descibe, pElemIpc->Attribute("describe"));
		vctIP.push_back(info);
	}while ((pElemIpc = pElemIpc->NextSiblingElement()));
}

