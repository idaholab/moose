//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActuallyExplicitEuler.h"
#include "MeshChangedInterface.h"

// Forward declarations
class CentralDifference;

template <>
InputParameters validParams<CentralDifference>();

/**
 * Implements a truly explicit (no nonlinear solve) first-order, forward Euler
 * time integration scheme.
 */
class CentralDifference : public ActuallyExplicitEuler
{
public:
  CentralDifference(const InputParameters & parameters);

  virtual void computeTimeDerivatives() override;

  virtual NumericVector<Number> & uDotDotResidual() const override;

  virtual NumericVector<Number> & uDotResidual() const override;

protected:
  /// solution vector for \f$ {du^dotdot}\over{du} \f$
  Real & _du_dotdot_du;

  /// vector storing residual corresponding to the second time derivative
  NumericVector<Number> & _u_dotdot_residual;

  /// vector storing residual corresponding to the first time derivative
  NumericVector<Number> & _u_dot_residual;
};
