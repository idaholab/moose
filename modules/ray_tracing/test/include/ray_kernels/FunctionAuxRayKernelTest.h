//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxRayKernel.h"

// Forward declarations
class Function;

/**
 * Test AuxRayKernel that appends a function value on each Ray segment.
 *
 * The segment midpoint is used as the function position.
 */
class FunctionAuxRayKernelTest : public AuxRayKernel
{
public:
  FunctionAuxRayKernelTest(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override;

protected:
  /// Function being used to compute the value to append
  const Function & _func;
};
