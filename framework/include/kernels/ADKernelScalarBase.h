//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * This ADKernel adds standardized methods for assembling to a primary
 * scalar variable associated with the primary variable of the ADKernel
 * object. Essentially, the entire row of the residual and Jacobian
 * associated with this scalar variable will also be assembled here
 * using the loops over volumetric elements.
 * This variable is "scalar_variable" in the input file and "kappa"
 * within the source code.
 */
class ADKernelScalarBase : public ADKernel
{
public:
  static InputParameters validParams();

  ADKernelScalarBase(const InputParameters & parameters);

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
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar, the number of the (other) scalar variable
   */
  void computeOffDiagJacobianScalar(unsigned int /*jvar_num*/) override;

  /**
   * Computes residual and jacobian block for field and scalar variables
   */
  void computeResidualAndJacobian() override;

protected:
  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual ADReal computeScalarQpResidual();

  /**
   * compute the \p _scalar_residuals member for filling the Jacobian. We want to calculate these
   * residuals up-front when doing loal derivative indexing because we can use those residuals to
   * fill \p _local_ke for every associated jvariable. We do not want to re-do these calculations
   * for every jvariable and corresponding \p _local_ke. For global indexing we will simply pass
   * the computed \p _residuals directly to \p Assembly::processJacobian
   */
  virtual void computeScalarResidualsForJacobian();

  /**
   * For coupling scalar variables
   * Added solely for GenericKernelScalar override; should not be used
   */
  virtual Real computeQpOffDiagJacobianScalar(unsigned int /*jvar*/) { return 0; }

  /**
   * Method for computing the scalar variable part of Jacobian at quadrature points
   * Added solely for GenericKernelScalar override; should not be used
   */
  virtual Real computeScalarQpJacobian() { return 0; }

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   * Added solely for GenericKernelScalar override; should not be used
   */
  virtual Real computeScalarQpOffDiagJacobian(const unsigned int /*jvar_num*/) { return 0; }

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   * Added solely for GenericKernelScalar override; should not be used
   */
  virtual Real computeScalarQpOffDiagJacobianScalar(const unsigned int /*svar_num*/) { return 0; }

  /**
   * Put necessary evaluations depending on qp but independent of test functions here
   */
  virtual void initScalarQpResidual() {}

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
  const ADVariableValue & _kappa;

  /// Used internally to iterate over each scalar component
  unsigned int _h;
  unsigned int _l;
  std::vector<ADReal> _scalar_residuals;
};

inline ADReal
ADKernelScalarBase::computeScalarQpResidual()
{
  mooseError(
      "A scalar_variable has been set and compute_scalar_residuals=true, ",
      "but the computeScalarQpResidual method was not overridden. Accidental call of base class?");
  return 0;
}
