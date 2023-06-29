//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReynoldsNumberFunctorMaterial.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", ReynoldsNumberFunctorMaterial);

InputParameters
ReynoldsNumberFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a Reynolds number.");
  params.addRequiredParam<MooseFunctorName>(NS::speed, "The velocity magnitude of the fluid.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density of the fluid.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The dynamic viscosity of the fluid.");
  params.addRequiredParam<MooseFunctorName>("characteristic_length",
                                            "The characteristic length of the geometry.");

  return params;
}

ReynoldsNumberFunctorMaterial::ReynoldsNumberFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _speed(getFunctor<ADReal>(NS::speed)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _rho(getFunctor<ADReal>(NS::density)),
    _characteristic_length(getFunctor<Real>("characteristic_length"))
{
  addFunctorProperty<ADReal>(
      NS::Reynolds,
      [this](const auto & r, const auto & t) -> ADReal
      { return _rho(r, t) * _speed(r, t) * _characteristic_length(r, t) / _mu(r, t); });
}
