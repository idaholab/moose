//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelBase.h"
#include "MooseVariableInterface.h"

class Kernel;

template <>
InputParameters validParams<Kernel>();

class Kernel : public KernelBase, public MooseVariableInterface<Real>
{
public:
  Kernel(const InputParameters & parameters);

  /// Compute this Kernel's contribution to the residual
  virtual void computeResidual() override;

  /// Compute this Kernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() override;

  /// Computes d-residual / d-jvar... storing the result in Ke.
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

  /**
   * Deprecated method
   */
  virtual void computeOffDiagJacobian(unsigned jvar);

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  virtual MooseVariable & variable() override { return _var; }

  /**
   * Compute this Kernel's contribution to the residual at the current quadrature point
   */
  virtual Real computeQpResidual() = 0;

protected:
  /// This is a regular kernel so we cast to a regular MooseVariable
  MooseVariable & _var;

  /// the current test function
  const VariableTestValue & _test;

  /// gradient of the test function
  const VariableTestGradient & _grad_test;

  /// the current shape functions
  const VariablePhiValue & _phi;

  /// gradient of the shape function
  const VariablePhiGradient & _grad_phi;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;
};

