#include "steady_state_problem_builder.h"

namespace platypus
{

void
SteadyStateProblemBuilder::SetOperatorGridFunctions()
{
  GetProblem()->GetOperator()->SetGridFunctions();
}

void
SteadyStateProblemBuilder::ConstructOperator()
{
  GetProblem()->ConstructOperator();
}

} // namespace platypus
