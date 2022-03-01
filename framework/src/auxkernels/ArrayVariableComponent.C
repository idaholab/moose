//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayVariableComponent.h"

registerMooseObject("MooseApp", ArrayVariableComponent);

InputParameters
ArrayVariableComponent::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("array_variable", "The name of the array variable");
  params.addParam<unsigned int>("component", 0, "Component of the array variable to be extracted");
  params.addClassDescription("Copy a component of an array variable.");
  return params;
}

ArrayVariableComponent::ArrayVariableComponent(const InputParameters & parameters)
  : AuxKernel(parameters),
    _u(coupledArrayValue("array_variable")),
    _component(getParam<unsigned int>("component"))
{
}

Real
ArrayVariableComponent::computeValue()
{
  return _u[_qp](_component);
}
