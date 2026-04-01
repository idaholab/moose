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
EquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  ProblemOperator::Init(X);
  GetEquationSystem()->BuildEquationSystem();
  // Assign initial condition as initial guess for non-linear problems
  if ((GetEquationSystem()->_non_linear))
    for (const auto i : index_range(_trial_variables))
      *(GetEquationSystem()->_var_ess_constraints.at(i)) = *_trial_variables[i];
}

void
EquationSystemProblemOperator::Solve()
{
  GetEquationSystem()->FormLinearSystem(_true_x, _true_rhs);

  if (_problem_data.nonlinear_solver->usesExternalLinearSolver() && !_problem_data.jacobian_solver)
    mooseError("The configured MFEM nonlinear solver requires an external linear solver, but "
               "none was "
               "provided.");

  if (_problem_data.nonlinear_solver->usesExternalLinearSolver())
  {
    if (_problem_data.jacobian_solver->isLOR() &&
        GetEquationSystem()->GetTestVarNames().size() > 1)
      mooseError("LOR solve is only supported for single-variable systems");
    _problem_data.jacobian_solver->updateSolver(
        *GetEquationSystem()->_blfs.Get(GetEquationSystem()->GetTestVarNames().at(0)),
        GetEquationSystem()->_ess_tdof_lists.at(0));
    _problem_data.nonlinear_solver->SetPreconditioner(_problem_data.jacobian_solver->getSolver());
  }
  _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem_data.nonlinear_solver->Mult(_true_rhs, _true_x);
  GetEquationSystem()->SetTrialVariablesFromTrueVectors(_true_x);
}

} // namespace Moose::MFEM

#endif
