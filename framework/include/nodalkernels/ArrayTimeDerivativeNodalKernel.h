//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayNodalKernel.h"

/**
 * Represents du/dt
 */
class ArrayTimeDerivativeNodalKernel : public ArrayNodalKernel
{
public:
  static InputParameters validParams();

  ArrayTimeDerivativeNodalKernel(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual void computeQpJacobian() override;

  /// Time derivative of u
  const ArrayVariableValue & _u_dot;

  /// Derivative of u_dot with respect to u. This value is only dependent on the
  /// time integration scheme, which is the same for every componenet. Therefore,
  /// _du_dot_du is VariableValue instead of an ArrayVariableValue
  const VariableValue & _du_dot_du;
};
