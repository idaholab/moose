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
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription(
      "Computes a body force due to gravity in Rhie-Chow based simulations.");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<MooseFunctorName>(NS::density, NS::density, "The value for the density");
  return params;
}

INSFVMomentumGravity::INSFVMomentumGravity(const InputParameters & params)
  : FVElementalKernel(params),
    INSFVMomentumResidualObject(*this),
    _gravity(getParam<RealVectorValue>("gravity")),
    _rho(getFunctor<ADReal>(NS::density))
{
}

ADReal
INSFVMomentumGravity::computeQpResidual()
{
  return -_rho(makeElemArg(_current_elem), determineState()) * _gravity(_index);
}
