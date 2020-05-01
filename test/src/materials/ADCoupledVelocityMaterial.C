//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledVelocityMaterial.h"

registerMooseObject("MooseTestApp", ADCoupledVelocityMaterial);

InputParameters
ADCoupledVelocityMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("vel_x", "the x velocity");
  params.addCoupledVar("vel_y", "the y velocity");
  params.addCoupledVar("vel_z", "the z velocity");
  params.addRequiredCoupledVar("rho", "The name of the density variable");
  params.addClassDescription("A material used to create a velocity from coupled variables");
  params.addParam<MaterialPropertyName>(
      "velocity", "velocity", "The name of the velocity material property to create");
  params.addParam<MaterialPropertyName>(
      "rho_u", "rho_u", "The product of the density and the x-velocity component");
  params.addParam<MaterialPropertyName>(
      "rho_v", "rho_v", "The product of the density and the y-velocity component");
  params.addParam<MaterialPropertyName>(
      "rho_w", "rho_w", "The product of the density and the z-velocity component");
  return params;
}

ADCoupledVelocityMaterial::ADCoupledVelocityMaterial(const InputParameters & parameters)
  : Material(parameters),
    _velocity(declareADProperty<RealVectorValue>(getParam<MaterialPropertyName>("velocity"))),
    _rho_u(declareADProperty<Real>(getParam<MaterialPropertyName>("rho_u"))),
    _rho_v(declareADProperty<Real>(getParam<MaterialPropertyName>("rho_v"))),
    _rho_w(declareADProperty<Real>(getParam<MaterialPropertyName>("rho_w"))),
    _vel_x(adCoupledValue("vel_x")),
    _vel_y(isParamValid("vel_y") ? &adCoupledValue("vel_y") : nullptr),
    _vel_z(isParamValid("vel_z") ? &adCoupledValue("vel_z") : nullptr),
    _rho(adCoupledValue("rho"))
{
}

void
ADCoupledVelocityMaterial::computeQpProperties()
{
  ADRealVectorValue & velocity = _velocity[_qp];

  velocity(0) = _vel_x[_qp];
  _rho_u[_qp] = _rho[_qp] * velocity(0);

  velocity(1) = _vel_y ? (*_vel_y)[_qp] : 0;
  _rho_v[_qp] = _vel_y ? _rho[_qp] * velocity(1) : 0;

  velocity(2) = _vel_z ? (*_vel_z)[_qp] : 0;
  _rho_w[_qp] = _vel_z ? _rho[_qp] * velocity(2) : 0;
}
