
// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "EquationSolver.h"
#include "afxdialogex.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MODELVIEWPRESENTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BOOL CMainDlg::PreTranslateMessage(MSG* msg)
{
	if (msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN)
	{
		auto focus = GetFocus();
		if (focus == GetDlgItem(IDC_COEFF_A))
		{
			OnChangeCoeffA();
			return TRUE;
		}
		else if (focus == GetDlgItem(IDC_COEFF_B))
		{
			OnChangeCoeffB();
			return TRUE;
		}
		else if (focus == GetDlgItem(IDC_COEFF_C))
		{
			OnChangeCoeffC();
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(msg);
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_COEFF_A, m_coeffA);
	DDX_Text(pDX, IDC_COEFF_B, m_coeffB);
	DDX_Text(pDX, IDC_COEFF_C, m_coeffC);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_EN_KILLFOCUS(IDC_COEFF_A, &CMainDlg::OnKillfocusCoeffA)
	ON_EN_KILLFOCUS(IDC_COEFF_B, &CMainDlg::OnKillfocusCoeffB)
	ON_EN_KILLFOCUS(IDC_COEFF_C, &CMainDlg::OnKillfocusCoeffC)
END_MESSAGE_MAP()

BOOL CMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE); // Set big icon
	SetIcon(m_hIcon, FALSE); // Set small icon

	m_chart.SubclassDlgItem(IDC_CHART, this);

	m_init();

	UpdateEquation();

	return TRUE; // return TRUE  unless you set the focus to a control
}

void CMainDlg::SetCoeffs(double a, double b, double c)
{
	m_coeffA = a;
	m_coeffB = b;
	m_coeffC = c;
	if (m_hWnd)
	{
		UpdateData(FALSE);
	}
}

sig::connection CMainDlg::DoOnInit(const InitSignal::slot_type& handler)
{
	return m_init.connect(handler);
}

IChartView& CMainDlg::GetChartView()
{
	return m_chart;
}

void CMainDlg::SetNoSolution()
{
	SetSolutionText(L"No real roots");
}

void CMainDlg::SetInfiniteSolutions()
{
	SetSolutionText(L"Infinite number of roots");
}

void CMainDlg::SetSingleSolution(double solution)
{
	SetSolutionText(std::format(L"One root: %1%", solution));
}

void CMainDlg::SetTwoRootsSolutuion(double root1, double root2)
{
	SetSolutionText(std::format(L"Two roots: %1% and %2%", root1, root2));
}

sig::connection CMainDlg::DoOnCoeffAChange(const CoeffChangeSignal::slot_type& handler)
{
	return m_coeffAChanged.connect(handler);
}

sig::connection CMainDlg::DoOnCoeffBChange(const CoeffChangeSignal::slot_type& handler)
{
	return m_coeffBChanged.connect(handler);
}

sig::connection CMainDlg::DoOnCoeffCChange(const CoeffChangeSignal::slot_type& handler)
{
	return m_coeffCChanged.connect(handler);
}

void CMainDlg::SetSolutionText(const std::wstring& text)
{
	SetDlgItemText(IDC_SOLUTION, text.c_str());
}

void CMainDlg::SetEquationText(const std::wstring& text)
{
	SetDlgItemText(IDC_EQUATION, text.c_str());
}

void CMainDlg::UpdateEquation()
{
	auto ToSignedString = [](double value) {
		std::wostringstream strm;
		strm << std::abs(value);

		return ((value < 0) ? L"- " : L"+ ") + strm.str();
	};

	SetEquationText(std::format(L"{}x\u00b2 {}x {} = 0", m_coeffA, ToSignedString(m_coeffB), ToSignedString(m_coeffC)));
}

void CMainDlg::OnChangeCoeffA()
{
	if (UpdateData())
	{
		m_coeffAChanged(m_coeffA);
		UpdateEquation();
	}
}

void CMainDlg::OnChangeCoeffB()
{
	if (UpdateData())
	{
		m_coeffBChanged(m_coeffB);
		UpdateEquation();
	}
}

void CMainDlg::OnChangeCoeffC()
{
	if (UpdateData())
	{
		m_coeffCChanged(m_coeffC);
		UpdateEquation();
	}
}

void CMainDlg::OnOK()
{
}

void CMainDlg::OnKillfocusCoeffA()
{
	OnChangeCoeffA();
}

void CMainDlg::OnKillfocusCoeffB()
{
	OnChangeCoeffB();
}

void CMainDlg::OnKillfocusCoeffC()
{
	OnChangeCoeffC();
}
