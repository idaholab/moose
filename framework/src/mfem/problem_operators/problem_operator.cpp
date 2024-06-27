#include "problem_operator.hpp"

namespace hephaestus
{

void
ProblemOperator::SetGridFunctions()
{
  ProblemOperatorInterface::SetGridFunctions();
  width = height = _true_offsets[_trial_variables.size()];
};

} // namespace hephaestus