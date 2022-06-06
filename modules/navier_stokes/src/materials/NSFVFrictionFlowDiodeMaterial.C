//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FrictionFlowDiodeMaterial.h"
#include "NS.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

registerMooseObject("NavierStokesApp", FrictionFlowDiodeMaterial);

InputParameters
FrictionFlowDiodeMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Creates a linear friction material, -K vel_i * |d(i)|, i being the momentum"
                             "component, if the flow is opposite the direction d of the diode");
  params.addRequiredParam<Real>("resistance",
                                "Friction factor multiplying the superficial velocity if the flow "
                                "is in a half plane in the opposite direction of the normal");
  params.addRequiredParam<RealVectorValue>(
      "direction",
      "Normal direction of the diode. Flow is free in this half-plane, "
      "subject to friction in the other halfplane");
  params.addRequiredParam<MooseFunctorName>("base_friction_name",
                                            "Name of the Darcy/linear base friction coefficents");
  params.addRequiredParam<MooseFunctorName>("new_friction_name",
                                            "Name of the compounded linear friction coefficents");
  params.addRequiredParam<MooseFunctorName>(NS::superficial_velocity_x,
                                            "superficial velocity x-component");
  params.addRequiredParam<MooseFunctorName>(NS::superficial_velocity_y,
                                            "superficial velocity y-component");
  params.addRequiredParam<MooseFunctorName>(NS::superficial_velocity_z,
                                            "superficial velocity z-component");

  return params;
}

FrictionFlowDiodeMaterial::FrictionFlowDiodeMaterial(const InputParameters & params)
  : FunctorMaterial(params),
    _direction(getParam<RealVectorValue>("direction")),
    _resistance(getParam<Real>("resistance")),
    _base_friction(getFunctor<ADRealVectorValue>("base_friction_name")),
    _u(getFunctor<ADReal>(NS::superficial_velocity_x)),
    _v(getFunctor<ADReal>(NS::superficial_velocity_y)),
    _w(getFunctor<ADReal>(NS::superficial_velocity_z))
{
  addFunctorProperty<ADRealVectorValue>(
    getParam<MooseFunctorName>("new_friction_name"),
    [this](const auto & r, const auto & t) -> ADRealVectorValue
    {
      return {
        (_u(r, t) * _direction(0) < 0) ?
          _base_friction(r, t)(0) + _resistance * std::abs(_direction(0)) * _u(r, t) :
          _base_friction(r, t)(0),
        (_v(r, t) * _direction(1) < 0) ?
          _base_friction(r, t)(1) + _resistance * std::abs(_direction(1)) * _v(r, t) :
          _base_friction(r, t)(1),
        (_w(r, t) * _direction(2) < 0) ?
          _base_friction(r, t)(2) + _resistance * std::abs(_direction(2)) * _w(r, t):
          _base_friction(r, t)(2)
        };
      });
}
