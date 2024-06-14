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
 * Interface for notifying objects that the solve has failed
 */
class SolveFailedInterface
{
public:
  static InputParameters validParams();

  SolveFailedInterface(const InputParameters & params);
  virtual ~SolveFailedInterface() = default;

  /**
   * Called on this object immediately after the solve failed
   */
  virtual void onSolveFailed() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _sfi_feproblem;
};
