//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelScalarBase.h"

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
class ADScalarLMKernel : public ADKernelScalarBase
{
public:
  static InputParameters validParams();

  ADScalarLMKernel(const InputParameters & parameters);
  virtual ~ADScalarLMKernel();

protected:
  /**
   * Method for computing the residual at quadrature points
   */
  virtual ADReal computeQpResidual() override;

  /**
   * Method for computing the scalar part of residual at quadrature points
   */
  virtual ADReal computeScalarQpResidual() override;

  /// Given (constant) which we want the integral of the solution variable to match
  Real _value;

  /// Name of the Postprocessor containing the volume of the domain
  const PostprocessorValue & _pp_value;
};
