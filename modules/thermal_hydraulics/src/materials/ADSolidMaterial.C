//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSolidMaterial.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", ADSolidMaterial);

InputParameters
ADSolidMaterial::validParams()
{
  InputParameters params = Material::validParams();
  // Coupled variables
  params.addRequiredCoupledVar("T", "Temperature in the solid");

  params.addRequiredParam<UserObjectName>(
      "properties", "The name of an user object describing material conductivity");
  return params;
}

ADSolidMaterial::ADSolidMaterial(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareADProperty<Real>(HeatConductionModel::THERMAL_CONDUCTIVITY)),
    _specific_heat(declareADProperty<Real>(HeatConductionModel::SPECIFIC_HEAT_CONSTANT_PRESSURE)),
    _density(declareADProperty<Real>(HeatConductionModel::DENSITY)),
    _temp(adCoupledValue("T")),
    _props(getUserObject<SolidMaterialProperties>("properties"))
{
}

void
ADSolidMaterial::computeQpProperties()
{
  _thermal_conductivity[_qp] = _props.k(_temp[_qp]);
  _specific_heat[_qp] = _props.cp(_temp[_qp]);
  _density[_qp] = _props.rho(_temp[_qp]);
}
