#include "equation_system_problem_operator.h"

namespace platypus
{
void
EquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->_trial_var_names;
  ProblemOperator::SetGridFunctions();
}

void
EquationSystemProblemOperator::Init(mfem::Vector & X)
{
  ProblemOperator::Init(X);

  GetEquationSystem()->BuildEquationSystem(_problem._bc_map);
}

void
EquationSystemProblemOperator::Solve(mfem::Vector & X)
{
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _trial_variables.at(ind)->MakeRef(
        _trial_variables.at(ind)->ParFESpace(), const_cast<mfem::Vector &>(X), _true_offsets[ind]);
  }
  GetEquationSystem()->BuildEquationSystem(_problem._bc_map);
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);

  _problem._nonlinear_solver->SetSolver(*_problem._jacobian_solver);
  _problem._nonlinear_solver->SetOperator(*GetEquationSystem());
  _problem._nonlinear_solver->Mult(_true_rhs, _true_x);

  GetEquationSystem()->RecoverFEMSolution(_true_x, _problem._gridfunctions);
}

} // namespace platypus