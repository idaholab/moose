//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local Includes
#include "RayKernelBase.h"

// MOOSE includes
#include "MooseVariableFE.h"
#include "MooseVariableInterface.h"

class AuxiliarySystem;

class AuxRayKernel : public RayKernelBase, public MooseVariableInterface<Real>
{
public:
  AuxRayKernel(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override = 0;

  /**
   * Use to accumulate a value into the corresponding AuxVariable from this AuxRayKernel
   */
  void addValue(const Real value);

  /**
   * Gets the variable this AuxRayKernel contributes to.
   */
  MooseVariableFE<Real> & variable() { return _var; }

protected:
  /// The aux system
  AuxiliarySystem & _aux;

  /// The AuxVariable this AuxRayKernel contributes to
  MooseVariableFE<Real> & _var;

private:
  /// Spin mutex object for adding values
  static Threads::spin_mutex _add_value_mutex;
};
