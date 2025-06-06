//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HydraulicDiameterCircularMaterial.h"

registerMooseObject("ThermalHydraulicsApp", HydraulicDiameterCircularMaterial);

InputParameters
HydraulicDiameterCircularMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Defines a circular-equivalent hydraulic diameter from the local area");
  params.addRequiredParam<MaterialPropertyName>("D_h_name",
                                                "Hydraulic diameter material property name");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

HydraulicDiameterCircularMaterial::HydraulicDiameterCircularMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _D_h(declareProperty<Real>(getParam<MaterialPropertyName>("D_h_name"))),
    _area(coupledValue("A"))
{
}

void
HydraulicDiameterCircularMaterial::computeQpProperties()
{
  _D_h[_qp] = std::sqrt(4. * _area[_qp] / libMesh::pi);
}
