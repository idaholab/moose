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

/**
 * Base class shared by AD and non-AD scalar kernels
 */
class ScalarKernelBase : public ResidualObject, public ScalarCoupleable
{
public:
  static InputParameters validParams();

  ScalarKernelBase(const InputParameters & parameters);

  /**
   * Reinitialization method called before each call to computeResidual()
   */
  virtual void reinit() = 0;

  /**
   * The variable that this kernel operates on.
   */
  virtual const MooseVariableScalar & variable() const override { return _var; }

  /**
   * Use this to enable/disable the scalar kernel
   * @return true if the scalar kernel is active
   */
  virtual bool isActive() { return true; }

  /**
   * Retrieves the old value of the variable that this ScalarKernelBase operates on.
   *
   * Store this as a _reference_ in the constructor.
   */
  const VariableValue & uOld() const;

protected:
  /// Scalar variable
  MooseVariableScalar & _var;

  unsigned int _i, _j;
};
