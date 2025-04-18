//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundsBase.h"

/**
 * Provides constant bound of a variable
 * for the PETSc's variational inequalities solver
 */
class ConstantBounds : public BoundsBase
{
public:
  static InputParameters validParams();

  ConstantBounds(const InputParameters & parameters);

protected:
  virtual Real getBound() override { return _bound_value; }

  /// The value of bound for the variable
  Real _bound_value;
};
