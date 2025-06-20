//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeIntegrator.h"
#include "MeshChangedInterface.h"
#include "FEProblemBase.h"

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
class ExplicitMixedOrder : public TimeIntegrator, public MeshChangedInterface
{
public:
  static InputParameters validParams();

  ExplicitMixedOrder(const InputParameters & parameters);

  virtual void init() override;
  virtual void meshChanged() override;
  virtual bool isExplicit() const override { return true; }
  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual bool overridesSolve() const override { return true; }
  virtual bool advancesProblemState() const override { return true; }

  virtual void preSolve() override {}
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual void solve() override;
  virtual void postSolve() override;

  void computeADTimeDerivatives(ADReal &, const dof_id_type &, ADReal &) const override
  {
    mooseError("NOT SUPPORTED");
  }

  enum TimeOrder
  {
    FIRST,
    SECOND
  };

  /**
   * Retrieve the order of the highest time derivative of a variable.
   * @return Returns the time order enum of this variable.
   */
  TimeOrder findVariableTimeOrder(unsigned int var_num) const;

protected:
  virtual TagID massMatrixTagID() const;

  virtual TagID dampingMatrixTagID() const;

  /// calculate velocity using the forward Euler method
  virtual void forwardEuler();

  /// calculate acceleration and velocity using the central difference method
  virtual void centralDifference();

  /// Update the solution vector. @return true if the solution converged, false otherwise.
  virtual bool solutionUpdate();

  /// Whether damping is present
  bool _has_damping;

  /// Whether we are reusing the mass matrix
  const bool & _constant_mass;

  /// Whether we aare reusing the damping matrix
  const bool & _constant_damping;

  /// Mass matrix name
  const TagName & _mass_matrix;

  /// Damping matrix name
  const TagName & _damping_matrix;

  /// The older solution
  const NumericVector<Number> & _solution_older;

  /// Residual used for the RHS
  NumericVector<Real> * _explicit_residual;

  /// Solution vector for the linear solve
  NumericVector<Real> * _solution_update;

  /// Diagonal of the lumped mass matrix (and its inversion)
  NumericVector<Real> * _mass_matrix_lumped;

  /// Diagonal of the lumped mass matrix (and its inversion)
  NumericVector<Real> * _damping_matrix_lumped;

  /// Vector of 1's to help with creating the lumped mass matrix
  NumericVector<Real> * _ones;

  /// Save off current time to reset it back and forth
  Real _current_time;

  // Variables that forward Euler time integration will be used for
  std::unordered_set<unsigned int> & _vars_first;

  // local dofs that will have forward euler time integration
  std::vector<dof_id_type> & _local_first_order_indices;

  // Variables that central difference time integration will be used for
  std::unordered_set<unsigned int> & _vars_second;

  // local dofs that will have central difference time integration
  std::vector<dof_id_type> & _local_second_order_indices;

  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2, typename T3, typename T4>
  void
  computeTimeDerivativeHelper(T & u_dot, T2 & u_dotdot, const T3 & u_old, const T4 & u_older) const;

  void computeICs();
};

template <typename T, typename T2, typename T3, typename T4>
void
ExplicitMixedOrder::computeTimeDerivativeHelper(T & u_dot,
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