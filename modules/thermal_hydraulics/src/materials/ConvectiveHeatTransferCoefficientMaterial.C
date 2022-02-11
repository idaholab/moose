//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectiveHeatTransferCoefficientMaterial.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ConvectiveHeatTransferCoefficientMaterial);

InputParameters
ConvectiveHeatTransferCoefficientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("Nu", "Nusselt number");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("k", "Thermal conductivity");
  params.addClassDescription("Computes convective heat transfer coefficient from Nusselt number");
  return params;
}

ConvectiveHeatTransferCoefficientMaterial::ConvectiveHeatTransferCoefficientMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareProperty<Real>("Hw")),
    _Nu(getMaterialProperty<Real>("Nu")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _k(getMaterialProperty<Real>("k"))
{
}

void
ConvectiveHeatTransferCoefficientMaterial::computeQpProperties()
{
  _Hw[_qp] = THM::wallHeatTransferCoefficient(_Nu[_qp], _k[_qp], _D_h[_qp]);
}
