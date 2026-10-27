//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorTimeDerivative.h"
#include "NonADFunctorInterface.h"

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
 *
 * The coefficient is a functor, so in the input file it may be a constant,
 * a Function, a functor material property, a variable, or a postprocessor.
 */
class CoefVectorTimeDerivative : public VectorTimeDerivative, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  CoefVectorTimeDerivative(const InputParameters & parameters);

protected:
  /// coef * (dA/dt) . test
  virtual Real computeQpResidual() override;

  /// coef * (d(dA/dt)/dA) * phi . test
  virtual Real computeQpJacobian() override;

private:
  /// Scalar coefficient functor (constant, Function, functor material property, ...)
  const Moose::Functor<Real> & _coef;
};
