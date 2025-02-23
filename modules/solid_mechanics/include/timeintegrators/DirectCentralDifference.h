//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitTimeIntegrator.h"

// Forward declarations
namespace libMesh
{
template <typename T>
class SparseMatrix;
}

/**
 * Implements a form of the central difference time integrator that calculates acceleration directly
 * from the residual forces.
 */
class DirectCentralDifference : public ExplicitTimeIntegrator
{
public:
  static InputParameters validParams();

  DirectCentralDifference(const InputParameters & parameters);

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;

  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual bool overridesSolve() const override { return true; }

  virtual bool performExplicitSolve(SparseMatrix<Number> & mass_matrix) override;

  void computeADTimeDerivatives(ADReal &, const dof_id_type &, ADReal &) const override
  {
    mooseError("NOT SUPPORTED");
  }

protected:
  virtual TagID massMatrixTagID() const override;

  const bool & _constant_mass;

  const TagName & _mass_matrix;

  /// The older solution
  const NumericVector<Number> & _solution_older;
  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2, typename T3, typename T4>
  void
  computeTimeDerivativeHelper(T & u_dot, T2 & u_dotdot, const T3 & u_old, const T4 & u_older) const;

  void computeICs();

private:
  /// flag to only compute once. PR reviewer do not let this in
  bool _compute_once;
};

template <typename T, typename T2, typename T3, typename T4>
void
DirectCentralDifference::computeTimeDerivativeHelper(T & u_dot,
                                                     T2 & u_dotdot,
                                                     const T3 & u_old,
                                                     const T4 & u_older) const
{
  // computing first derivative
  // using the Central Difference method
  // u_dot_old = (first_term - second_term) / 2 / dt
  //       first_term = u
  //      second_term = u_older
  u_dot -= u_older; // 'older than older' solution
  u_dot *= 1.0 / (2.0 * _dt);

  // computing second derivative
  // using the Central Difference method
  // u_dotdot_old = (first_term - second_term + third_term) / dt / dt
  //       first_term = u
  //      second_term = 2 * u_old
  //       third_term = u_older
  u_dotdot -= u_old;
  u_dotdot -= u_old;
  u_dotdot += u_older;
  u_dotdot *= 1.0 / (_dt * _dt);
}
