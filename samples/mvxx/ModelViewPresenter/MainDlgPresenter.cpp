#include "stdafx.h"
#include "MainDlgPresenter.h"
#include "IMainDlgView.h"
#include "EquationSolver.h"
#include "IChartView.h"

CMainDlgPresenter::CMainDlgPresenter(CEquationSolver & solver, IMainDlgView & view)
	: m_solver(solver)
	, m_view(view)
{
	m_view.DoOnCoeffAChange(std::bind(&CEquationSolver::SetQuadraticCoeff, &m_solver, std::placeholders::_1));
	m_view.DoOnCoeffBChange(std::bind(&CEquationSolver::SetLinearCoeff, &m_solver, std::placeholders::_1));
	m_view.DoOnCoeffCChange(std::bind(&CEquationSolver::SetConstantCoeff, &m_solver, std::placeholders::_1));
	m_view.DoOnInit(std::bind(&CMainDlgPresenter::InitView, this));
	m_solver.DoOnSolutionChange([this] {
		UpdateSolution();
		UpdateChart();
	});
}

void CMainDlgPresenter::InitView()
{
	m_view.SetCoeffs(m_solver.GetQuadraticCoeff(), m_solver.GetLinearCoeff(), m_solver.GetConstantCoeff());
	UpdateSolution();
	UpdateChart();
}

void CMainDlgPresenter::UpdateSolution()
{
	struct SolutionPrinter
	{
		IMainDlgView & view;
		SolutionPrinter(IMainDlgView & view)
			:view(view)
		{
		}
		void operator()(NoRealRoots)
		{
			view.SetNoSolution();
		}
		void operator()(InfiniteNumberOfRoots)
		{
			view.SetInfiniteSolutions();
		}
		void operator()(double root)
		{
			view.SetSingleSolution(root);
		}
		void operator()(const std::pair<double, double> & roots)
		{
			view.SetTwoRootsSolutuion(roots.first, roots.second);
		}
	};

	SolutionPrinter printer(m_view);
	std::visit(printer, m_solver.GetEquationRoots());
}

void CMainDlgPresenter::UpdateChart()
{
	IChartView::Points2D points;
	for (double x = -10.0; x <= +10; x += 0.1)
	{
		points.emplace_back(x, m_solver.CalculateValue(x));
	}
	m_view.GetChartView().SetData(points);
}

