//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallFrictionChurchillFunctorMaterial.h"
#include "WallFrictionModels.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallFrictionChurchillFunctorMaterial);

InputParameters
ADWallFrictionChurchillFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the Darcy friction factor using the Churchill correlation.");
  params.addRequiredParam<MooseFunctorName>("rho", "Density");
  params.addRequiredParam<MooseFunctorName>("vel", "x-component of the velocity");
  params.addRequiredParam<MooseFunctorName>("D_h", "hydraulic diameter");

  params.addRequiredParam<MooseFunctorName>("f_D", "Darcy friction factor material property");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity material property");

  params.addParam<Real>("roughness", 0, "Surface roughness");
  params.declareControllable("roughness");
  return params;
}

ADWallFrictionChurchillFunctorMaterial::ADWallFrictionChurchillFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _f_D_name(getParam<MooseFunctorName>("f_D")),
    _mu(getFunctor<ADReal>("mu")),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _D_h(getFunctor<ADReal>("D_h")),
    _roughness(getParam<Real>("roughness"))
{
  addFunctorProperty<ADReal>(
      _f_D_name,
      [this](const auto & r, const auto & t) -> ADReal
      {
        ADReal Re = THM::Reynolds(1, _rho(r, t), _vel(r, t), _D_h(r, t), _mu(r, t));

        const ADReal f_F = WallFriction::FanningFrictionFactorChurchill(Re, _roughness, _D_h(r, t));

        return WallFriction::DarcyFrictionFactor(f_F);
      });
}
