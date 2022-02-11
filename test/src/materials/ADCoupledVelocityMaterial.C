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
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("vel_x", "the x velocity");
  params.addParam<MooseFunctorName>("vel_y", "the y velocity");
  params.addParam<MooseFunctorName>("vel_z", "the z velocity");
  params.addRequiredParam<MooseFunctorName>("rho", "The name of the density variable");
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
  : FunctorMaterial(parameters),
    _vel_x(getFunctor<ADReal>("vel_x")),
    _vel_y(isParamValid("vel_y") ? &getFunctor<ADReal>("vel_y") : nullptr),
    _vel_z(isParamValid("vel_z") ? &getFunctor<ADReal>("vel_z") : nullptr),
    _rho(getFunctor<ADReal>("rho"))
{
  addFunctorProperty<ADRealVectorValue>(getParam<MaterialPropertyName>("velocity"),
                                        [this](const auto & r, const auto & t) -> ADRealVectorValue
                                        {
                                          ADRealVectorValue velocity(_vel_x(r, t));
                                          velocity(1) = _vel_y ? (*_vel_y)(r, t) : ADReal(0);
                                          velocity(2) = _vel_z ? (*_vel_z)(r, t) : ADReal(0);
                                          return velocity;
                                        });

  addFunctorProperty<ADReal>(getParam<MaterialPropertyName>("rho_u"),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _rho(r, t) * _vel_x(r, t); });

  addFunctorProperty<ADReal>(getParam<MaterialPropertyName>("rho_v"),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _vel_y ? _rho(r, t) * (*_vel_y)(r, t) : ADReal(0); });

  addFunctorProperty<ADReal>(getParam<MaterialPropertyName>("rho_w"),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _vel_z ? _rho(r, t) * (*_vel_z)(r, t) : ADReal(0); });
}
