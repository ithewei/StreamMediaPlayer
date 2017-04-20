// RTSPDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "IPCDirection.h"
#include "RTSPDialog.h"


// CRTSPDialog 对话框

IMPLEMENT_DYNAMIC(CRTSPDialog, CDialog)

CRTSPDialog::CRTSPDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRTSPDialog::IDD, pParent)
{
	memset(m_szUser, 0, sizeof(m_szUser));
	memset(m_szPswd, 0, sizeof(m_szPswd));
}

CRTSPDialog::~CRTSPDialog()
{
}

void CRTSPDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USER, m_szUser, 32);
	DDX_Text(pDX, IDC_EDIT_PSWD, m_szPswd, 32);
}


BEGIN_MESSAGE_MAP(CRTSPDialog, CDialog)
END_MESSAGE_MAP()


// CRTSPDialog 消息处理程序

