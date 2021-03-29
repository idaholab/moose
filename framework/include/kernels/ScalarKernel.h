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

  /**
   * Retrieves the old value of the variable that this ScalarKernel operates on.
   *
   * Store this as a _reference_ in the constructor.
   */
  const VariableValue & uOld() const;

protected:
  /// Scalar variable
  MooseVariableScalar & _var;

  unsigned int _i, _j;

  /// The current solution (old solution if explicit)
  const VariableValue & _u;
};
