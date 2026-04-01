//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TimeDependentEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
TimeDependentEquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->GetTrialVarNames();
  _test_var_names = GetEquationSystem()->GetTestVarNames();
  TimeDependentProblemOperator::SetGridFunctions();
}

void
TimeDependentEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  TimeDependentProblemOperator::Init(X);
  GetEquationSystem()->BuildEquationSystem();
  // Set timestepper
  auto & ode_solver = _problem_data.ode_solver;
  ode_solver = std::make_unique<mfem::BackwardEulerSolver>();
  ode_solver->Init(*(this));
  SetTime(_problem.time());
  SetImplicitVariableType(STATE);
}

void
TimeDependentEquationSystemProblemOperator::Solve()
{
  // Initialise time derivative
  for (const auto i : index_range(_trial_var_names))
  {
    auto & trial_var_name = _trial_var_names.at(i);
    auto & time_derivative_name =
        _problem_data.time_derivative_map.getTimeDerivativeName(trial_var_name);
    auto * trial_var = _problem_data.gridfunctions.Get(trial_var_name);
    auto * trial_var_time_derivative = _problem_data.gridfunctions.Get(time_derivative_name);

    *trial_var_time_derivative = *trial_var;
  }
  // Advance time step of the MFEM problem. Time is also updated here, and
  // _problem_operator->SetTime is called inside the ode_solver->Step method to
  // update the time used by time dependent (function) coefficients.
  _problem_data.ode_solver->Step(*_trial_true_vector, _problem.time(), _problem.dt());
  // Synchonise time dependent GridFunctions with updated DoF data.
  SetTrialVariablesFromTrueVectors();

  // Set time derivatives
  for (const auto i : index_range(_trial_var_names))
  {
    auto & trial_var_name = _trial_var_names.at(i);
    auto & time_derivative_name =
        _problem_data.time_derivative_map.getTimeDerivativeName(trial_var_name);
    auto * trial_var = _problem_data.gridfunctions.Get(trial_var_name);
    auto * trial_var_time_derivative = _problem_data.gridfunctions.Get(time_derivative_name);

    *trial_var_time_derivative -= *trial_var;
    *trial_var_time_derivative /= -_problem.dt();
  }
}

void
TimeDependentEquationSystemProblemOperator::ImplicitSolve(const mfem::real_t dt,
                                                          const mfem::Vector & X_old,
                                                          mfem::Vector & X_new)
{
  X_new = X_old;
  SetTrialVariablesFromTrueVectors();

  _problem_data.coefficients.setTime(GetTime());
  BuildEquationSystemOperator(dt);

  if (_problem_data.jacobian_solver->isLOR() && GetEquationSystem()->GetTestVarNames().size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");
  _problem_data.jacobian_solver->updateSolver(
      *GetEquationSystem()->_blfs.Get(GetEquationSystem()->GetTestVarNames().at(0)),
      GetEquationSystem()->_ess_tdof_lists.at(0));

  _problem_data.nonlinear_solver->SetSolver(_problem_data.jacobian_solver->getSolver());
  _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem_data.nonlinear_solver->Mult(_true_rhs, X_new);
  SetTrialVariablesFromTrueVectors();
}

void
TimeDependentEquationSystemProblemOperator::BuildEquationSystemOperator(mfem::real_t dt)
{
  GetEquationSystem()->SetTimeStep(dt);
  GetEquationSystem()->BuildEquationSystem();
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);
}

} // namespace Moose::MFEM

#endif
