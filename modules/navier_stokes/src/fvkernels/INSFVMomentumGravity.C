//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumGravity.h"

registerMooseObject("NavierStokesApp", INSFVMomentumGravity);

InputParameters
INSFVMomentumGravity::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Computes a body force due to gravity.");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  return params;
}

INSFVMomentumGravity::INSFVMomentumGravity(const InputParameters & params)
  : FVElementalKernel(params),
    _gravity(getParam<RealVectorValue>("gravity")),
    _rho(getParam<Real>("rho")),
    _index(getParam<MooseEnum>("momentum_component"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVMomentumGravity::computeQpResidual()
{
  return -_rho * _gravity(_index);
}
