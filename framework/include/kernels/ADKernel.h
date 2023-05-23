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
#include "ADFunctorInterface.h"
#include "DualRealOps.h"

// forward declarations
template <typename>
class ADKernelTempl;

using ADKernel = ADKernelTempl<Real>;
using ADVectorKernel = ADKernelTempl<RealVectorValue>;

template <typename T>
class ADKernelTempl : public KernelBase, public MooseVariableInterface<T>, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ADKernelTempl(const InputParameters & parameters);

  void jacobianSetup() override;

  const MooseVariableFE<T> & variable() const override { return _var; }

protected:
  void computeJacobian() override;
  void computeResidualAndJacobian() override;
  void computeOffDiagJacobian(unsigned int) override;
  void computeOffDiagJacobianScalar(unsigned int jvar) override;

  /**
   * Just as we allow someone deriving from this to modify the
   * \p _residuals data member in their \p computeResidualsForJacobian
   * overrides, we must also potentially allow them to modify the dof
   * indices. For example a user could have something like an \p LMKernel which sums computed
   * strong residuals into both primal and LM residuals. That user needs to be
   * able to feed dof indices from both the primal and LM variable into
   * \p Assembly::addJacobian
   */
  virtual const std::vector<dof_id_type> & dofIndices() const { return _var.dofIndices(); }

protected:
  // See KernelBase base for documentation of these overridden methods
  void computeResidual() override;

  /**
   * compute the \p _residuals member for filling the Jacobian. We want to calculate these residuals
   * up-front when doing loal derivative indexing because we can use those residuals to fill \p
   * _local_ke for every associated jvariable. We do not want to re-do these calculations for every
   * jvariable and corresponding \p _local_ke. For global indexing we will simply pass the computed
   * \p _residuals directly to \p Assembly::addJacobian
   */
  virtual void computeResidualsForJacobian();

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

private:
  /**
   * compute all the Jacobian entries
   */
  void computeADJacobian();

  const Elem * _my_elem;
};
