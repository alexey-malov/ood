#pragma once

#include "Signals.h"

struct NoRealRoots {};
struct InfiniteNumberOfRoots {};

using EquationRoots = std::variant<NoRealRoots, InfiniteNumberOfRoots, double, std::pair<double, double>>;

class CEquationSolver
{
public:
	CEquationSolver();
	~CEquationSolver();

	using SolutionChangeSignal = sig::signal<void()>;
	sig::connection DoOnSolutionChange(const SolutionChangeSignal::slot_type & handler);

	EquationRoots GetEquationRoots()const;

	double GetQuadraticCoeff()const;
	void SetQuadraticCoeff(double a);

	double GetLinearCoeff()const;
	void SetLinearCoeff(double b);

	double GetConstantCoeff()const;
	void SetConstantCoeff(double c);
private:
	double m_a = 0;
	double m_b = 0;
	double m_c = 0;

	SolutionChangeSignal m_solutionChanged;
};

