//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothTransitionFunction.h"

InputParameters
SmoothTransitionFunction::validParams()
{
  InputParameters params = Function::validParams();

  MooseEnum axis("x=0 y=1 z=2 t=3");
  params.addRequiredParam<MooseEnum>(
      "axis", axis, "Coordinate axis on which the transition occurs");
  params.addRequiredParam<FunctionName>("function1", "First function");
  params.addRequiredParam<FunctionName>("function2", "Second function");
  params.addRequiredParam<Real>("transition_center", "Center position of transition");
  params.addRequiredParam<Real>("transition_width", "Width of transition");

  return params;
}

SmoothTransitionFunction::SmoothTransitionFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),

    _component(getParam<MooseEnum>("axis")),
    _use_time(_component == 3),

    _function1(getFunction("function1")),
    _function2(getFunction("function2")),

    _x_center(getParam<Real>("transition_center")),
    _transition_width(getParam<Real>("transition_width"))
{
}
