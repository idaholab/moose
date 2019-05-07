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

class ArrayKernel;

template <>
InputParameters validParams<ArrayKernel>();

class ArrayKernel : public KernelBase, public MooseVariableInterface<RealArrayValue>
{
public:
  ArrayKernel(const InputParameters & parameters);

  /// Compute this ArrayKernel's contribution to the residual
  virtual void computeResidual() override;

  /// Compute this ArrayKernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() override;

  /// Computes full Jacobian of jvar and the array variable this kernel operates on
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  virtual ArrayMooseVariable & variable() override { return _var; }

protected:
  /**
   * Compute this Kernel's contribution to the residual at the current quadrature point
   */
  virtual RealArrayValue computeQpResidual() = 0;

  /**
   * Compute this Kernel's contribution to the diagonal Jacobian at the current quadrature point
   */
  virtual RealArrayValue computeQpJacobian() { return RealArrayValue(_var.count()); }

  /**
   * This is the virtual that derived classes should override for computing a full Jacobian
   * component
   */
  virtual RealArray computeQpOffDiagJacobian(MooseVariableFEBase & jvar);

  /**
   * This is the virtual that derived classes should override for computing a full Jacobian
   * component
   */
  virtual RealArray computeQpOffDiagJacobianScalar(MooseVariableScalar & jvar);

  /// for array kernel
  void saveLocalArrayResidual(DenseVector<Number> & re,
                              unsigned int i,
                              unsigned int ntest,
                              const RealArrayValue & v)
  {
    for (unsigned int j = 0; j < v.size(); ++j, i += ntest)
      re(i) += v(j);
  }
  void saveDiagLocalArrayJacobian(DenseMatrix<Number> & ke,
                                  unsigned int i,
                                  unsigned int ntest,
                                  unsigned int j,
                                  unsigned int nphi,
                                  const RealArrayValue & v)
  {
    for (unsigned int k = 0; k < v.size(); ++k, i += ntest, j += nphi)
      ke(i, j) += v(k);
  }
  void saveFullLocalArrayJacobian(DenseMatrix<Number> & ke,
                                  unsigned int i,
                                  unsigned int ntest,
                                  unsigned int j,
                                  unsigned int nphi,
                                  const RealArray & v)
  {
    unsigned int saved_j = j;
    for (unsigned int k = 0; k < v.rows(); ++k, i += ntest)
    {
      j = saved_j;
      for (unsigned int l = 0; l < v.cols(); ++l, j += nphi)
        ke(i, j) += v(k, l);
    }
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
};
