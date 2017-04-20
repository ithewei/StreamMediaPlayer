// InsertIPCDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "IPCDirection.h"
#include "InsertIPCDialog.h"


// CInsertIPCDialog 对话框

IMPLEMENT_DYNAMIC(CInsertIPCDialog, CDialog)

CInsertIPCDialog::CInsertIPCDialog(int type, CWnd* pParent /*=NULL*/)
	: CDialog(CInsertIPCDialog::IDD, pParent)
{
	memset(&m_IpcInfo, 0, sizeof(IPCINFO));
	m_type = type;
}

CInsertIPCDialog::~CInsertIPCDialog()
{
}

void CInsertIPCDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_INSERT_IP, m_IpcInfo.ip, sizeof(m_IpcInfo.ip));
	DDX_Text(pDX, IDC_EDIT_INSERT_PORT, m_IpcInfo.port);
	DDX_Text(pDX, IDC_EDIT_INSERT_URL, m_IpcInfo.url, sizeof(m_IpcInfo.url));
	DDX_Text(pDX, IDC_EDIT_INSERT_LEFT, m_IpcInfo.left, sizeof(m_IpcInfo.left));
	DDX_Text(pDX, IDC_EDIT_INSERT_RIGHT, m_IpcInfo.right, sizeof(m_IpcInfo.right));
	DDX_Text(pDX, IDC_EDIT_INSERT_TOP, m_IpcInfo.top, sizeof(m_IpcInfo.top));
	DDX_Text(pDX, IDC_EDIT_INSERT_BOTTOM, m_IpcInfo.bottom, sizeof(m_IpcInfo.bottom));
	DDX_Text(pDX, IDC_EDIT_INSERT_DESCRIBE, m_IpcInfo.descibe, sizeof(m_IpcInfo.descibe));
}

BOOL CInsertIPCDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_type == IPC_DIALOG_TYPE_INSERT)
	{
		SetWindowText("添加IPC");
		GetDlgItem(IDOK)->SetWindowText("添加");
	}
	else if (m_type == IPC_DIALOG_TYPE_UPDATE)
	{
		SetWindowText("修改IPC");
		GetDlgItem(IDOK)->SetWindowText("修改");
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CInsertIPCDialog, CDialog)
END_MESSAGE_MAP()


// CInsertIPCDialog 消息处理程序
