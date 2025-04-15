#ifdef MFEM_ENABLED

#include "ProblemOperator.h"

namespace Moose::MFEM
{

void
ProblemOperator::SetGridFunctions()
{
  ProblemOperatorInterface::SetGridFunctions();
  width = height = _block_true_offsets[_trial_variables.size()];
}

} // namespace Moose::MFEM

#endif
