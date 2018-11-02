
// FtpServerUIDlg.h : 头文件
//

#pragma once
#include "FtpServer.h"

// CFtpServerUIDlg 对话框
class CFtpServerUIDlg : public CDialogEx
{
// 构造
public:
	CFtpServerUIDlg(CWnd* pParent = NULL);	// 标准构造函数

	~CFtpServerUIDlg();

// 对话框数据
	enum { IDD = IDD_FTPSERVERUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CFtpServer	*m_server;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
