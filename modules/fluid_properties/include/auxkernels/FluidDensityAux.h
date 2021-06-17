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

class SinglePhaseFluidProperties;

/**
 * Computes density from pressure and temperature.
 */
class FluidDensityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  FluidDensityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// pressure
  const VariableValue & _pressure;

  /// fluid temperature
  const VariableValue & _temperature;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};
