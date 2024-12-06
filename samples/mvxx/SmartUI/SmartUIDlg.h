
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

private:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
	BOOL OnInitDialog() override;

private:
	void AddAboutItemToSystemMenu();

	// Generated message map functions
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButton();
	DECLARE_MESSAGE_MAP()
private:
	HICON m_hIcon;
	CButton m_button;
	int m_clickCount = 0;
};
