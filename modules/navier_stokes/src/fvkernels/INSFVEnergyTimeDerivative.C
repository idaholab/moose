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
  params.addRequiredParam<Real>(NS::density, "The value for the density");
  params.declareControllable(NS::density);
  params.addParam<MooseFunctorName>("cp_name", NS::cp, "The name of the specific heat capacity");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params), _rho(getParam<Real>(NS::density)), _cp(getFunctor<ADReal>("cp_name"))
{
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  return _rho * _cp(makeElemArg(_current_elem)) * FVTimeKernel::computeQpResidual();
}
