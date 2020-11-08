//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ResidualObject.h"
#include "ScalarCoupleable.h"
#include "MooseVariableScalar.h"

// Forward declarations
class ScalarKernel;

template <>
InputParameters validParams<ScalarKernel>();

class ScalarKernel : public ResidualObject, public ScalarCoupleable
{
public:
  static InputParameters validParams();

  ScalarKernel(const InputParameters & parameters);

  virtual void reinit() = 0;

  /**
   * The variable that this kernel operates on.
   */
  virtual const MooseVariableScalar & variable() const override { return _var; }

  /**
   * Use this to enable/disable the constraint
   * @return true if the constrain is active
   */
  virtual bool isActive() { return true; }

protected:
  /// Scalar variable
  MooseVariableScalar & _var;

  unsigned int _i, _j;

  /// Value(s) of the scalar variable
  VariableValue & _u;

  /// Old value(s) of the scalar variable
  VariableValue & _u_old;
};
