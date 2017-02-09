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

#include "ConstantStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<ConstantStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<Real>("dt", "The dt to maintain");

  return params;
}

ConstantStepper::ConstantStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _input_dt(getParam<Real>("dt"))
{
}

Real
ConstantStepper::computeInitialDT()
{
  return _input_dt;
}


Real
ConstantStepper::computeDT()
{
  return _input_dt;
}

Real
ConstantStepper::computeFailedDT()
{
  mooseError("Solve failed!  ConstantStepper does not allow cutting the timestep! \nYou may want to change your Stepper to something more sophisticated like SimpleStepper.");
}
