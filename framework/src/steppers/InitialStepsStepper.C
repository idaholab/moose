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

#include "InitialStepsStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<InitialStepsStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<StepperName>("incoming_stepper", "The name of the Stepper to get the current dt from");
  params.addRequiredParam<Real>("dt", "The dt to use for the first n_steps");
  params.addRequiredParam<unsigned int>("n_steps", "The number of startup steps to do");

  return params;
}

InitialStepsStepper::InitialStepsStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _incoming_stepper_dt(getStepperDT("incoming_stepper")),
    _input_dt(getParam<Real>("dt")),
    _n_steps(getParam<unsigned int>("n_steps"))
{
}

Real
InitialStepsStepper::computeInitialDT()
{
  return computeDT();
}

Real
InitialStepsStepper::computeDT()
{
  if ( _step_count  <= _n_steps )
    return std::min(_incoming_stepper_dt, _input_dt);

  return _incoming_stepper_dt;
}

Real
InitialStepsStepper::computeFailedDT()
{
  return computeDT();
}

InitialStepsStepper::~InitialStepsStepper()
{
}
