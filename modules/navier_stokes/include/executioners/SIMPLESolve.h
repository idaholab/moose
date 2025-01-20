//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "LinearAssemblySegregatedSolve.h"

/**
 * Implementation of Semi-Implicit Method for Pressure-Linked Equations (SIMPLE)
 * with MOOSE-based applications. It is a slightly restricted version of
 * the PIMPLE algorithm: (i) no PISO iterations are allowed, (ii) no time
 * kernels are allowed.
 */
class SIMPLESolve : public LinearAssemblySegregatedSolve
{
public:
  SIMPLESolve(Executioner & ex);

  static InputParameters validParams();

  /// Check if the user defined time kernels, if yes error out
  virtual void checkIntegrity() override;

protected:
  /// Check if the system contains time kernels
  virtual void checkTimeKernels(LinearSystem & system);
};
