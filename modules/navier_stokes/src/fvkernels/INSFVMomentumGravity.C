//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumGravity.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMomentumGravity);

InputParameters
INSFVMomentumGravity::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Computes a body force due to gravity in Rhie-Chow based simulations.");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<MooseFunctorName>(NS::density, NS::density, "The value for the density");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

INSFVMomentumGravity::INSFVMomentumGravity(const InputParameters & params)
  : FVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _rho(getFunctor<ADReal>(NS::density))
{
}

ADReal
INSFVMomentumGravity::computeQpResidual()
{
  return -_rho(_current_elem) * _gravity(_index);
}
