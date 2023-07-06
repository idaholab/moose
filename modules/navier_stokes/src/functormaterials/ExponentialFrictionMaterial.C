//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExponentialFrictionMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", ExponentialFrictionMaterial);

InputParameters
ExponentialFrictionMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a Reynolds number-exponential friction factor.");
  params.addRequiredParam<MooseFunctorName>(NS::Reynolds, "The Reynolds number.");
  params.addParam<MooseFunctorName>(NS::speed, "The velocity magnitude of the fluid.");
  params.addRequiredParam<Real>("c1", "c2 in c1/Re^(c2) expression.");
  params.addRequiredParam<Real>("c2", "c2 in c1/Re^(c2) expression.");
  params.addRequiredParam<std::string>("friction_factor_name",
                                       "The name of the output friction factor.");
  params.addParam<bool>(
      "include_velocity_factor",
      false,
      "If a factor of velocity magnitude should be included in the friction factor. This is "
      "typically the case for prorous medium Forcheimer friction therms.");
  return params;
}

ExponentialFrictionMaterial::ExponentialFrictionMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _Re(getFunctor<ADReal>(NS::Reynolds)),
    _speed(isParamValid(NS::speed) ? &getFunctor<ADReal>(NS::speed) : nullptr),
    _c1(getParam<Real>("c1")),
    _c2(getParam<Real>("c2")),
    _friction_factor_name(getParam<std::string>("friction_factor_name")),
    _include_velocity_factor(getParam<bool>("include_velocity_factor"))
{
  if (_include_velocity_factor && !_speed)
    paramError(NS::speed,
               "To be able to include an additional multiplier in the friction factor, please "
               "provide the speed of the fluid.");
  addFunctorProperty<ADReal>(_friction_factor_name,
                             [this](const auto & r, const auto & t) -> ADReal
                             {
                               return _c1 * std::pow(_Re(r, t), _c2) *
                                      (_include_velocity_factor ? (*_speed)(r, t) : ADReal(1.0));
                             });
}
