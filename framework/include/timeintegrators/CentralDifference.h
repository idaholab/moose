//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitTimeIntegrator.h"
// #include "MeshChangedInterface.h"

// Forward declarations
class CentralDifference;

template <>
InputParameters validParams<CentralDifference>();

/**
 * Implements a truly explicit (no nonlinear solve) Central Difference time
 * integration scheme.
 */
class CentralDifference : public ExplicitTimeIntegrator
{
public:
  CentralDifference(const InputParameters & parameters);

  virtual int order() override { return 2; }

  virtual void computeTimeDerivatives() override;

  void computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof) const override;

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
