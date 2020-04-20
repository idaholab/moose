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

#include "DualRealOps.h"

// forward declarations
template <typename>
class ADKernelTempl;

using ADKernel = ADKernelTempl<Real>;
using ADVectorKernel = ADKernelTempl<RealVectorValue>;

template <typename T>
class ADKernelTempl : public KernelBase, public MooseVariableInterface<T>
{
public:
  static InputParameters validParams();

  ADKernelTempl(const InputParameters & parameters);

  void jacobianSetup() override;

  // See KernelBase base for documentation of these overridden methods
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(MooseVariableFEBase &) override final;
  virtual void computeADOffDiagJacobian() override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  virtual MooseVariableFE<T> & variable() override { return _var; }

protected:
  /// Compute this Kernel's contribution to the residual at the current quadrature point
  virtual ADReal computeQpResidual() = 0;

  /**
   * Compute this Kernel's contribution to the Jacobian at the current quadrature point
   */
  virtual Real computeQpJacobian() { return 0; }

  /**
   * This is the virtual that derived classes should override for computing an off-diagonal Jacobian
   * component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/) { return 0; }

  /// This is a regular kernel so we cast to a regular MooseVariable
  MooseVariableFE<T> & _var;

  /// the current test function
  const ADTemplateVariableTestValue<T> & _test;

  /// gradient of the test function
  const ADTemplateVariableTestGradient<T> & _grad_test;

  // gradient of the test function without possible displacement derivatives
  const typename OutputTools<T>::VariableTestGradient & _regular_grad_test;

  /// Holds the solution at current quadrature points
  const ADTemplateVariableValue<T> & _u;

  /// Holds the solution gradient at the current quadrature points
  const ADTemplateVariableGradient<T> & _grad_u;

  /// The ad version of JxW
  const MooseArray<ADReal> & _ad_JxW;

  /// The ad version of coord
  const MooseArray<ADReal> & _ad_coord;

  /// The ad version of q_point
  const MooseArray<ADPoint> & _ad_q_point;

  /// The current shape functions
  const ADTemplateVariablePhiValue<T> & _phi;

  ADReal _r;
  std::vector<ADReal> _residuals;

  /// The current gradient of the shape functions
  const ADTemplateVariablePhiGradient<T> & _grad_phi;

  /// The current gradient of the shape functions without possible displacement derivatives
  const typename OutputTools<T>::VariablePhiGradient & _regular_grad_phi;

  /// Whether this object is acting on the displaced mesh
  const bool _use_displaced_mesh;

private:
  const Elem * _my_elem;
};
