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
}

void
EquationSystemProblemOperator::Solve()
{

  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);
  if (_problem_data.jacobian_solver->isLOR() && GetEquationSystem()->GetTestVarNames().size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");
  _problem_data.jacobian_solver->updateSolver(
      *GetEquationSystem()->_blfs.Get(GetEquationSystem()->GetTestVarNames().at(0)),
      GetEquationSystem()->_ess_tdof_lists.at(0));
/*
  _problem_data.nonlinear_solver->SetSolver(_problem_data.jacobian_solver->getSolver());
  _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem_data.nonlinear_solver->Mult(_true_rhs, _true_x);
*/
  mfem::Vector zero_vec(_true_rhs.Size());
  zero_vec = 0.0;
  _problem_data.nonlinear_solver->SetSolver(_problem_data.jacobian_solver->getSolver());
  _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());

  if (!(GetEquationSystem()->_non_linear))
    _problem_data.nonlinear_solver->Mult(_true_rhs, _true_x);
  else
    _problem_data.nonlinear_solver->Mult(zero_vec, _true_x);


  GetEquationSystem()->RecoverFEMSolution(
      _true_x, _problem_data.gridfunctions, _problem_data.cmplx_gridfunctions);
}

} // namespace Moose::MFEM

#endif
