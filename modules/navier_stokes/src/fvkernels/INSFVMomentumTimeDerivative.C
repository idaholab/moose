//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMomentumTimeDerivative);

InputParameters
INSFVMomentumTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  return params;
}

INSFVMomentumTimeDerivative::INSFVMomentumTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params), _rho(getParam<Real>("rho"))
{
}

ADReal
INSFVMomentumTimeDerivative::computeQpResidual()
{
  return _rho * FVTimeKernel::computeQpResidual();
}
