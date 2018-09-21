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
  // we do a bisection search because math is hard
  //

  // this is the target value
  const Real p2n = getParam<Real>("p2nucleus");

  // initial guess
  _max_lambda = 0.1;
  Real step = _max_lambda;
  Real multiplier = 1.0;
  for (unsigned int i = 0; i < 60; ++i)
  {
    const Real p = 1.0 - (1.0 + _max_lambda) * std::exp(-_max_lambda);
    if (MooseUtils::absoluteFuzzyEqual(p, p2n))
      break;
    else if (p > p2n)
    {
      // the interval has no upper end, so we can only start bisecting when we have
      // exceeded the target value at least once
      multiplier = 0.5;
      step *= multiplier;
      _max_lambda -= step;
    }
    else
    {
      step *= multiplier;
      _max_lambda += step;
    }
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
