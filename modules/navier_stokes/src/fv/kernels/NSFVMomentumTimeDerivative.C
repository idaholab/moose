//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMomentumTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVMomentumTimeDerivative);

InputParameters
NSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  return params;
}

NSFVMomentumTimeDerivative::NSFVMomentumTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params), _rho(getADMaterialProperty<Real>("rho_name"))
{
}

ADReal
NSFVMomentumTimeDerivative::computeQpResidual()
{
  return _rho[_qp] * FVTimeKernel::computeQpResidual();
}
