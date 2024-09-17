#include "problem_builder.h"

namespace platypus
{

Problem::~Problem()
{
  // Ensure that all owned memory is properly freed!
  _f.reset();
  _ode_solver.reset();
}

void
ProblemBuilder::SetCoefficients(platypus::Coefficients & coefficients)
{
  GetProblem()->_coefficients = coefficients;
}

void
ProblemBuilder::ConstructNonlinearSolver()
{
  auto nl_solver = std::make_shared<mfem::NewtonSolver>(GetProblem()->_comm);

  // Defaults to one iteration, without further nonlinear iterations
  nl_solver->SetRelTol(0.0);
  nl_solver->SetAbsTol(0.0);
  nl_solver->SetMaxIter(1);

  GetProblem()->_nonlinear_solver = nl_solver;
}

void
ProblemBuilder::InitializeKernels()
{
}

void
ProblemBuilder::InitializeOutputs()
{
  GetProblem()->_outputs.Init(GetProblem()->_gridfunctions);
}

void
ProblemBuilder::FinalizeProblem(bool build_operator)
{
  RegisterFESpaces();
  RegisterGridFunctions();
  RegisterCoefficients();

  if (build_operator)
  {
    ConstructOperator();
  }

  InitializeKernels();
  SetOperatorGridFunctions();

  ConstructNonlinearSolver();

  ConstructState();
  ConstructTimestepper();
  // InitializeOutputs();
}

} // namespace platypus
