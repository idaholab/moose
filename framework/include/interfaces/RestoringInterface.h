//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"

// Forward declarations
class FEProblemBase;
class InputParameters;

/**
 * Interface for notifying objects that the problem is restoring to the previous state (i.e.
 * previous time step).
 */
class RestoringInterface
{
public:
  static InputParameters validParams();

  RestoringInterface(const InputParameters & params);
  virtual ~RestoringInterface() = default;

  /**
   * Called on this object when the problem is restoring to the previous state (i.e. previous time
   * step)
   */
  virtual void restoringProblem() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _rsti_feproblem;
};
