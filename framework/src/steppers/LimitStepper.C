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

#include "LimitStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<LimitStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<StepperName>("incoming_stepper", "The name of the Stepper to get the current dt from");
  params.addParam<Real>("min", 0, "The minimum allowable dt.");
  params.addParam<Real>("max", std::numeric_limits<Real>::max(), "The maximum allowable dt.");

  return params;
}

LimitStepper::LimitStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _incoming_stepper_dt(getStepperDT("incoming_stepper")),
    _min(getParam<Real>("min")),
    _max(getParam<Real>("max"))
{
}

Real
LimitStepper::computeInitialDT()
{
  return computeDT();
}

Real
LimitStepper::computeDT()
{
  return std::min(std::max(_incoming_stepper_dt, _min), _max);
}

Real
LimitStepper::computeFailedDT()
{
  return computeDT();
}
