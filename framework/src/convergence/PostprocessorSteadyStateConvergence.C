//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorSteadyStateConvergence.h"

registerMooseObject("MooseApp", PostprocessorSteadyStateConvergence);

InputParameters
PostprocessorSteadyStateConvergence::validParams()
{
  InputParameters params = Convergence::validParams();

  params.addClassDescription(
      "Compares the absolute value of a post-processor to a tolerance for steady-state checks.");

  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "Post-processor to use for convergence criteria");
  params.addRequiredParam<Real>("tolerance", "Tolerance to use for convergence criteria");

  return params;
}

PostprocessorSteadyStateConvergence::PostprocessorSteadyStateConvergence(
    const InputParameters & parameters)
  : Convergence(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _tol(getParam<Real>("tolerance"))
{
}

Convergence::MooseConvergenceStatus
PostprocessorSteadyStateConvergence::checkConvergence(unsigned int iter)
{
  // Often post-processors checking steady conditions are falsely 0 on INITIAL
  if (iter == 0)
    return MooseConvergenceStatus::ITERATING;

  if (std::abs(_postprocessor) <= _tol)
  {
    std::ostringstream oss;
    oss << "Converged due to |post-processor| (" << std::abs(_postprocessor) << ") <= tolerance ("
        << _tol << ").";
    verboseOutput(oss);
    return MooseConvergenceStatus::CONVERGED;
  }
  else
  {
    std::ostringstream oss;
    oss << "Still iterating due to |post-processor| (" << std::abs(_postprocessor)
        << ") > tolerance (" << _tol << ").";
    verboseOutput(oss);
    return MooseConvergenceStatus::ITERATING;
  }
}
