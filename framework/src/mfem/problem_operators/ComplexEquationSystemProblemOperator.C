//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "ComplexEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
ComplexEquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->TrialVarNames();

  _cpx_trial_variables = _problem.complex_gridfunctions.Get(_trial_var_names);

  // Set operator size and block structure
  _block_true_offsets.SetSize(_cpx_trial_variables.size() + 1);
  _block_true_offsets[0] = 0;
  for (unsigned int ind = 0; ind < _cpx_trial_variables.size(); ++ind)
  {
    _block_true_offsets[ind + 1] = 2 * _cpx_trial_variables.at(ind)->ParFESpace()->TrueVSize();
  }
  _block_true_offsets.PartialSum();

  _true_x.Update(_block_true_offsets);
  _true_rhs.Update(_block_true_offsets);

  width = height = _block_true_offsets[_cpx_trial_variables.size()];
}

void
ComplexEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  X.Update(_block_true_offsets);
  X = 0.0;
  GetEquationSystem()->BuildEquationSystem();
}

void
ComplexEquationSystemProblemOperator::Solve(mfem::Vector &)
{
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);

  if (_problem.jacobian_solver->isLOR())
    mooseError("LOR solve is not supported for complex equation systems.");

  _problem.nonlinear_solver->SetSolver(_problem.jacobian_solver->getSolver());
  _problem.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem.nonlinear_solver->Mult(_true_rhs, _true_x);

  if (auto cpx_eq_sys =
          std::dynamic_pointer_cast<Moose::MFEM::ComplexEquationSystem>(_equation_system))
  {
    cpx_eq_sys->RecoverFEMSolution(_true_x, _problem.complex_gridfunctions);
  }
}

} // namespace Moose::MFEM

#endif
