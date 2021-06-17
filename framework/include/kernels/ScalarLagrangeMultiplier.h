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
class ScalarLagrangeMultiplier : public Kernel
{
public:
  static InputParameters validParams();

  ScalarLagrangeMultiplier(const InputParameters & parameters);
  virtual ~ScalarLagrangeMultiplier();

  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobianScalar(unsigned int jvar) override;

  /// Lagrange multiplier variable ID
  unsigned int _lambda_var;

  /// Lagrange multiplier variable value
  const VariableValue & _lambda;
};
