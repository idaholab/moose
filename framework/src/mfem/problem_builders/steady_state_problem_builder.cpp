#include "steady_state_problem_builder.hpp"

namespace hephaestus
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

void
SteadyStateProblemBuilder::ConstructState()
{
  auto problem_operator = GetProblem()->GetOperator();

  GetProblem()->_f =
      std::make_unique<mfem::BlockVector>(problem_operator->_true_offsets); // Vector of dofs
  problem_operator->Init(*(GetProblem()->_f)); // Set up initial conditions
}
} // namespace hephaestus
