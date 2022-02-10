//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionHeatFluxMaterial.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectionHeatFluxMaterial);

InputParameters
ADConvectionHeatFluxMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("q_wall", "Wall heat flux material property");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid phase temperature material property");
  params.addRequiredCoupledVar("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>(
      "htc_wall", "Fluid phase wall heat transfer coefficient material property");
  params.addRequiredParam<MaterialPropertyName>(
      "kappa", "Fluid phase wall contact fraction material property");

  params.addClassDescription("Computes heat flux from convection for a given fluid phase.");

  return params;
}

ADConvectionHeatFluxMaterial::ADConvectionHeatFluxMaterial(const InputParameters & parameters)
  : Material(parameters),

    _q_wall(declareADProperty<Real>(getParam<MaterialPropertyName>("q_wall"))),
    _T(getADMaterialProperty<Real>("T")),
    _T_wall(adCoupledValue("T_wall")),
    _htc_wall(getADMaterialProperty<Real>("htc_wall")),
    _kappa(getADMaterialProperty<Real>("kappa"))
{
}

void
ADConvectionHeatFluxMaterial::computeQpProperties()
{
  _q_wall[_qp] = _kappa[_qp] * _htc_wall[_qp] * (_T_wall[_qp] - _T[_qp]);
}
