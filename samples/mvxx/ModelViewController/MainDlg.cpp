
// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MVCApp.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "EquationSolver.h"
#include "IMainDlgController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainDlg::CMainDlg(CEquationSolver & solver, IMainDlgController & controller, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MODELVIEWCONTROLLER_DIALOG, pParent)
	, m_controller(controller)
	, m_solver(solver)
	, m_coeffA(solver.GetQuadraticCoeff())
	, m_coeffB(solver.GetLinearCoeff())
	, m_coeffC(solver.GetConstantCoeff())
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BOOL CMainDlg::PreTranslateMessage(MSG * msg)
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_solutionChangeConnection = m_solver.DoOnSolutionChange([=] {
		UpdateEquation();
	});

	UpdateEquation();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMainDlg::SetSolutionText(const std::wstring & text)
{
	SetDlgItemText(IDC_SOLUTION, text.c_str());
}

void CMainDlg::SetEquationText(const std::wstring & text)
{
	SetDlgItemText(IDC_EQUATION, text.c_str());
}

void CMainDlg::UpdateEquation()
{
	auto solution = m_solver.GetEquationRoots();
	struct SolutionPrinter : boost::static_visitor<void>
	{
		CMainDlg & self;
		SolutionPrinter(CMainDlg &self)
			:self(self)
		{
		}
		void operator()(NoRealRoots)
		{
			self.SetSolutionText(L"No real roots");
		}
		void operator()(InfiniteNumberOfRoots)
		{
			self.SetSolutionText(L"Infinite number of roots");
		}
		void operator()(double root)
		{
			self.SetSolutionText(std::format(L"One root: %1%", root));
		}
		void operator()(const std::pair<double, double> & roots)
		{
			self.SetSolutionText(std::format(L"Two roots: %1% and %2%", roots.first, roots.second));
		}
	};

	SolutionPrinter printer(*this);
	std::visit(printer, solution);

	auto ToSignedString = [](double value) {
		std::wostringstream strm;
		strm << std::abs(value);

		return ((value < 0) ? L"- " : L"+ ") + strm.str();
	};

	SetEquationText(std::format(L"{0}x\u00b2 {1}x {2} = 0", 
		m_solver.GetQuadraticCoeff(), 
		ToSignedString(m_solver.GetLinearCoeff()),
		ToSignedString(m_solver.GetConstantCoeff())));
}

void CMainDlg::OnChangeCoeffA()
{
	if (UpdateData())
	{
		m_controller.SetCoeffA(m_coeffA);
	}
}

void CMainDlg::OnChangeCoeffB()
{
	if (UpdateData())
	{
		m_controller.SetCoeffB(m_coeffB);
	}
}

void CMainDlg::OnChangeCoeffC()
{
	if (UpdateData())
	{
		m_controller.SetCoeffC(m_coeffC);
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
