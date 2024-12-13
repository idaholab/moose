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

/**
 * Explicit strong stability preserving Runge-Kutta methods
 */
class ExplicitSSPRungeKutta : public ExplicitTimeIntegrator
{
public:
  static InputParameters validParams();

  ExplicitSSPRungeKutta(const InputParameters & parameters);

  virtual void computeTimeDerivatives() override;
  virtual void computeADTimeDerivatives(ADReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        ADReal & ad_u_dotdot) const override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual int order() override { return _order; }
  virtual bool overridesSolve() const override { return true; }

protected:
  /**
   * Solves a stage of the time integrator
   *
   * @returns true if converged, false if not
   */
  bool solveStage();

  virtual Real duDotDuCoeff() const override;

  /// Order of time integration
  const MooseEnum & _order;

  /// Number of stages
  unsigned int _n_stages;
  /// Runge-Kutta "a" coefficient matrix
  std::vector<std::vector<Real>> _a;
  /// Runge-Kutta "b" coefficient vector
  std::vector<Real> _b;
  /// Runge-Kutta "c" coefficient vector
  std::vector<Real> _c;
  /// Current stage
  unsigned int _stage;
  /// Pointer to solution vector for each stage
  std::vector<const NumericVector<Number> *> _solution_stage;

  /// Solution vector for intermediate stage
  NumericVector<Number> * _solution_intermediate_stage;
  /// Temporary solution vector
  NumericVector<Number> * _tmp_solution;
  /// Temporary mass-matrix/solution vector product
  NumericVector<Number> * _tmp_mass_solution_product;
};
