//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVFrictionMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVFrictionMaterial);

InputParameters
PINSFVFrictionMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the friction factor for simple cases.");
  params.addRequiredParam<MooseFunctorName>("u", "Velocity functor in direction x.");
  params.addParam<MooseFunctorName>("v", "Velocity functor in direction y.");
  params.addParam<MooseFunctorName>("w", "Velocity functor in direction z.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The liquid dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The liquid density.");
  params.addRequiredParam<MooseFunctorName>("characteristic_lenght",
                                            "The characteristic length in the domain.");
  params.addRequiredParam<MooseFunctorName>("c1", "Constant in c1/Re^(c2) expression.");
  params.addRequiredParam<MooseFunctorName>("c2", "Constant in c1/Re^(c2) expression.");
  params.addParam<MooseFunctorName>("porosity", "1", "prosity.");
  params.addRequiredParam<MooseFunctorName>("friction_factor_name", "prosity.");
  return params;
}

PINSFVFrictionMaterial::PINSFVFrictionMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _mu(getFunctor<ADReal>(NS::mu)),
    _rho(getFunctor<ADReal>(NS::density)),
    _characteristic_length(getFunctor<ADReal>("characteristic_lenght")),
    _c1(getFunctor<ADReal>("c1")),
    _c2(getFunctor<ADReal>("c2")),
    _porosity(getFunctor<ADReal>("porosity")),
    _friction_factor_name(getParam<std::string>("friction_factor_name"))
{
  addFunctorProperty<ADReal>(_friction_factor_name,
                             [this](const auto & r, const auto & t) -> ADReal
                             {
                               ADRealVectorValue velocity;
                               velocity(0) = _u(r, t);
                               if (_v)
                                 velocity(1) = (*_v)(r, t);
                               if (_w)
                                 velocity(2) = (*_w)(r, t);

                               const auto Re = _rho(r, t) * _characteristic_length(r, t) *
                                               velocity.norm() / _mu(r, t);

                               return _c1(r, t) / (pow(Re, _c2(r, t))) * velocity.norm();
                             });
}
