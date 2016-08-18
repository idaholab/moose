/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ARRAYKERNEL_H
#define ARRAYKERNEL_H

#include "KernelBase.h"
#include "ArrayMooseVariableInterface.h"

class ArrayKernel;

template<>
InputParameters validParams<ArrayKernel>();

class ArrayKernel :
  public KernelBase,
  public ArrayMooseVariableInterface
{
public:
  ArrayKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  /// Compute this ArrayKernel's contribution to the residual at the current quadrature point
  virtual void computeQpResidual() = 0;

  /// Compute this ArrayKernel's contribution to the Jacobian at the current quadrature point
  virtual void computeQpJacobian();

  /// This is the virtual that derived classes should override for computing an off-diagonal Jacobian component.
  virtual void computeQpOffDiagJacobian(unsigned int jvar);

  /// This callback is used for ArrayKernels that need to perform a per-element calculation
  virtual void precalculateResidual();

  /// The variable this ArrayKernel is operating on
  ArrayMooseVariable & _array_var;

  /// Holds the solution at current quadrature points
  const ArrayVariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const ArrayVariableGradient & _grad_u;

  /// Time derivative of u
  const ArrayVariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const ArrayVariableValue & _du_dot_du;

  /// the current test function
  const VariableTestValue & _test;

  /// gradient of the current test function
  const ArrayVariableTestGradient & _grad_test;

  /// the current shape functions
  const VariablePhiValue & _phi;

  /// gradient of the shape function
  const VariablePhiGradient & _grad_phi;

  /// The residual for each shape function.  This should be set within computeQpResidual()
  Eigen::VectorXd _residual;
};

#endif /* ARRAYKERNEL_H */
