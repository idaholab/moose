//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorConvergence.h"

registerMooseObject("MooseApp", PostprocessorConvergence);

InputParameters
PostprocessorConvergence::validParams()
{
  InputParameters params = IterationCountConvergence::validParams();

  params.addClassDescription("Compares the absolute value of a post-processor to a tolerance.");

  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "Post-processor to use for convergence criteria");
  params.addRequiredParam<Real>("tolerance", "Tolerance to use for convergence criteria");

  return params;
}

PostprocessorConvergence::PostprocessorConvergence(const InputParameters & parameters)
  : IterationCountConvergence(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _tol(getParam<Real>("tolerance"))
{
}

Convergence::MooseConvergenceStatus
PostprocessorConvergence::checkConvergenceInner(unsigned int /*iter*/)
{
  if (std::abs(_postprocessor) <= _tol)
  {
    std::ostringstream oss;
    oss << "Converged due to |post-processor| (" << std::abs(_postprocessor) << ") <= tolerance ("
        << _tol << ").";
    verboseOutput(oss);
    return MooseConvergenceStatus::CONVERGED;
  }
  else
    return MooseConvergenceStatus::ITERATING;
}
