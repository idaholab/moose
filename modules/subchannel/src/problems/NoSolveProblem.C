#include "NoSolveProblem.h"

registerMooseObject("SubChannelApp", NoSolveProblem);

InputParameters
NoSolveProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  return params;
}

NoSolveProblem::NoSolveProblem(const InputParameters & params) : ExternalProblem(params) {}

void
NoSolveProblem::externalSolve()
{
}

void NoSolveProblem::syncSolutions(Direction /*direction*/) {}

bool
NoSolveProblem::converged()
{
  return true;
}
