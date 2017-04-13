/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSSTAGNATIONTEMPERATUREBC_H
#define NSSTAGNATIONTEMPERATUREBC_H

#include "NSStagnationBC.h"

// Forward Declarations
class NSStagnationTemperatureBC;

template <>
InputParameters validParams<NSStagnationTemperatureBC>();

/**
 * This Dirichlet condition imposes the condition T_0 = T_0_desired,
 * where T_0 is the stagnation temperature, defined as:
 * T_0 = T * (1 + (gam-1)/2 * M^2)
 */
class NSStagnationTemperatureBC : public NSStagnationBC
{
public:
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

#endif // NSSTAGNATIONTEMPERATUREBC_H
