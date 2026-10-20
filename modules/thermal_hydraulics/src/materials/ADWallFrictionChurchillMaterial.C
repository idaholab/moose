//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallFrictionChurchillMaterial.h"
#include "WallFrictionModels.h"
#include "Numerics.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADWallFrictionChurchillMaterial);

InputParameters
ADWallFrictionChurchillMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the Darcy friction factor using the Churchill correlation.");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density");
  params.addRequiredParam<MaterialPropertyName>("vel", "x-component of the velocity");
  params.addRequiredParam<MaterialPropertyName>("D_h", "hydraulic diameter");

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity material property");

  params.addParam<FunctionName>("roughness", "0", "Function specifying the surface roughness");
  return params;
}

ADWallFrictionChurchillMaterial::ADWallFrictionChurchillMaterial(const InputParameters & parameters)
  : Material(parameters),
    _f_D_name(getParam<MaterialPropertyName>("f_D")),
    _f_D(declareADProperty<Real>(_f_D_name)),

    _mu(getADMaterialProperty<Real>("mu")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _roughness_fn(getFunction("roughness"))
{
}

void
ADWallFrictionChurchillMaterial::computeQpProperties()
{
  ADReal Re = THM::Reynolds(1, _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);

  const Real roughness = _roughness_fn.value(_t, _q_point[_qp]);
  const ADReal f_F = WallFriction::FanningFrictionFactorChurchill(Re, roughness, _D_h[_qp]);

  _f_D[_qp] = WallFriction::DarcyFrictionFactor(f_F);
}
