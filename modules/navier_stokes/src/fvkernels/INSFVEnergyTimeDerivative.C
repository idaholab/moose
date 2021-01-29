//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVEnergyTimeDerivative);

InputParameters
INSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params), _rho(getParam<Real>("rho")), _cp(getADMaterialProperty<Real>("cp_name"))
{
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  return _rho * _cp[_qp] * FVTimeKernel::computeQpResidual();
}
