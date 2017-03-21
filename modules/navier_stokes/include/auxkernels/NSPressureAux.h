/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSPRESSUREAUX_H
#define NSPRESSUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSPressureAux;
class IdealGasFluidProperties;

template <>
InputParameters validParams<NSPressureAux>();

/**
 * Nodal auxiliary variable, for computing pressure at the nodes
 */
class NSPressureAux : public AuxKernel
{
public:
  NSPressureAux(const InputParameters & parameters);

  virtual ~NSPressureAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _specific_volume;
  const VariableValue & _internal_energy;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};

#endif // VELOCITYAUX_H
