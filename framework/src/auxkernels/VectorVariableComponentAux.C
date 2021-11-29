//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorVariableComponentAux.h"

registerMooseObject("MooseApp", VectorVariableComponentAux);

InputParameters
VectorVariableComponentAux::validParams()
{
  MooseEnum component("x=0 y=1 z=2");
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Creates a field consisting of one component of a coupled vector variable.");
  params.addRequiredCoupledVar("vector_variable",
                               "The variable from which to compute the component");
  params.addParam<MooseEnum>("component", component, "The component to compute");
  return params;
}

VectorVariableComponentAux::VectorVariableComponentAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nodal_variable_value(_nodal ? &coupledNodalValue<RealVectorValue>("vector_variable")
                                 : nullptr),
    _elemental_variable_value(_nodal ? nullptr : &coupledVectorValue("vector_variable")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
VectorVariableComponentAux::computeValue()
{
  if (_nodal)
    return (*_nodal_variable_value)(_component);
  else
    return (*_elemental_variable_value)[_qp](_component);
}
