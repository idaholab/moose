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
  return params;
}

ADCoupledVelocityMaterial::ADCoupledVelocityMaterial(const InputParameters & parameters)
  : Material(parameters),
    _velocity(declareADProperty<RealVectorValue>("velocity")),
    _vel_x(adCoupledFVValue("vel_x")),
    _vel_y(isParamValid("vel_y") ? &adCoupledFVValue("vel_y") : nullptr),
    _vel_z(isParamValid("vel_z") ? &adCoupledFVValue("vel_z") : nullptr)
{
}

void
ADCoupledVelocityMaterial::computeQpProperties()
{
  ADRealVectorValue & velocity = _velocity[_qp];

  velocity(0) = _vel_x[_qp];
  if (_vel_y)
    velocity(1) = (*_vel_y)[_qp];
  if (_vel_z)
    velocity(2) = (*_vel_z)[_qp];
}
