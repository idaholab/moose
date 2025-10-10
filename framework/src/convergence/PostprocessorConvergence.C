//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addRequiredParam<Real>("tolerance", "Absolute tolerance to use for convergence criteria");
  params.addParam<unsigned int>(
      "max_diverging_iterations",
      std::numeric_limits<unsigned int>::max(),
      "Number of consecutive iterations of the post-processor value either increasing or not "
      "reducing fast enough, at which to consider the solve diverged");
  params.addParam<Real>(
      "diverging_iteration_rel_reduction",
      0.0,
      "The relative reduction with respect to the previous iteration, for which the iteration "
      "counts as a diverging iteration with respect to 'max_diverging_iterations': an iteration is "
      "diverging if (|pp_old| - |pp_new|)/|pp_old| < rel_reduction. The default value of 0 "
      "corresponds to checking that the iteration is actually converging, whereas values < 1 can "
      "be used to additionally check that the iteration is converging at an acceptable rate.");

  return params;
}

PostprocessorConvergence::PostprocessorConvergence(const InputParameters & parameters)
  : IterationCountConvergence(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _tol(getParam<Real>("tolerance")),
    _max_diverging_iterations(getParam<unsigned int>("max_diverging_iterations")),
    _div_rel_reduction(getParam<Real>("diverging_iteration_rel_reduction")),
    _diverging_iterations(0),
    _pp_value_old(std::numeric_limits<Real>::max())
{
}

Convergence::MooseConvergenceStatus
PostprocessorConvergence::checkConvergenceInner(unsigned int iter)
{
  const auto pp_value = std::abs(_postprocessor);

  // Initialize diverging iteration data
  if (iter == 0)
  {
    _diverging_iterations = 0;
    _pp_value_old = std::numeric_limits<Real>::max();
  }

  // Check for diverging iteration
  const Real rel_reduction = (_pp_value_old - pp_value) / _pp_value_old;
  if (rel_reduction < _div_rel_reduction)
  {
    _diverging_iterations++;

    std::ostringstream oss;
    oss << COLOR_YELLOW << "Diverging iteration: (" << _pp_value_old << "-" << pp_value << ")/"
        << _pp_value_old << " = " << rel_reduction << " < " << _div_rel_reduction << "."
        << COLOR_DEFAULT;
    verboseOutput(oss);
  }
  else
    _diverging_iterations = 0;
  _pp_value_old = pp_value;

  if (pp_value <= _tol)
  {
    std::ostringstream oss;
    oss << COLOR_GREEN << "Converged due to |post-processor| (" << pp_value << ") <= tolerance ("
        << _tol << ")." << COLOR_DEFAULT;
    verboseOutput(oss);
    return MooseConvergenceStatus::CONVERGED;
  }
  else if (_diverging_iterations > _max_diverging_iterations)
  {
    std::ostringstream oss;
    oss << COLOR_RED << "Diverged due to exceeding maximum number of diverging iterations ("
        << _max_diverging_iterations << ")." << COLOR_DEFAULT;
    verboseOutput(oss);
    return MooseConvergenceStatus::DIVERGED;
  }
  else
  {
    std::ostringstream oss;
    oss << "Still iterating due to |post-processor| (" << pp_value << ") > tolerance (" << _tol
        << ").";
    verboseOutput(oss);
    return MooseConvergenceStatus::ITERATING;
  }
}
