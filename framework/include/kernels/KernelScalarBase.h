//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * This Kernel adds standardized methods for assembling to a primary
 * scalar variable associated with the primary variable of the Kernel
 * object. Essentially, the entire row of the residual and Jacobian
 * associated with this scalar variable will also be assembled here
 * using the loops over volumetric elements.
 * This variable is "scalar_variable" in the input file and "kappa"
 * within the source code.
 */
class KernelScalarBase : public Kernel
{
public:
  static InputParameters validParams();

  KernelScalarBase(const InputParameters & parameters);

  /**
   * The scalar variable that this kernel operates on.
   */
  const MooseVariableScalar & scalarVariable() const
  {
    mooseAssert(_kappa_var_ptr, "kappa pointer should have been set in the constructor");
    return *_kappa_var_ptr;
  }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  /**
   * Computes d-_var-residual / d-jvar as well as d-_kappa-residual / d-jvar
   */
  virtual void computeOffDiagJacobian(unsigned int jvar_num) override;
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param svar_num, the number of the (other) scalar variable
   */
  void computeOffDiagJacobianScalar(unsigned int svar_num) override;

protected:
  /**
   * Precalculate method that is executed prior to scalar coupled variable loop
   * within computeOffDiagJacobianScalar; analogous to precalculateResidual(),
   * precalculateJacobian(), and precalculateOffDiagJacobian().
   */
  virtual void precalculateOffDiagJacobianScalar(unsigned int /* svar_num */) {}

  /**
   * Method for computing the scalar part of residual
   */
  virtual void computeScalarResidual();

  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual Real computeScalarQpResidual();

  /**
   * Method for computing the scalar variable part of Jacobian
   */
  virtual void computeScalarJacobian();

  /**
   * Method for computing the scalar variable part of Jacobian at
   * quadrature points
   */
  virtual Real computeScalarQpJacobian() { return 0; }

  /**
   * Method for computing an off-diagonal jacobian component d-_kappa-residual / d-jvar
   */
  virtual void computeScalarOffDiagJacobian(const unsigned int jvar_num);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(const unsigned int /*jvar_num*/) { return 0; }

  /**
   * Method for computing an off-diagonal jacobian component d-_var-residual / d-scalar
   * Revised version of Kernel::computeOffDiagJacobianScalar
   */
  virtual void computeOffDiagJacobianScalarLocal(const unsigned int svar_num);

  /**
   * Method for computing an off-diagonal jacobian component d-_kappa-residual / d-scalar
   */
  virtual void computeScalarOffDiagJacobianScalar(const unsigned int svar_num);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobianScalar(const unsigned int /*svar_num*/) { return 0; }

  /**
   * Put necessary evaluations depending on qp but independent of test functions here
   */
  virtual void initScalarQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here
   */
  virtual void initScalarQpJacobian(const unsigned int /*svar_num*/) {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initScalarQpOffDiagJacobian(const MooseVariableFEBase &) {}

  /// Whether a scalar variable is declared for this kernel
  const bool _use_scalar;

  /// Whether to compute scalar contributions for this instance
  const bool _compute_scalar_residuals;

  /// Whether to compute field contributions for this instance
  const bool _compute_field_residuals;

  /// (Pointer to) Scalar variable this kernel operates on
  const MooseVariableScalar * const _kappa_var_ptr;

  /// The unknown scalar variable ID
  const unsigned int _kappa_var;

  /// Order of the scalar variable, used in several places
  const unsigned int _k_order;

  /// Reference to the current solution at the current quadrature point
  const VariableValue & _kappa;

  /// Used internally to iterate over each scalar component
  unsigned int _h;
  unsigned int _l;
};

inline Real
KernelScalarBase::computeScalarQpResidual()
{
  mooseError(
      "A scalar_variable has been set and compute_scalar_residuals=true, ",
      "but the computeScalarQpResidual method was not overridden. Accidental call of base class?");
  return 0;
}
