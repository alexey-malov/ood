// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class View : public ATL::CWindowImpl<View>
{
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(View)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_LBUTTONUP(OnLButtonUp)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	void OnPaint(WTL::CDCHandle /*unused*/)
	{
		// Создаём контекст рисования устройства, связанный с текущим окном.
		WTL::CPaintDC dc(*this);
		CRect clientRect;
		ATLVERIFY(GetClientRect(clientRect));
		dc.DrawText(std::to_wstring(m_counter).c_str(), -1, clientRect, DT_CENTER | DT_VCENTER);
	}

	void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
	{
		++m_counter;
		RedrawWindow();
	}

	int m_counter = 0;
};
