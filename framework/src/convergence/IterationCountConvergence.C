//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IterationCountConvergence.h"
#include "FixedPointSolve.h"

registerMooseObject("MooseApp", IterationCountConvergence);

InputParameters
IterationCountConvergence::validParams()
{
  InputParameters params = Convergence::validParams();

  params.addParam<unsigned int>("min_iterations", 0, "Minimum number of iterations");
  params.addParam<unsigned int>("max_iterations", 50, "Maximum number of iterations");
  params.addParam<bool>(
      "converge_at_max_iterations", false, "Converge at 'max_iterations' instead of diverging");

  params.addClassDescription("Checks the iteration count.");

  return params;
}

IterationCountConvergence::IterationCountConvergence(const InputParameters & parameters)
  : Convergence(parameters),
    _min_iterations(getParam<unsigned int>("min_iterations")),
    _max_iterations(getParam<unsigned int>("max_iterations")),
    _converge_at_max_iterations(getParam<bool>("converge_at_max_iterations"))
{
  if (_max_iterations < _min_iterations)
    mooseError("'max_iterations' must be >= 'min_iterations'.");
}

Convergence::MooseConvergenceStatus
IterationCountConvergence::checkConvergence(unsigned int n_iter)
{
  const auto status_inner = checkConvergenceInner(n_iter);

  std::ostringstream oss;
  switch (status_inner)
  {
    case MooseConvergenceStatus::ITERATING:
      if (n_iter >= _max_iterations)
      {
        if (_converge_at_max_iterations)
        {
          oss << "Converged due to iterations (" << n_iter << ") >= max iterations ("
              << _max_iterations << ") and 'converge_at_max_iterations' = 'true'.";
          verboseOutput(oss);
          return MooseConvergenceStatus::CONVERGED;
        }
        else
        {
          oss << "Diverged due to iterations (" << n_iter << ") >= max iterations ("
              << _max_iterations << ").";
          verboseOutput(oss);
          return MooseConvergenceStatus::DIVERGED;
        }
      }
      else
        return MooseConvergenceStatus::ITERATING;
      break;

    case MooseConvergenceStatus::CONVERGED:
      if (n_iter < _min_iterations)
      {
        oss << "Still iterating because iterations (" << n_iter << ") < min iterations ("
            << _min_iterations << ").";
        verboseOutput(oss);
        return MooseConvergenceStatus::ITERATING;
      }
      else
        return MooseConvergenceStatus::CONVERGED;
      break;

    case MooseConvergenceStatus::DIVERGED:
      return MooseConvergenceStatus::DIVERGED;
      break;

    default:
      mooseError("Invalid convergence status");
  }
}

Convergence::MooseConvergenceStatus
IterationCountConvergence::checkConvergenceInner(unsigned int /*n_iter*/)
{
  return MooseConvergenceStatus::ITERATING;
}
