#pragma once
#include "afxcmn.h"


// CSelectIpcDialog 对话框
#define IPC_DIALOG_TYPE_SELECT 1
#define IPC_DIALOG_TYPE_DELETE 2

class CSelectIpcDialog : public CDialog
{
	DECLARE_DYNAMIC(CSelectIpcDialog)

public:
	CSelectIpcDialog(int type, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSelectIpcDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_SELECTIPC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_ListCtrl;
	int m_type;
public:
	afx_msg void OnNMDblclkListIpc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDelete();
};
