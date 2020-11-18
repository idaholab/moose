//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVEnergyTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", FVEnergyTimeDerivative);

InputParameters
FVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  return params;
}

FVEnergyTimeDerivative::FVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _cp(getADMaterialProperty<Real>("cp_name"))
{
}

ADReal
FVEnergyTimeDerivative::computeQpResidual()
{
  return _rho[_qp] * _cp[_qp] * FVTimeKernel::computeQpResidual();
}
