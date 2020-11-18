//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

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
 * the lambda term in Eq. (5), and both terms in Eq. (6)
 *
 * [0]: https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf
 */
class FVScalarLagrangeMultiplier : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVScalarLagrangeMultiplier(const InputParameters & parameters);

private:
  void computeResidual() override final;
  void computeJacobian() override final;
  void computeOffDiagJacobian() override final;
  ADReal computeQpResidual() override final;

  /// The Lagrange Multiplier variable
  const MooseVariableScalar & _lambda_var;

  /// The Lagrange Multiplier value
  const ADVariableValue & _lambda;
};

#endif
