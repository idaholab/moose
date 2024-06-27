#include "steady_state_equation_system_problem_builder.hpp"

namespace hephaestus
{

void
SteadyStateEquationSystemProblemBuilder::InitializeKernels()
{
  ProblemBuilder::InitializeKernels();

  GetEquationSystem()->Init(GetProblem()->_gridfunctions,
                            GetProblem()->_fespaces,
                            GetProblem()->_bc_map,
                            GetProblem()->_coefficients);
}

} // namespace hephaestus
