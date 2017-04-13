/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
  virtual Real computeValue();

  const VariableValue & _specific_volume;
  const VariableValue & _specific_internal_energy;
  const VariableValue & _velocity;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* STAGNATIONTEMPERATUREAUX_H */
