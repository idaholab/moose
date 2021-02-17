//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMomentumGravity.h"

registerMooseObject("NavierStokesApp", NSFVMomentumGravity);
registerMooseObjectRenamed("NavierStokesApp",
                           INSFVMomentumGravity,
                           "07/01/2021 00:00",
                           NSFVMomentumGravity);

InputParameters
NSFVMomentumGravity::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Computes a body force due to gravity.");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<MaterialPropertyName>(NS::density, NS::density, "The value for the density");
  return params;
}

NSFVMomentumGravity::NSFVMomentumGravity(const InputParameters & params)
  : FVElementalKernel(params),
    _gravity(getParam<RealVectorValue>("gravity")),
    _rho(getADMaterialProperty<Real>(NS::density)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

ADReal
NSFVMomentumGravity::computeQpResidual()
{
  return -_rho[_qp] * _gravity(_index);
}
