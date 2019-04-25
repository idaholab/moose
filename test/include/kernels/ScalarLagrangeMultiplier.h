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

class ScalarLagrangeMultiplier;

template <>
InputParameters validParams<ScalarLagrangeMultiplier>();

/**
 * This Kernel is part of a test [0] that solves the constrained
 * Neumann problem:
 *
 * -\nabla^2 \phi = f
 * -\partial u / \partial n = g
 * \int \phi = V_0
 *
 * where V_0 is a given constant, using a Lagrange multiplier
 * approach. Without the integral constraint, this problem is not
 * well-posed since it has only flux boundary conditions.
 *
 * In particular, this Kernel implements the residual contribution for
 * the lambda term in Eq. (5), and the Jacobian contributions
 * corresponding to (9) and (10) in our detailed description [1] of
 * the problem.
 *
 * See also: test/src/scalarkernels/PostprocessorCED.C
 * [0]: kernels/scalar_constraints/scalar_constraint_kernel.i
 * [1]: https://github.com/idaholab/large_media/blob/master/scalar_constraint_kernel.pdf
 */
class ScalarLagrangeMultiplier : public Kernel
{
public:
  ScalarLagrangeMultiplier(const InputParameters & parameters);
  virtual ~ScalarLagrangeMultiplier();

  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _lambda_var;
  VariableValue & _lambda;
};

