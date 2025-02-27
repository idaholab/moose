#ifdef MFEM_ENABLED

#include "problem_operator.h"

namespace MooseMFEM
{

void
ProblemOperator::SetGridFunctions()
{
  ProblemOperatorInterface::SetGridFunctions();
  width = height = _block_true_offsets[_trial_variables.size()];
};

} // namespace MooseMFEM

#endif
