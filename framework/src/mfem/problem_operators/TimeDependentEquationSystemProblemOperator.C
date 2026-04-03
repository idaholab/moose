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
  auto & dt = _problem.dt();
  auto & gfs = _problem_data.gridfunctions;
  auto & tdm = _problem_data.time_derivative_map;

  // Initialise time derivative
  for (const auto & trial_var_name : _trial_var_names)
    gfs.GetRef(tdm.getTimeDerivativeName(trial_var_name)) = gfs.GetRef(trial_var_name);

  // Advance time step of the MFEM problem. Time is also updated here, and
  // _problem_operator->SetTime is called inside the ode_solver->Step method to
  // update the time used by time dependent (function) coefficients.
  _problem_data.ode_solver->Step(*_trial_true_vector, _problem.time(), dt);
  // Synchonise time dependent GridFunctions with updated DoF data.
  SetTrialVariablesFromTrueVectors();

  // Set time derivatives
  for (const auto & trial_var_name : _trial_var_names)
    (gfs.GetRef(tdm.getTimeDerivativeName(trial_var_name)) -= gfs.GetRef(trial_var_name)) /= -dt;
}

void
TimeDependentEquationSystemProblemOperator::ImplicitSolve(const mfem::real_t dt,
                                                          const mfem::Vector & X_old,
                                                          mfem::Vector & X_new)
{
  X_new = X_old;

  if ((GetEquationSystem()->nonlinear()))
    for (const auto i : index_range(_trial_variables))
      *(GetEquationSystem()->_var_ess_constraints.at(i)) = *_trial_variables[i];

  _problem_data.coefficients.setTime(GetTime());
  BuildEquationSystemOperator(dt);

  if (GetEquationSystem()->nonlinear())
  {
    if (!_problem_data.nonlinear_solver)
      mooseError("A nonlinear MFEM solve requires a nonlinear solver, but none was provided.");

    if (_problem_data.nonlinear_solver->usesExternalLinearSolver() &&
        !_problem_data.jacobian_solver)
      mooseError("The configured MFEM nonlinear solver requires an external linear solver, but "
                 "none was provided.");

    if (_problem_data.nonlinear_solver->usesExternalLinearSolver())
    {
      if (_problem_data.jacobian_solver->isLOR() &&
          GetEquationSystem()->GetTestVarNames().size() > 1)
        mooseError("LOR solve is only supported for single-variable systems");
      _problem_data.jacobian_solver->updateSolver(
          *GetEquationSystem()->_blfs.Get(GetEquationSystem()->GetTestVarNames().at(0)),
          GetEquationSystem()->_ess_tdof_lists.at(0));
      _problem_data.nonlinear_solver->SetLinearSolver(_problem_data.jacobian_solver->getSolver());
    }
    _problem_data.nonlinear_solver->SetOperator(*GetEquationSystem());
    _problem_data.nonlinear_solver->Mult(_true_rhs, X_new);
  }
  else
  {
    if (!_problem_data.jacobian_solver)
      mooseError("A linear MFEM solve requires a linear solver, but none was provided.");

    if (_problem_data.jacobian_solver->isLOR() && GetEquationSystem()->GetTestVarNames().size() > 1)
      mooseError("LOR solve is only supported for single-variable systems");
    _problem_data.jacobian_solver->updateSolver(
        *GetEquationSystem()->_blfs.Get(GetEquationSystem()->GetTestVarNames().at(0)),
        GetEquationSystem()->_ess_tdof_lists.at(0));
    _problem_data.jacobian_solver->getSolver().SetOperator(
        GetEquationSystem()->GetLinearOperator());
    _problem_data.jacobian_solver->getSolver().Mult(_true_rhs, X_new);
  }
}

void
TimeDependentEquationSystemProblemOperator::BuildEquationSystemOperator(mfem::real_t dt)
{
  GetEquationSystem()->SetTimeStep(dt);
  GetEquationSystem()->BuildEquationSystem();
  GetEquationSystem()->FormLinearSystem(_true_x, _true_rhs);
}

} // namespace Moose::MFEM

#endif
