#pragma once


// CRTSPDialog 对话框

class CRTSPDialog : public CDialog
{
	DECLARE_DYNAMIC(CRTSPDialog)

public:
	CRTSPDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRTSPDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_AUTHORIZE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	char m_szUser[32];
	char m_szPswd[32];
};
