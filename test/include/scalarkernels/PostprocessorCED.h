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

class PostprocessorCED;

template <>
InputParameters validParams<PostprocessorCED>();

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
 * In particular, this Kernel implements the residual contribution
 * corresponding to equation (6) in the detailed description [1] of
 * this problem.
 *
 * See also: test/src/kernels/ScalarLagrangeMultiplier.C
 * [0]: kernels/scalar_constraints/scalar_constraint_kernel.i
 * [1]: https://github.com/idaholab/large_media/blob/master/scalar_constraint_kernel.pdf
 */
class PostprocessorCED : public ScalarKernel
{
public:
  PostprocessorCED(const InputParameters & parameters);
  virtual ~PostprocessorCED();

  virtual void reinit();
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _i;

  Real _value;
  const PostprocessorValue & _pp_value;
};

