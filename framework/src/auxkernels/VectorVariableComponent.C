//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorVariableComponent.h"

registerMooseObject("MooseApp", VectorVariableComponent);

template <>
InputParameters
validParams<VectorVariableComponent>()
{
  MooseEnum component("x=0 y=1 z=2");
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription(
      "Creates a field consisting of one component of a coupled vector variable.");
  params.addRequiredCoupledVar("vector_variable",
                               "The variable from which to compute the component");
  params.addParam<MooseEnum>("component", component, "The component to compute");
  return params;
}

VectorVariableComponent::VectorVariableComponent(const InputParameters & parameters)
  : AuxKernel(parameters),
    _vector_variable_value(coupledNodalValue<RealVectorValue>("vector_variable")),
    _component(getParam<MooseEnum>("component"))
{
  if (!isNodal())
    mooseError("VectorVariableComponent is meant to be used with LAGRANGE_VEC variables and so the "
               "auxiliary variable should be nodal");
}

Real
VectorVariableComponent::computeValue()
{
  return _vector_variable_value(_component);
}
