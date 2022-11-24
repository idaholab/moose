//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorIntegratedBC.h"

/**
 *  Boundary condition of a Neumann style whose value is computed by a
 *  user-defined function for vector variables and whose Jacobian is computed
 *  by the automatic differentiation system.
 */
class ADVectorFunctionNeumannBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  ADVectorFunctionNeumannBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

private:
  /// x component function
  const Function & _function_x;
  /// y component function
  const Function & _function_y;
  /// z component function
  const Function & _function_z;
};
