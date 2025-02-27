#ifdef MFEM_ENABLED

#include "TimeDomainEquationSystemProblemOperator.h"

namespace MooseMFEM
{

void
TimeDomainEquationSystemProblemOperator::SetGridFunctions()
{
  _test_var_names = GetEquationSystem()->TestVarNames();
  _trial_var_names = GetEquationSystem()->TrialVarNames();
  _trial_variable_time_derivatives =
      _problem.gridfunctions.Get(GetEquationSystem()->TrialVarTimeDerivativeNames());

  TimeDomainProblemOperator::SetGridFunctions();
}

void
TimeDomainEquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  TimeDomainProblemOperator::Init(X);
  GetEquationSystem()->BuildEquationSystem(_problem.bc_map);
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
  const double time = GetTime();
  for (auto & coef : _problem.scalar_manager)
  {
    coef->SetTime(time);
  }
  for (auto & coef : _problem.vector_manager)
  {
    coef->SetTime(time);
  }
  for (auto & coef : _problem.matrix_manager)
  {
    coef->SetTime(time);
  }
  BuildEquationSystemOperator(dt);

  _problem.nonlinear_solver->SetSolver(*_problem.jacobian_solver);
  _problem.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem.nonlinear_solver->Mult(_true_rhs, dX_dt);
  SetTrialVariablesFromTrueVectors();
}

void
TimeDomainEquationSystemProblemOperator::BuildEquationSystemOperator(double dt)
{
  GetEquationSystem()->SetTimeStep(dt);
  GetEquationSystem()->UpdateEquationSystem(_problem.bc_map);
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);
}

} // namespace MooseMFEM

#endif
