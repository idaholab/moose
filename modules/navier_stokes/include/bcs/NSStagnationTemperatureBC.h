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

/**
 * This Dirichlet condition imposes the condition T_0 = T_0_desired,
 * where T_0 is the stagnation temperature, defined as:
 * T_0 = T * (1 + (gam-1)/2 * M^2)
 */
class NSStagnationTemperatureBC : public NSStagnationBC
{
public:
  static InputParameters validParams();

  NSStagnationTemperatureBC(const InputParameters & parameters);

protected:
  // NodalBC's can (currently) only specialize the computeQpResidual function,
  // the computeQpJacobian() function automatically assembles a "1" onto the main
  // diagonal for this DoF.
  virtual Real computeQpResidual();

  const VariableValue & _temperature;

  // Required paramters
  const Real _desired_stagnation_temperature;
};
