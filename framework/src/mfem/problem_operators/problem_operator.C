#include "problem_operator.h"

namespace platypus
{

void
ProblemOperator::SetGridFunctions()
{
  ProblemOperatorInterface::SetGridFunctions();
  width = height = _block_true_offsets[_trial_variables.size()];
};

} // namespace platypus