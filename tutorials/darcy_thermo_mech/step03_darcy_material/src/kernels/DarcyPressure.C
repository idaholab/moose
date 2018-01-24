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

  // No parameters are necessary here because we're going to get
  // permeability and viscosity from the Material
  // so we just return params...
  return params;
}

DarcyPressure::DarcyPressure(const InputParameters & parameters)
  : Diffusion(parameters),

    // Get the permeability and viscosity from the Material system
    // This returns a MaterialProperty<Real> reference that we store
    // in the class and then index into in computeQpResidual/Jacobian....
    _permeability(getMaterialProperty<Real>("permeability")),
    _viscosity(getMaterialProperty<Real>("viscosity"))
{
}

Real
DarcyPressure::computeQpResidual()
{
  // Use the MaterialProperty references we stored earlier
  return (_permeability[_qp] / _viscosity[_qp]) * Diffusion::computeQpResidual();
}

Real
DarcyPressure::computeQpJacobian()
{
  // Use the MaterialProperty references we stored earlier
  return (_permeability[_qp] / _viscosity[_qp]) * Diffusion::computeQpJacobian();
}
