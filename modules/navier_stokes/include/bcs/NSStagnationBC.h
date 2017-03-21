/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSSTAGNATIONBC_H
#define NSSTAGNATIONBC_H

#include "NodalBC.h"

// Forward Declarations
class NSStagnationBC;
class IdealGasFluidProperties;

// Specialization required of all user-level Moose objects
template <>
InputParameters validParams<NSStagnationBC>();

/**
 * This is the base class for the "imposed stagnation" value boundary
 * conditions.  Derived classes impose specified stagnation pressure
 * and temperature BCs as Dirichlet terms in the governing equations.
 */
class NSStagnationBC : public NodalBC
{
public:
  NSStagnationBC(const InputParameters & parameters);

protected:
  const VariableValue & _mach;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};

#endif // NSSTAGNATIONBC_H
