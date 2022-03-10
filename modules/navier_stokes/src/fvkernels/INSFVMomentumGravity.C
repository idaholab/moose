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
  InputParameters params = INSFVElementalKernel::validParams();
  params.addClassDescription(
      "Computes a body force due to gravity in Rhie-Chow based simulations.");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addParam<MooseFunctorName>(NS::density, NS::density, "The value for the density");
  return params;
}

INSFVMomentumGravity::INSFVMomentumGravity(const InputParameters & params)
  : INSFVElementalKernel(params),
    _gravity(getParam<RealVectorValue>("gravity")),
    _rho(getFunctor<ADReal>(NS::density))
{
}

void
INSFVMomentumGravity::gatherRCData(const Elem & elem)
{
  const auto strong_residual = -_rho(makeElemArg(&elem)) * _gravity(_index);
  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);

  _rc_uo.addToA(&elem, _index, strong_residual);

  processResidual(strong_residual * _assembly.elementVolume(&elem), dof_number);
}
