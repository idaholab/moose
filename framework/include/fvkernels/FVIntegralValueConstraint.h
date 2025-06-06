//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVScalarLagrangeMultiplierConstraint.h"

/**
 * This Kernel implements the residuals that enforce the constraint
 *
 * \int \phi = \int \phi_0
 *
 * using a Lagrange multiplier approach. E.g. this kernel enforces the constraint that the average
 * value of \phi matches \phi_0
 *
 * In particular, this Kernel implements the residual contribution for
 * the lambda term in Eq. (5), and both terms in Eq. (6) where \int \phi_0 = V_0
 *
 * [0]: https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf
 */
class FVIntegralValueConstraint : public FVScalarLagrangeMultiplierConstraint
{
public:
  static InputParameters validParams();

  FVIntegralValueConstraint(const InputParameters & parameters);

private:
  ADReal computeQpResidual() override final;
};
