//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBCBase.h"

// Forward Declarations
class Function;

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 *
 * Deprecated: use FunctionDirichletBC with preset = true instead.
 */
class ADFunctionPresetBC : public ADDirichletBCBase
{
public:
  static InputParameters validParams();

  ADFunctionPresetBC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual ADReal computeQpValue() override;

  /// Function being used for evaluation of this BC
  const Function & _func;
};
