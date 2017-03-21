/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSTEMPERATUREAUX_H
#define NSTEMPERATUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSTemperatureAux;
class IdealGasFluidProperties;

template <>
InputParameters validParams<NSTemperatureAux>();

/**
 * Temperature is an auxiliary value computed from the total energy
 * based on the FluidProperties.
 */
class NSTemperatureAux : public AuxKernel
{
public:
  NSTemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _specific_volume;
  const VariableValue & _internal_energy;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};

#endif // NSTEMPERATUREAUX_H
