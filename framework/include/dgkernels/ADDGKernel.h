//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernelBase.h"

#include "DualRealOps.h"

class ADDGKernel : public DGKernelBase, public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  ADDGKernel(const InputParameters & parameters);

private:
  void computeElemNeighResidual(Moose::DGResidualType type) override final;
  void computeElemNeighJacobian(Moose::DGJacobianType type) override final;
  void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                       const MooseVariableFEBase & jvar) override final;
  void computeJacobian() override final;
  void computeOffDiagJacobian(unsigned int jvar) override final;

protected:
  const MooseVariableFEBase & variable() const override { return _var; }

  /// Compute this Kernel's contribution to the residual at the current quadrature point
  virtual ADReal computeQpResidual(Moose::DGResidualType type) = 0;

  /// Variable this kernel operates on
  MooseVariable & _var;
  /// Shape functions
  const VariablePhiValue & _phi;
  /// Gradient of shape function
  const VariablePhiGradient & _grad_phi;
  /// test functions
  const VariableTestValue & _test;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test;
  /// Side shape function
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_neighbor;
  /// Side test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_neighbor;

  /// Holds the solution at current quadrature points
  const ADVariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const ADVariableGradient & _grad_u;

  /// Holds the current solution at the current quadrature point
  const ADVariableValue & _u_neighbor;

  /// Holds the current solution gradient at the current quadrature point
  const ADVariableGradient & _grad_u_neighbor;
};
