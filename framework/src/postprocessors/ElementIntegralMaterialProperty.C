//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralMaterialProperty.h"

template <>
InputParameters
validParams<ElementIntegralMaterialProperty>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The name of the material property");
  return params;
}

ElementIntegralMaterialProperty::ElementIntegralMaterialProperty(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters), _scalar(getMaterialProperty<Real>("mat_prop"))
{
}

Real
ElementIntegralMaterialProperty::computeQpIntegral()
{
  return _scalar[_qp];
}
