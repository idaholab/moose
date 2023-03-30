//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVBoundaryScalarLagrangeMultiplierConstraint.h"

/**
 * This boundary object implements the residuals that enforce the constraint on the boundary
 *
 * \int \phi = \int \phi_0
 *
 * using a Lagrange multiplier approach. E.g. this boundary object  enforces the constraint that the
 * average value of \phi on the boundary matches \phi_0
 */
class FVBoundaryIntegralValueConstraint : public FVBoundaryScalarLagrangeMultiplierConstraint
{
public:
  static InputParameters validParams();

  FVBoundaryIntegralValueConstraint(const InputParameters & parameters);

private:
  ADReal computeQpResidual() override final;
};
