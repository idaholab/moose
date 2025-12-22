//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ComplexEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
ComplexEquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->GetTrialVarNames();
  _test_var_names = GetEquationSystem()->GetTestVarNames();

  _cmplx_trial_variables = _problem_data.cmplx_gridfunctions.Get(_trial_var_names);
  _cmplx_test_variables = _problem_data.cmplx_gridfunctions.Get(_test_var_names);

  // Set operator size and block structure for trial spaces
  _block_true_offsets_trial.SetSize(_cmplx_trial_variables.size() + 1);
  _block_true_offsets_trial[0] = 0;
  for (unsigned int ind = 0; ind < _cmplx_trial_variables.size(); ++ind)
  {
    _block_true_offsets_trial[ind + 1] = 2 * _cmplx_trial_variables.at(ind)->ParFESpace()->TrueVSize();
  }
  _block_true_offsets_trial.PartialSum();

  // Set operator size and block structure for test spaces
  _block_true_offsets_test.SetSize(_cmplx_test_variables.size() + 1);
  _block_true_offsets_test[0] = 0;
  for (unsigned int ind = 0; ind < _cmplx_test_variables.size(); ++ind)
  {
    _block_true_offsets_test[ind + 1] = 2 * _cmplx_test_variables.at(ind)->ParFESpace()->TrueVSize();
  }
  _block_true_offsets_test.PartialSum();


  _true_x.Update(_block_true_offsets_trial);
  _true_rhs.Update(_block_true_offsets_test);

  width = _block_true_offsets_trial[_cmplx_trial_variables.size()];
  height = _block_true_offsets_test[_cmplx_test_variables.size()];
}

void
ComplexEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  X.Update(_block_true_offsets_trial);
  X = 0.0;
  GetEquationSystem()->BuildEquationSystem();
}

void
ComplexEquationSystemProblemOperator::Solve()
{
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);

  if (_problem_data.jacobian_solver->isLOR())
    mooseError("LOR solve is not supported for complex equation systems.");

  _problem_data.nonlinear_solver->SetSolver(_problem_data.jacobian_solver->getSolver());
  _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem_data.nonlinear_solver->Mult(_true_rhs, _true_x);

  GetEquationSystem()->RecoverComplexFEMSolution(
      _true_x, _problem_data.gridfunctions, _problem_data.cmplx_gridfunctions);
}

} // namespace Moose::MFEM

#endif
