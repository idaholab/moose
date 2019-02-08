//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORKERNEL_H
#define VECTORKERNEL_H

#include "KernelBase.h"
#include "MooseVariableInterface.h"

class VectorKernel;

template <>
InputParameters validParams<VectorKernel>();

class VectorKernel : public KernelBase, public MooseVariableInterface<RealVectorValue>
{
public:
  VectorKernel(const InputParameters & parameters);

  /// Compute this VectorKernel's contribution to the residual
  virtual void computeResidual() override;

  /// Compute this VectorKernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() override;

  /// Computes d-residual / d-jvar... storing the result in Ke.
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  virtual VectorMooseVariable & variable() override { return _var; }

  /**
   * Compute this Kernel's contribution to the residual at the current quadrature point
   */
  virtual Real computeQpResidual() = 0;

protected:
  /// This is a regular kernel so we cast to a regular MooseVariable
  VectorMooseVariable & _var;

  /// the current test function
  const VectorVariableTestValue & _test;

  /// gradient of the test function
  const VectorVariableTestGradient & _grad_test;

  /// the current shape functions
  const VectorVariablePhiValue & _phi;

  /// gradient of the shape function
  const VectorVariablePhiGradient & _grad_phi;

  /// Holds the solution at current quadrature points
  const VectorVariableValue & _u;

  /// Holds the solution gradient at current quadrature points
  const VectorVariableGradient & _grad_u;
};

#endif /* VECTORKERNEL_H */
