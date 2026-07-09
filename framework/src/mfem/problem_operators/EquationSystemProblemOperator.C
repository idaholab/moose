//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "EquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
EquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->GetTrialVarNames();
  _test_var_names = GetEquationSystem()->GetTestVarNames();
  ProblemOperator::SetGridFunctions();
}

void
EquationSystemProblemOperator::Solve()
{
  FormEquationSystemOperator();

  auto * const es = GetEquationSystem();
  SolveWithOperator(*es, _true_rhs, _true_x, es->Nonlinear(), es->GetLinearOperator());

  es->SetTrialVariablesFromTrueVectors(_true_x);
}

void
EquationSystemProblemOperator::FormEquationSystemOperator()
{
  GetEquationSystem()->FormSystem(_true_x, _true_rhs);
}

} // namespace Moose::MFEM

#endif
