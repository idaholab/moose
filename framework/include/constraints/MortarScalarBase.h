//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraint.h"

/**
 * This Constraint adds standardized methods for assembling to a primary
 * scalar variable associated with the major variables of the Mortar
 * Constraint object. Essentially, the entire row of the residual and Jacobian
 * associated with this scalar variable will also be assembled here
 * using the loops over mortar segments.
 * This variable is "scalar_variable" in the input file and "kappa"
 * within the source code.
 */

class MortarScalarBase : public MortarConstraint
{
public:
  static InputParameters validParams();

  MortarScalarBase(const InputParameters & parameters);

  // Using declarations necessary to pull in computeResidual with different parameter list and avoid
  // hidden method warning
  using MortarConstraintBase::computeResidual;

  // Using declarations necessary to pull in computeJacobian with different parameter list and avoid
  // hidden method warning
  using MortarConstraintBase::computeJacobian;

  /**
   * The scalar variable that this kernel operates on.
   */
  const MooseVariableScalar & scalarVariable() const { return *_kappa_var_ptr; }

  /**
   * Computes _var-residuals as well as _kappa-residual
   */
  virtual void computeResidual() override;
  /**
   * Computes d-_var-residual / d-_var and d-_var-residual / d-jvar,
   * as well as d-_kappa-residual / d-_var and d-_kappa-residual / d-jvar
   */
  virtual void computeJacobian() override;

protected:

  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual Real computeScalarQpResidual()
  {
    return 0;
  }

  /**
   * Method for computing the scalar variable part of Jacobian
   */
  virtual void computeScalarJacobian();

  /**
   * Method for computing the scalar variable part of Jacobian at
   * quadrature points
   */
  virtual Real computeScalarQpJacobian()
  {
    return 0;
  }

  /**
   * Method for computing an off-diagonal jacobian component d-_kappa-residual / d-jvar
   */
  void computeScalarOffDiagJacobian(const Moose::MortarType mortar_type, const unsigned int jvar_num);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(const Moose::MortarType /*mortar_type*/, 
                                              const unsigned int /*jvar_num*/)
  {
    return 0;
  }

  void computeOffDiagJacobianScalar(unsigned int) override final;
    /**
   * Method for computing an off-diagonal jacobian component d-_var-residual / d-scalar
   */
  void computeOffDiagJacobianScalar(const Moose::MortarType mortar_type, const unsigned int svar_num);

  /**
   * For coupling scalar variables
   */
  virtual Real computeQpOffDiagJacobianScalar(const Moose::MortarType /*mortar_type*/,
                                              unsigned int /*svar_num*/)
  {
    return 0;
  }

  /**
   * Method for computing an off-diagonal jacobian component d-_kappa-residual / d-scalar
   */
  void computeScalarOffDiagJacobianScalar(const unsigned int jvar_num);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobianScalar(const unsigned int /*jvar_num*/)
  {
    return 0;
  }

  /**
   * Put necessary evaluations depending on qp but independent of test functions here
   */
  virtual void initScalarQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here
   */
  virtual void initScalarQpJacobian(const unsigned int /*jvar_num*/) {}

  /**
   * Put necessary evaluations depending on qp but independent of test and shape functions here for
   * off-diagonal Jacobian assembly.
   * If jvar_num is a scalar, then this for computeQpOffDiagJacobianScalar
   * If jvar_num is a variable, then this for computeScalarQpOffDiagJacobian
   */
  virtual void initScalarQpOffDiagJacobian(const Moose::MortarType /*mortar_type*/,
                                           const unsigned int /*jvar_num*/)
  {
  }

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

  /// Used internally to iterate over each scalar component
  unsigned int _h;
  unsigned int _l;
};

inline void
MortarScalarBase::computeOffDiagJacobianScalar(unsigned int)
{
  mooseError("Must call the mortar type overload instead");
}
