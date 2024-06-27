#include "equation_system_problem_operator.hpp"

namespace hephaestus
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

  GetEquationSystem()->BuildEquationSystem(_problem._bc_map, _problem._sources);
}

} // namespace hephaestus