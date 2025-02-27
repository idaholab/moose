#ifdef MFEM_ENABLED

#include "TimeDomainProblemOperator.h"

namespace MooseMFEM
{
std::vector<std::string>
GetTimeDerivativeNames(std::vector<std::string> gridfunction_names)
{
  std::vector<std::string> time_derivative_names;
  for (auto & gridfunction_name : gridfunction_names)
  {
    time_derivative_names.push_back(GetTimeDerivativeName(gridfunction_name));
  }
  return time_derivative_names;
}

void
TimeDomainProblemOperator::SetGridFunctions()
{
  ProblemOperatorInterface::SetGridFunctions();
  width = height = _block_true_offsets[_trial_variables.size()];
}

} // namespace MooseMFEM

#endif
