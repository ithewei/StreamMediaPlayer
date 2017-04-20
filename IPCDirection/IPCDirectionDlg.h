
// IPCDirectionDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CIPCDirectionDlg 对话框
class CIPCDirectionDlg : public CDialog
{
// 构造
public:
	CIPCDirectionDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CIPCDirectionDlg();

// 对话框数据
	enum { IDD = IDD_IPCDIRECTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	bool play(const char* ip);
	friend int __stdcall cb_direction(int direction, void *pParam);
	bool IsCursorInControl(CPoint point, UINT ID);

private:
	long m_handle1;
	char m_szIP[16];
	bool m_bLButtonDown;
	CPoint m_PointDown;
	CRect m_Rect;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnInsertIpc();
	afx_msg void OnSelectIpc();
	afx_msg void OnDeleteIpc();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
