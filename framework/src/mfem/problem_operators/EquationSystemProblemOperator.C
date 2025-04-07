#ifdef MFEM_ENABLED

#include "EquationSystemProblemOperator.h"

namespace Moose::MFEM
{
void
EquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->TrialVarNames();
  ProblemOperator::SetGridFunctions();
}

void
EquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  ProblemOperator::Init(X);

  GetEquationSystem()->BuildEquationSystem(_problem.bc_map);
}

void
EquationSystemProblemOperator::Solve(mfem::Vector &)
{
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);

  _problem.nonlinear_solver->SetSolver(*_problem.jacobian_solver);
  _problem.nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem.nonlinear_solver->Mult(_true_rhs, _true_x);

  GetEquationSystem()->RecoverFEMSolution(_true_x, _problem.gridfunctions);
}

} // namespace Moose::MFEM

#endif
