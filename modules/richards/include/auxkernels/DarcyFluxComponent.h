//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations

/**
 * Computes a component of the Darcy flux:
 * -k_ij/mu (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * mu is the fluid viscosity,
 * P is the fluid pressure (the variable)
 * and w_j is the fluid weight
 * This is measured in m^3 . s^-1 . m^-2
 *
 * Sometimes the fluid velocity is required,
 * rather than the flux.  In this case
 * velocity_scaling may be used, and the
 * result quoted above is multiplied by
 * (1/velocity_scaling)
 */
class DarcyFluxComponent : public AuxKernel
{
public:
  static InputParameters validParams();

  DarcyFluxComponent(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// gradient of the pressure
  const VariableGradient & _grad_pp;

  /// fluid weight (gravity*density) as a vector pointing downwards, eg '0 0 -10000'
  RealVectorValue _fluid_weight;

  /// fluid dynamic viscosity
  Real _fluid_viscosity;

  /// (1/velocity_scaling)
  Real _poro_recip;

  /// Material permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// Desired component
  unsigned int _component;
};
