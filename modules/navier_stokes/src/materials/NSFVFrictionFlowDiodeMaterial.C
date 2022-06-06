//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVFrictionFlowDiodeMaterial.h"
#include "NS.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

registerMooseObject("NavierStokesApp", NSFVFrictionFlowDiodeMaterial);

InputParameters
NSFVFrictionFlowDiodeMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Increases the anistropic friction coefficients, linear or quadratic, "
                             "by K_i * |direction_i| when the diode is turned on with a boolean");
  params.addRequiredParam<RealVectorValue>("direction", "Direction of the diode");
  params.addRequiredRangeCheckedParam<RealVectorValue>("additional_linear_resistance",
                                                       "additional_linear_resistance >= 0",
                                                       "Additional linear friction factor");
  params.addRequiredRangeCheckedParam<RealVectorValue>("additional_quadratic_resistance",
                                                       "additional_quadratic_resistance >= 0",
                                                       "Additional quadratic friction factor");
  params.addRequiredParam<MooseFunctorName>(
      "base_linear_friction_coefs", "Name of the base anistropic Darcy/linear friction functor");
  params.addRequiredParam<MooseFunctorName>(
      "base_quadratic_friction_coefs",
      "Name of the base anistropic Forchheimer/quadratic friction functor");
  params.addRequiredParam<MooseFunctorName>("sum_linear_friction_name",
                                            "Name of the additional Darcy/linear friction functor");
  params.addRequiredParam<MooseFunctorName>(
      "sum_quadratic_friction_name",
      "Name of the additional Forchheimer/quadratic friction functor");
  params.addRequiredParam<bool>("turn_on_diode", "Whether to add the additional friction");
  params.declareControllable("turn_on_diode");

  return params;
}

NSFVFrictionFlowDiodeMaterial::NSFVFrictionFlowDiodeMaterial(const InputParameters & params)
  : FunctorMaterial(params),
    _direction(getParam<RealVectorValue>("direction")),
    _linear_resistance(getParam<RealVectorValue>("additional_linear_resistance")),
    _quadratic_resistance(getParam<RealVectorValue>("additional_quadratic_resistance")),
    _base_linear_friction(getFunctor<ADRealVectorValue>("base_linear_friction_coefs")),
    _base_quadratic_friction(getFunctor<ADRealVectorValue>("base_quadratic_friction_coefs")),
    _diode_on(getParam<bool>("turn_on_diode"))
{
  addFunctorProperty<ADRealVectorValue>(
      getParam<MooseFunctorName>("sum_linear_friction_name"),
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        return {_base_linear_friction(r, t)(0) +
                    (_diode_on ? _linear_resistance(0) * std::abs(_direction(0)) : 0),
                _base_linear_friction(r, t)(1) +
                    (_diode_on ? _linear_resistance(1) * std::abs(_direction(1)) : 0),
                _base_linear_friction(r, t)(2) +
                    (_diode_on ? _linear_resistance(2) * std::abs(_direction(2)) : 0)};
      });
  addFunctorProperty<ADRealVectorValue>(
      getParam<MooseFunctorName>("sum_quadratic_friction_name"),
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        return {_base_quadratic_friction(r, t)(0) +
                    (_diode_on ? _quadratic_resistance(0) * std::abs(_direction(0)) : 0),
                _base_quadratic_friction(r, t)(1) +
                    (_diode_on ? _quadratic_resistance(1) * std::abs(_direction(1)) : 0),
                _base_quadratic_friction(r, t)(2) +
                    (_diode_on ? _quadratic_resistance(2) * std::abs(_direction(2)) : 0)};
      });
}
