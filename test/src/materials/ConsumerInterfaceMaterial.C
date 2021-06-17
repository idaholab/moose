//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConsumerInterfaceMaterial.h"

registerMooseObject("MooseTestApp", ConsumerInterfaceMaterial);

InputParameters
ConsumerInterfaceMaterial::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_consumed",
      "the material property that will be used to compute the property provided by this");
  params.addRequiredParam<MaterialPropertyName>(
      "prop_produced", "The name of the material property that this material produces");
  return params;
}

ConsumerInterfaceMaterial::ConsumerInterfaceMaterial(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _prop_consumed(getADMaterialProperty<Real>("prop_consumed")),
    _prop_produced(declareADProperty<Real>("prop_produced"))
{
}

void
ConsumerInterfaceMaterial::computeQpProperties()
{
  _prop_produced[_qp] = _prop_consumed[_qp];
}
