//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

// Forward Declarations
class IdealGasFluidProperties;

// Specialization required of all user-level Moose objects

/**
 * This is the base class for the "imposed stagnation" value boundary
 * conditions.  Derived classes impose specified stagnation pressure
 * and temperature BCs as Dirichlet terms in the governing equations.
 */
class NSStagnationBC : public NodalBC
{
public:
  static InputParameters validParams();

  NSStagnationBC(const InputParameters & parameters);

protected:
  const VariableValue & _mach;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};
