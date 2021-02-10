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
class IdealGasFluidProperties;

/**
 * Temperature is an auxiliary value computed from the total energy
 * based on the FluidProperties.
 */
class NSTemperatureAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NSTemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _specific_volume;
  const VariableValue & _specific_internal_energy;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};
