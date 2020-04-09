//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSStagnationBC.h"

// Forward Declarations

// Specialization required of all user-level Moose objects

/**
 * This Dirichlet condition imposes the condition p_0 = p_0_desired,
 * where p_0 is the stagnation pressure, defined as:
 * p_0 = p * (1 + (gam-1)/2 * M^2)^(gam/(gam-1))
 */
class NSStagnationPressureBC : public NSStagnationBC
{
public:
  static InputParameters validParams();

  NSStagnationPressureBC(const InputParameters & parameters);

protected:
  // NodalBC's can (currently) only specialize the computeQpResidual function,
  // the computeQpJacobian() function automatically assembles a "1" onto the main
  // diagonal for this DoF.
  virtual Real computeQpResidual();

  // Coupled variables
  const VariableValue & _pressure;

  // Required paramters
  const Real _desired_stagnation_pressure;
};
