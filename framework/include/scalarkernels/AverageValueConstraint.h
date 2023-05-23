//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"

/**
 * This Kernel implements part of the equation that enforces the constraint
 * of
 *
 * \int \phi = V_0
 *
 * where V_0 is a given constant, using a Lagrange multiplier
 * approach.
 *
 * In particular, this Kernel implements the residual contribution
 * corresponding to equation (6) in the detailed description [1] of
 * this problem.
 *
 * See also: test/src/kernels/ScalarLagrangeMultiplier.C
 * [0]: kernels/scalar_constraints/scalar_constraint_kernel.i
 * [1]: https://github.com/idaholab/large_media/blob/master/scalar_constraint_kernel.pdf
 */
class AverageValueConstraint : public ScalarKernel
{
public:
  static InputParameters validParams();

  AverageValueConstraint(const InputParameters & parameters);

  virtual void reinit() override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobianScalar(unsigned int jvar);

  /// Given (constant) which we want the integral of the solution variable to match
  Real _value;

  /// Name of the Postprocessor value we are trying to equate with 'value'
  const PostprocessorValue & _pp_value;
};
