#include "steady_state_problem_builder.h"

namespace platypus
{

void
SteadyStateProblemBuilder::ConstructOperator()
{
  GetProblem()->ConstructOperator();
}

} // namespace platypus
