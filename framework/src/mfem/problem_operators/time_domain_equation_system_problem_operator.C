#include "time_domain_equation_system_problem_operator.h"

namespace platypus
{

void
TimeDomainEquationSystemProblemOperator::SetGridFunctions()
{
  _test_var_names = GetEquationSystem()->_test_var_names;
  _trial_var_names = GetEquationSystem()->_trial_var_names;
  _trial_variable_time_derivatives =
      _problem._gridfunctions.Get(GetEquationSystem()->_trial_var_time_derivative_names);

  TimeDomainProblemOperator::SetGridFunctions();
}

void
TimeDomainEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  TimeDomainProblemOperator::Init(X);
  GetEquationSystem()->BuildEquationSystem(_problem._bc_map);
}

void
TimeDomainEquationSystemProblemOperator::ImplicitSolve(const double dt,
                                                       const mfem::Vector & X,
                                                       mfem::Vector & dX_dt)
{
  dX_dt = 0.0;
  SetTestVariablesFromTrueVectors();
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _trial_variables.at(ind)->MakeTRef(
        _trial_variables.at(ind)->ParFESpace(), dX_dt, _block_true_offsets[ind]);
  }
  const double time = GetTime();
  for (auto & coef : _problem._scalar_manager)
  {
    coef->SetTime(time);
  }
  for (auto & coef : _problem._vector_manager)
  {
    coef->SetTime(time);
  }
  for (auto & coef : _problem._matrix_manager)
  {
    coef->SetTime(time);
  }
  BuildEquationSystemOperator(dt);

  _problem._nonlinear_solver->SetSolver(*_problem._jacobian_solver);
  _problem._nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem._nonlinear_solver->Mult(_true_rhs, dX_dt);
  SetTrialVariablesFromTrueVectors();
}

void
TimeDomainEquationSystemProblemOperator::BuildEquationSystemOperator(double dt)
{
  GetEquationSystem()->SetTimeStep(dt);
  GetEquationSystem()->UpdateEquationSystem(_problem._bc_map);
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);
}

} // namespace platypus
