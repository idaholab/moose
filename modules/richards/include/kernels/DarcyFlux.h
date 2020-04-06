//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations

/**
 * Kernel = grad(permeability*(grad(pressure) - weight))
 * This is mass flow according to the Darcy equation
 */
class DarcyFlux : public Kernel
{
public:
  static InputParameters validParams();

  DarcyFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /// fluid weight (gravity*density) as a vector pointing downwards, eg '0 0 -10000'
  RealVectorValue _fluid_weight;

  /// fluid dynamic viscosity
  Real _fluid_viscosity;

  /// Material permeability
  const MaterialProperty<RealTensorValue> & _permeability;
};
