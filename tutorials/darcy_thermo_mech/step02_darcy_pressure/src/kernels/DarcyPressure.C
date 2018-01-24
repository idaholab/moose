//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyPressure.h"

template <>
InputParameters
validParams<DarcyPressure>()
{
  // Start with the parameters from our parent
  InputParameters params = validParams<Diffusion>();

  // Now add any extra parameters this class needs:

  // Add a required parameter.  If this isn't provided in the input file MOOSE will error.
  params.addRequiredParam<Real>("permeability", "The permeability (K) of the fluid");

  // Add a parameter with a default value.  This value can be overriden in the input file.
  params.addParam<Real>(
      "viscosity", 7.98e-4, "The viscosity (mu) of the fluid.  Default is for 30 degrees C.");

  return params;
}

DarcyPressure::DarcyPressure(const InputParameters & parameters)
  : Diffusion(parameters),

    // Get the parameters from the input file
    _permeability(getParam<Real>("permeability")),
    _viscosity(getParam<Real>("viscosity"))
{
}

Real
DarcyPressure::computeQpResidual()
{
  // K / mu * grad_u * grad_phi[i]
  return (_permeability / _viscosity) * Diffusion::computeQpResidual();
}

Real
DarcyPressure::computeQpJacobian()
{
  // K / mu * grad_phi[j] * grad_phi[i]
  return (_permeability / _viscosity) * Diffusion::computeQpJacobian();
}
