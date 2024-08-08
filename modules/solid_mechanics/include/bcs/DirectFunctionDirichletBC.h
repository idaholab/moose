//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirectDirichletBCBase.h"

class Function;

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 * For use with direct central difference time integrator
 */
class DirectFunctionDirichletBC : public DirectDirichletBCBase
{
public:
  static InputParameters validParams();

  DirectFunctionDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;

  /// The function being used for evaluation
  const Function & _func;
};
