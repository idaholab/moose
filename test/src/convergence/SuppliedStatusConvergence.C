//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SuppliedStatusConvergence.h"

registerMooseObject("MooseTestApp", SuppliedStatusConvergence);

InputParameters
SuppliedStatusConvergence::validParams()
{
  InputParameters params = IterationCountConvergence::validParams();

  params.addRequiredParam<std::vector<int>>(
      "convergence_statuses",
      "List of convergence status integers for each iteration. Valid values are: -1 for diverged, "
      "0 for iterating, and 1 for converged.");

  params.addClassDescription(
      "Takes a user-supplied vector of convergence statuses for each iteration.");

  return params;
}

SuppliedStatusConvergence::SuppliedStatusConvergence(const InputParameters & parameters)
  : IterationCountConvergence(parameters),
    _convergence_statuses(getParam<std::vector<int>>("convergence_statuses"))
{
}

Convergence::MooseConvergenceStatus
SuppliedStatusConvergence::checkConvergenceInner(unsigned int iter)
{
  if (iter > _convergence_statuses.size() - 1)
    mooseError("Iteration index greater than last index of 'convergence_statuses'.");

  std::ostringstream oss;
  switch (_convergence_statuses[iter])
  {
    case -1:
    {
      oss << "Diverged as specified.";
      verboseOutput(oss);
      return MooseConvergenceStatus::DIVERGED;
      break;
    }
    case 0:
      return MooseConvergenceStatus::ITERATING;
      break;
    case 1:
    {
      oss << "Converged as specified.";
      verboseOutput(oss);
      return MooseConvergenceStatus::CONVERGED;
      break;
    }
    default:
      mooseError("Invalid status integer in 'convergence_statuses'. Valid values are: -1 for "
                 "diverged, 0 for iterating, and 1 for converged.");
  }
}
