//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STAGNATIONTEMPERATUREAUX_H
#define STAGNATIONTEMPERATUREAUX_H

#include "AuxKernel.h"

class StagnationTemperatureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<StagnationTemperatureAux>();

/**
 * Compute stagnation temperature from specific volume, specific internal energy, and velocity.
 */
class StagnationTemperatureAux : public AuxKernel
{
public:
  StagnationTemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _specific_volume;
  const VariableValue & _specific_internal_energy;
  const VariableValue & _velocity;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* STAGNATIONTEMPERATUREAUX_H */
