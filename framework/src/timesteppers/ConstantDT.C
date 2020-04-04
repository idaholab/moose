//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantDT.h"

registerMooseObject("MooseApp", ConstantDT);

InputParameters
ConstantDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addRequiredParam<Real>("dt", "Size of the time step");

  // The addRangeCheckedParam and addClassDescription are used in MOOSE documentation, if you
  // change the order or insert something you will mess it up.
  params.addRangeCheckedParam<Real>(
      "growth_factor",
      2,
      "growth_factor>=1",
      "Maximum ratio of new to previous timestep sizes following a step that required the time"
      " step to be cut due to a failed solve.");
  params.addClassDescription("Timestepper that takes a constant time step size");

  return params;
}

ConstantDT::ConstantDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _constant_dt(getParam<Real>("dt")),
    _growth_factor(getParam<Real>("growth_factor"))
{
}

Real
ConstantDT::computeInitialDT()
{
  return _constant_dt;
}

Real
ConstantDT::computeDT()
{
  return std::min(_constant_dt, _growth_factor * getCurrentDT());
}
