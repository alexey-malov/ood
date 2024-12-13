#pragma once
#include "Document.h"

class View
	: public ATL::CWindowImpl<View>
	, private IDocumentListener
{
public:
	DECLARE_WND_CLASS(NULL)
	View(Document& document)
		: m_document(document)
	{
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(View)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
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
		dc.DrawText(std::to_wstring(m_document.GetCounter()).c_str(), -1, clientRect, DT_CENTER | DT_VCENTER);
	}

	int OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
	{
		m_document.AddListener(*this);
		return 0;
	}

	void OnDestroy()
	{
		m_document.RemoveListener(*this);
	}

	void OnDocumentChanged() override
	{
		RedrawWindow();
	}

	void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
	{
		m_document.Increment();
	}

	Document& m_document;
};
