//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableGradientComponent.h"

registerMooseObject("MooseApp", VariableGradientComponent);

InputParameters
VariableGradientComponent::validParams()
{
  MooseEnum component("x=0 y=1 z=2");
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Creates a field consisting of one component of the gradient of a coupled variable.");
  params.addRequiredCoupledVar("gradient_variable",
                               "The variable from which to compute the gradient component");
  params.addParam<MooseEnum>("component", component, "The gradient component to compute");
  return params;
}

VariableGradientComponent::VariableGradientComponent(const InputParameters & parameters)
  : AuxKernel(parameters),
    _gradient(coupledGradient("gradient_variable")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
VariableGradientComponent::computeValue()
{
  return _gradient[_qp](_component);
}
