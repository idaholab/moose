#include "time_domain_equation_system_problem_builder.h"

namespace platypus
{

void
TimeDomainEquationSystemProblemBuilder::InitializeKernels()
{
  ProblemBuilder::InitializeKernels();

  GetEquationSystem()->Init(
      GetProblem()->_gridfunctions, GetProblem()->_fespaces, GetProblem()->_bc_map);
}

} // namespace platypus
