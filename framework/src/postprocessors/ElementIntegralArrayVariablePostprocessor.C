//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralArrayVariablePostprocessor.h"

registerMooseObject("MooseApp", ElementIntegralArrayVariablePostprocessor);

InputParameters
ElementIntegralArrayVariablePostprocessor::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the array variable that this object operates on");
  params.addParam<unsigned int>("component", 0, "Component of the array variable to be integrated");
  params.addClassDescription("Integral of one component of an array variable.");
  return params;
}

ElementIntegralArrayVariablePostprocessor::ElementIntegralArrayVariablePostprocessor(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    MooseVariableInterface<RealEigenVector>(
        this, false, "variable", Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ARRAY),
    _u(coupledArrayValue("variable")),
    _grad_u(coupledArrayGradient("variable")),
    _component(getParam<unsigned int>("component"))
{
  addMooseVariableDependency(&mooseVariableField());
}

Real
ElementIntegralArrayVariablePostprocessor::computeQpIntegral()
{
  return _u[_qp](_component);
}
