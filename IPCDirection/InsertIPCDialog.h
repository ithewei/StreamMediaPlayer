#pragma once


// CInsertIPCDialog 对话框
#define IPC_DIALOG_TYPE_INSERT 1
#define IPC_DIALOG_TYPE_UPDATE 2

class CInsertIPCDialog : public CDialog
{
	DECLARE_DYNAMIC(CInsertIPCDialog)

public:
	CInsertIPCDialog(int type, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInsertIPCDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_INSERTIPC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	IPCINFO m_IpcInfo;
	int m_type;
};
