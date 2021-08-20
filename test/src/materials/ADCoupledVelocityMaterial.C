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
  : FunctorMaterial(parameters),
    _velocity(
        declareFunctorProperty<ADRealVectorValue>(getParam<MaterialPropertyName>("velocity"))),
    _rho_u(declareFunctorProperty<ADReal>(getParam<MaterialPropertyName>("rho_u"))),
    _rho_v(declareFunctorProperty<ADReal>(getParam<MaterialPropertyName>("rho_v"))),
    _rho_w(declareFunctorProperty<ADReal>(getParam<MaterialPropertyName>("rho_w"))),
    _vel_x(getFunctor<MooseVariableFVReal>("vel_x", 0)),
    _vel_y(isParamValid("vel_y") ? &getFunctor<MooseVariableFVReal>("vel_y", 0) : nullptr),
    _vel_z(isParamValid("vel_z") ? &getFunctor<MooseVariableFVReal>("vel_z", 0) : nullptr),
    _rho(getFunctor<MooseVariableFVReal>("rho", 0))
{
  _velocity.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADRealVectorValue {
    ADRealVectorValue velocity(_vel_x(geom_quantity));
    velocity(1) = _vel_y ? (*_vel_y)(geom_quantity) : ADReal(0);
    velocity(2) = _vel_z ? (*_vel_z)(geom_quantity) : ADReal(0);
    return velocity;
  });

  _rho_u.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    return _rho(geom_quantity) * _vel_x(geom_quantity);
  });

  _rho_v.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    return _vel_y ? _rho(geom_quantity) * (*_vel_y)(geom_quantity) : ADReal(0);
  });

  _rho_w.setFunctor(_mesh, blockIDs(), [this](auto & geom_quantity) -> ADReal {
    return _vel_z ? _rho(geom_quantity) * (*_vel_z)(geom_quantity) : ADReal(0);
  });
}
