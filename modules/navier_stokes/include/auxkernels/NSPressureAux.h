//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
