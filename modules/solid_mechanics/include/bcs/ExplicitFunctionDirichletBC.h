//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitDirichletBCBase.h"

class Function;

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 * For use with the ExplicitMixedOrder time integrator
 */
class ExplicitFunctionDirichletBC : public ExplicitDirichletBCBase
{
public:
  static InputParameters validParams();

  ExplicitFunctionDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;

  /// The function being used for evaluation
  const Function & _func;
};
