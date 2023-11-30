
// SmartUIDlg.h : header file
//

#pragma once

// SmartUIDlg dialog
class SmartUIDlg : public CDialogEx
{
	// Construction
public:
	SmartUIDlg(CWnd* pParent = nullptr); // standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_SMARTUI_DIALOG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	// Implementation
protected:
	virtual BOOL OnInitDialog();

private:
	HICON m_hIcon;

	// Generated message map functions
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButton();
	DECLARE_MESSAGE_MAP()
private:
	CButton m_button;
	int m_clickCount = 0;
};
