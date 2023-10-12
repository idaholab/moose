//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPrandtlNumberMaterial.h"
#include "Numerics.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", ADPrandtlNumberMaterial);

InputParameters
ADPrandtlNumberMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("Pr", "Pr", "Prandlt number property name");
  params.addParam<MaterialPropertyName>("cp",
                                        FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE,
                                        "Constant-pressure specific heat");
  params.addParam<MaterialPropertyName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Dynamic viscosity");
  params.addParam<MaterialPropertyName>(
      "k", FlowModelSinglePhase::THERMAL_CONDUCTIVITY, "Thermal conductivity");
  params.addClassDescription("Computes Prandtl number as material property");
  return params;
}

ADPrandtlNumberMaterial::ADPrandtlNumberMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Pr(declareADProperty<Real>("Pr")),
    _cp(getADMaterialProperty<Real>("cp")),
    _mu(getADMaterialProperty<Real>("mu")),
    _k(getADMaterialProperty<Real>("k"))
{
}

void
ADPrandtlNumberMaterial::computeQpProperties()
{
  _Pr[_qp] = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
}
