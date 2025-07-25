//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TimeDomainEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
TimeDomainEquationSystemProblemOperator::SetGridFunctions()
{
  _test_var_names = GetEquationSystem()->TestVarNames();
  _trial_var_names = GetEquationSystem()->TrialVarNames();
  TimeDomainProblemOperator::SetGridFunctions();
}

void
TimeDomainEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  TimeDomainProblemOperator::Init(X);
  GetEquationSystem()->BuildEquationSystem();
  // Set timestepper
  auto & ode_solver = _problem_data.ode_solver;
  ode_solver = std::make_unique<mfem::BackwardEulerSolver>();
  ode_solver->Init(*(this));
  SetTime(_problem.time());
}

void
TimeDomainEquationSystemProblemOperator::Solve()
{
  // Advance time step of the MFEM problem. Time is also updated here, and
  // _problem_operator->SetTime is called inside the ode_solver->Step method to
  // update the time used by time dependent (function) coefficients.
  _problem_data.ode_solver->Step(_problem_data.f, _problem.time(), _problem.dt());
  // Synchonise time dependent GridFunctions with updated DoF data.
  SetTestVariablesFromTrueVectors();
  // Sync Host/Device
  _problem_data.f.HostRead();
}

void
TimeDomainEquationSystemProblemOperator::ImplicitSolve(const double dt,
                                                       const mfem::Vector & /*X*/,
                                                       mfem::Vector & dX_dt)
{
  dX_dt = 0.0;
  SetTestVariablesFromTrueVectors();
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _trial_variables.at(ind)->MakeTRef(
        _trial_variables.at(ind)->ParFESpace(), dX_dt, _block_true_offsets[ind]);
  }
  _problem_data.coefficients.setTime(GetTime());
  BuildEquationSystemOperator(dt);

  if (_problem_data.jacobian_solver->isLOR() && _equation_system->_test_var_names.size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");

  _problem_data.jacobian_solver->updateSolver(
      *_equation_system->_blfs.Get(_equation_system->_test_var_names.at(0)),
      _equation_system->_ess_tdof_lists.at(0));

  _problem_data.nonlinear_solver->SetSolver(_problem_data.jacobian_solver->getSolver());
  _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem_data.nonlinear_solver->Mult(_true_rhs, dX_dt);
  SetTrialVariablesFromTrueVectors();
}

void
TimeDomainEquationSystemProblemOperator::BuildEquationSystemOperator(double dt)
{
  GetEquationSystem()->SetTimeStep(dt);
  GetEquationSystem()->UpdateEquationSystem();
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);
}

} // namespace Moose::MFEM

#endif
