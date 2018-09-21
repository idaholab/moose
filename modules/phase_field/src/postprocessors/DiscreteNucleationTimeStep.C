//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationTimeStep.h"
#include "DiscreteNucleationInserter.h"
#include "MooseUtils.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationTimeStep);

template <>
InputParameters
validParams<DiscreteNucleationTimeStep>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription(
      "Return a timestep limit for nucleation event to be used by IterationAdaptiveDT");
  params.addRequiredParam<Real>("dt_max",
                                "Time step to cut back to at the start of a nucleation event");
  params.addRangeCheckedParam<Real>(
      "p2nucleus",
      0.01,
      "p2nucleus > 0 & p2nucleus < 1",
      "Maximum probability for more than one nucleus to appear during a time "
      "step. This will limit the time step base on the total nucleation rate for "
      "the domain to make sure the probability for two or more nuclei to appear "
      "is always below the chosen number.");
  params.addRequiredParam<UserObjectName>("inserter", "DiscreteNucleationInserter user object");
  return params;
}

DiscreteNucleationTimeStep::DiscreteNucleationTimeStep(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _inserter(getUserObject<DiscreteNucleationInserter>("inserter")),
    _dt_nucleation(getParam<Real>("dt_max"))
{
  //
  // We do a bisection search because math is hard
  // (i.e. probability function is not analytically invertible)
  //

  // this is the target value
  const Real p2n = getParam<Real>("p2nucleus");

  // initial guess
  _max_lambda = 0.1;
  Real upper_bound = _max_lambda;
  Real lower_bound = 0.0;

  // At this point we do not know a proper upper bound for a search interval
  // so we grow our initial guess until we find it (we only allow for a finite
  // number of iterations)
  for (unsigned int i = 0; i < 100; ++i)
  {
    const Real p_upper = 1.0 - (1.0 + upper_bound) * std::exp(-upper_bound);

    // we have found a lambda value that results in a p > p2n, this is the upper end of the interval
    if (p_upper > p2n)
      break;

    // upper_bound is actually below our target lambda, set it as the new lower
    // bound and double the upper_bound value
    lower_bound = upper_bound;
    upper_bound *= 2.0;
  }

  // now that we have an upper and a lower interval bounds we can do a proper bisection
  for (unsigned int i = 0; i < 100; ++i)
  {
    // pick the middle of the current interval
    _max_lambda = (upper_bound - lower_bound) / 2.0 + lower_bound;

    // calculate new probability for 2 or more nuclei to appear
    const Real p = 1.0 - (1.0 + _max_lambda) * std::exp(-_max_lambda);

    // quit if we zeroed in on the target
    if (MooseUtils::absoluteFuzzyEqual(p, p2n))
      break;

    // otherwise adjust interval bounds
    else if (p > p2n)
      upper_bound = _max_lambda;
    else
      lower_bound = _max_lambda;
  }
}

PostprocessorValue
DiscreteNucleationTimeStep::getValue()
{
  if (_inserter.isMapUpdateRequired())
    return _dt_nucleation;

  const Real rate = _inserter.getRate();
  if (rate == 0.0)
    return std::numeric_limits<Real>::max();
  else
    return _max_lambda / rate;
}
