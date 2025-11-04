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
#include "ADFunctorInterface.h"

/**
 * Base class for array variable (equation) kernels using automatic differentiation
 */
class ADArrayKernel : public KernelBase,
                      public MooseVariableInterface<RealEigenVector>,
                      public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ADArrayKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void jacobianSetup() override;

  virtual const ArrayMooseVariable & variable() const override { return _var; }

protected:
  /**
   * Compute this Kernel's contribution to the residual at the current quadrature point,
   * to be filled in \p residual.
   */
  virtual void computeQpResidual(ADRealEigenVector & residual) = 0;

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual() {}

  /// This is an array kernel so we cast to a ArrayMooseVariable
  ArrayMooseVariable & _var;

  /// the current test function
  const ArrayVariableTestValue & _test;

  /// gradient of the test function
  const ArrayVariableTestGradient & _grad_test;
  const MappedArrayVariablePhiGradient & _array_grad_test;

  /// the current shape functions
  const ArrayVariablePhiValue & _phi;

  /// gradient of the shape function
  const ArrayVariablePhiGradient & _grad_phi;

  /// Holds the solution at current quadrature points
  const ADArrayVariableValue & _u;

  /// Holds the solution gradient at current quadrature points
  const ADArrayVariableGradient & _grad_u;

  /// Number of components of the array variable
  const unsigned int _count;

  // Pre-allocated vector for storing the AD array residual of the kernel
  std::vector<ADReal> _local_ad_re;

private:
  /// Work vector for residual and diag jacobian
  ADRealEigenVector _work_vector;

  /// Cache variable to prevent multiple invocations of Jacobian computation for one element (recall that AD computes the Jacobian at all once for all variable couplings)
  const Elem * _my_elem = nullptr;
};
