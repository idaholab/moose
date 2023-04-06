//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernelScalar.h"

/**
 * This Kernel implements part of the equation that enforces the constraint
 * of
 *
 * \int \phi = V_0
 *
 * where V_0 is a given constant, using a Lagrange multiplier
 * approach.
 *
 * In particular, this Kernel implements the residual contribution for
 * the lambda term in Eq. (5), and the Jacobian contributions
 * corresponding to (9) and (10) in our detailed description [1] of
 * the problem.
 *
 * See also: test/src/scalarkernels/AverageValueConstraint.C
 * [0]: kernels/scalar_constraints/scalar_constraint_kernel.i
 * [1]: https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf
 */
template <bool is_ad>
class ScalarLMKernelTempl : public GenericKernelScalar<is_ad>
{
public:
  static InputParameters validParams();

  ScalarLMKernelTempl(const InputParameters & parameters);

protected:
  /**
   * Method for computing the residual at quadrature points
   */
  virtual GenericReal<is_ad> computeQpResidual() override;

  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual GenericReal<is_ad> computeScalarQpResidual() override;

  /**
   * Method for computing the scalar variable part of Jacobian at
   * quadrature points
   */
  virtual Real computeScalarQpJacobian() override;

  /**
   * Method for computing d-_var-residual / d-_kappa at quadrature points.
   */
  virtual Real computeQpOffDiagJacobianScalar(const unsigned int svar) override;

  /**
   * Method for computing d-_kappa-residual / d-_var at quadrature points.
   */
  virtual Real computeScalarQpOffDiagJacobian(const unsigned int jvar) override;

  /// Given (constant) which we want the integral of the solution variable to match
  Real _value;

  /// Name of the Postprocessor containing the volume of the domain
  const PostprocessorValue & _pp_value;

  usingGenericKernelScalarMembers;
};

typedef ScalarLMKernelTempl<false> ScalarLMKernel;
typedef ScalarLMKernelTempl<true> ADScalarLMKernel;
