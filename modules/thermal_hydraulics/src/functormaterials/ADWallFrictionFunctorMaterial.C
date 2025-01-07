//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallFrictionFunctorMaterial.h"

registerMooseObject("ThermalHydraulicsApp", ADWallFrictionFunctorMaterial);

InputParameters
ADWallFrictionFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Defines a Darcy friction factor equal to the value of the functor specified");

  params.addRequiredParam<MooseFunctorName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MooseFunctorName>("functor", "Darcy friction factor function");

  return params;
}

ADWallFrictionFunctorMaterial::ADWallFrictionFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _functor(getFunctor<ADReal>("functor")),
    _f_D_name(getParam<MooseFunctorName>("f_D"))
{
  addFunctorProperty<ADReal>(
      _f_D_name, [this](const auto & r, const auto & t) -> ADReal { return _functor(r, t); });
}
