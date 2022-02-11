//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrandtlNumberMaterial.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", PrandtlNumberMaterial);

InputParameters
PrandtlNumberMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("cp", "Constant-pressure specific heat");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity");
  params.addRequiredParam<MaterialPropertyName>("k", "Thermal conductivity");
  params.addClassDescription("Computes Prandtl number as material property");
  return params;
}

PrandtlNumberMaterial::PrandtlNumberMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Pr(declareProperty<Real>("Pr")),
    _cp(getMaterialProperty<Real>("cp")),
    _mu(getMaterialProperty<Real>("mu")),
    _k(getMaterialProperty<Real>("k"))
{
}

void
PrandtlNumberMaterial::computeQpProperties()
{
  _Pr[_qp] = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
}
