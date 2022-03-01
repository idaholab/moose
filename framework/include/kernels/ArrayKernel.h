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
#include "MooseVariableScalar.h"

class ArrayKernel : public KernelBase, public MooseVariableInterface<RealEigenVector>
{
public:
  static InputParameters validParams();

  ArrayKernel(const InputParameters & parameters);

  /// Compute this ArrayKernel's contribution to the residual
  virtual void computeResidual() override;

  /// Compute this ArrayKernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() override;

  /// Computes full Jacobian of jvar and the array variable this kernel operates on
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  virtual const ArrayMooseVariable & variable() const override { return _var; }

protected:
  /**
   * Compute this Kernel's contribution to the residual at the current quadrature point,
   * to be filled in \p residual.
   */
  virtual void computeQpResidual(RealEigenVector & residual) = 0;

  /**
   * Compute this Kernel's contribution to the diagonal Jacobian at the current quadrature point
   */
  virtual RealEigenVector computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for computing a full Jacobian
   * component
   */
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar);

  /**
   * This is the virtual that derived classes should override for computing a full Jacobian
   * component
   */
  virtual RealEigenMatrix computeQpOffDiagJacobianScalar(const MooseVariableScalar & jvar);

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here
   */
  virtual void initQpJacobian() {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initQpOffDiagJacobian(const MooseVariableFEBase & jvar)
  {
    if (jvar.number() == _var.number())
      initQpJacobian();
  }

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
  const ArrayVariableValue & _u;

  /// Holds the solution gradient at current quadrature points
  const ArrayVariableGradient & _grad_u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  /// Work vector for residual and diag jacobian
  RealEigenVector _work_vector;

  /// Work vector for off diag jacobian
  RealEigenMatrix _work_matrix;
};
