//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// This file is part of the MOOSE framework
// https://mooseframework.inl.gov
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorTimeDerivative.h"

/**
 * Implements the time derivative term for a vector variable
 * with a scalar coefficient:
 *
 *   coef * dA/dt
 *
 * Used for the magnetoquasistatic EM equation:
 *
 *   mu0 * sigma * dA/dt = nabla^2 A + mu0 * J
 *
 * where coef = mu0 * sigma.
 */
class CoefVectorTimeDerivative : public VectorTimeDerivative
{
public:
  /**
   * Constructor — takes InputParameters as all MOOSE objects do.
   */
  CoefVectorTimeDerivative(const InputParameters & parameters);

  /**
   * validParams declares the parameters this kernel accepts.
   * Body must be in the .C file.
   */
  static InputParameters validParams();

protected:
  /**
   * Computes the residual contribution at one quadrature point:
   *   coef * (dA/dt) . test
   */
  virtual Real computeQpResidual() override;

  /**
   * Computes the Jacobian contribution at one quadrature point:
   *   coef * (dphi/dt) . test
   */
  virtual Real computeQpJacobian() override;

private:
  /// Scalar coefficient multiplying the time derivative (mu0 * sigma)
  const Real _coef;
};
