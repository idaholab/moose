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

  virtual const MooseVariable & variable() const override { return _var; }

  /**
   * The scalar variable that this kernel operates on.
   */
  const MooseVariableScalar & scalarVariable() const { return *_kappa_var_ptr; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  /**
   * Computes d-_var-residual / d-jvar as well as d-_kappa-residual / d-jvar
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar, the number of the (other) scalar variable
   */
  void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual Real computeScalarQpResidual() { return 0; }

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
  void computeScalarOffDiagJacobian(const unsigned int jvar);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(const unsigned int /*jvar*/) { return 0; }

  void computeOffDiagJacobianScalarLocal(const unsigned int jvar);
  /**
   * Method for computing an off-diagonal jacobian component d-_kappa-residual / d-scalar
   */
  void computeScalarOffDiagJacobianScalar(const unsigned int jvar);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobianScalar(const unsigned int /*jvar*/) { return 0; }

  /**
   * Put necessary evaluations depending on qp but independent of test functions here
   */
  virtual void initScalarQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here
   */
  virtual void initScalarQpJacobian(const unsigned int /*jvar*/) {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initScalarQpOffDiagJacobian(const MooseVariableFEBase &) {}

  /// Whether to compute scalar contributions
  const bool _use_scalar;

  /// A dummy object useful for constructing _kappa when not using scalars
  const VariableValue _kappa_dummy;

  /// (Pointer to) Scalar variable this kernel operates on
  const MooseVariableScalar * const _kappa_var_ptr;

  /// The unknown scalar variable ID
  const unsigned int _kappa_var;

  /// Order of the scalar variable, used in several places
  const unsigned int _k_order;

  /// Reference to the current solution at the current quadrature point
  const VariableValue & _kappa;
};
