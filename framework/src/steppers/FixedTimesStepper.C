/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FixedTimesStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<FixedTimesStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addParam<StepperName>("incoming_stepper", "The name of the Stepper to get the current dt from.");
  params.addRequiredParam<std::vector<Real> >("times", "The values of t");

  return params;
}

FixedTimesStepper::FixedTimesStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _incoming_stepper_dt(getStepperDT("incoming_stepper")),
    _times(getParam<std::vector<Real> >("times"))
{
}

Real
FixedTimesStepper::computeInitialDT()
{
  return computeDT();
}

Real
FixedTimesStepper::computeDT()
{
  if (_times.size() == 0)
    return _incoming_stepper_dt;

  Real tol = _executioner.timestepTol();

  for (auto t : _times)
    if (_time < t - tol)
      return std::min(t - _time, _incoming_stepper_dt);

  return _incoming_stepper_dt;
}

Real
FixedTimesStepper::computeFailedDT()
{
  if (isParamValid("incoming_stepper"))
    return _incoming_stepper_dt;
  else
    return 0.5 * _dt[0];
}

FixedTimesStepper::~FixedTimesStepper()
{
}
