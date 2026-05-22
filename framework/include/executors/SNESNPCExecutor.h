//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SNESExecutor.h"

class SNESNPCExecutor : public SNESExecutor
{
public:
  static InputParameters validParams();
  SNESNPCExecutor(const InputParameters & params);
  virtual ~SNESNPCExecutor();

  /**
   * Interface for applying our nonlinear preconditioning at the matrix level
   */
  virtual PetscErrorCode applyBA(Mat A, Vec X, Vec Y) = 0;

protected:
  /// Cached work vector to avoid repeat allocations. This work vector should always be used as the intermediate storage between application of A, and application of this object's preconditioning (B)
  Vec _work = nullptr;
};
