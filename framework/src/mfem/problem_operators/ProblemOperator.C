#ifdef MFEM_ENABLED

#include "ProblemOperator.h"

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
