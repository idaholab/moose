//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"
#include "NonlinearTimeIntegratorInterface.h"
#include "LinearTimeIntegratorInterface.h"

class FEProblemBase;
class SystemBase;
class NonlinearSystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
} // namespace libMesh

/**
 * Base class for time integrators
 *
 * Time integrators fulfill two functions:
 * 1) computing u_dot vector (used for computing time derivatives in kernels) and its derivative
 * 2) combining the residual vectors into the final one
 *
 * Capability (1) is used by both NonlinearSystem and AuxiliarySystem, while (2) can be obviously
 * used
 * only by NonlinearSystem (AuxiliarySystem does not produce residual).
 */
class TimeIntegrator : public MooseObject,
                       public Restartable,
                       public NonlinearTimeIntegratorInterface,
                       public LinearTimeIntegratorInterface
{
public:
  static InputParameters validParams();

  TimeIntegrator(const InputParameters & parameters);

  /**
   * Called to setup datastructures.
   *
   * WILL be called during recover/restart
   *
   * Should NOT do any computation for initial values
   * use init() for that
   *
   * Note: this doesn't inherit this from SetupInterface because
   * I'm not sure that we need all the other setup functions for
   * TimeIntegrator
   */
  virtual void initialSetup() {}

  /**
   * Called _only_ before the very first timestep (t_step = 0)
   * Never called again (not even during recover/restart)
   */
  virtual void init();
  virtual void preSolve() {}
  virtual void preStep() {}

  /**
   * Solves the time step and sets the number of nonlinear and linear iterations.
   */
  virtual void solve();

  /**
   * Callback to the TimeIntegrator called immediately after
   * TimeIntegrator::solve() (so the name does make sense!).  See
   * e.g. CrankNicolson for an example of what can be done in the
   * postSolve() callback -- there it is used to move the residual
   * from the "old" timestep forward in time to avoid recomputing it.
   */
  virtual void postSolve() {}

  /**
   * Callback to the TimeIntegrator called at the very end of time step.
   */
  virtual void postStep() {}

  virtual int order() = 0;

  /**
   * Computes the time derivative and the Jacobian of the time derivative
   *
   * This function is responsible for the following:
   * - computing the time derivative; a reference is retrieved from _sys.solutionUDot().
   * - computing the time derivative Jacobian, stored in _du_dot_du, which is a
   *   reference to _sys.duDotDus().
   */
  virtual void computeTimeDerivatives() = 0;

  /**
   * method for computing local automatic differentiation time derivatives
   */
  virtual void computeADTimeDerivatives(ADReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        ADReal & ad_u_dot_dot) const = 0;

  /**
   * Gets the total number of nonlinear iterations over all stages of the time step.
   */
  virtual unsigned int getNumNonlinearIterations() const { return _n_nonlinear_iterations; }

  /**
   * Gets the total number of linear iterations over all stages of the time step.
   */
  virtual unsigned int getNumLinearIterations() const { return _n_linear_iterations; }

  /**
   * Returns the time step size
   * @return The time step size
   */
  const Real & dt() const { return _dt; }

  /**
   * Returns whether the explicit solvers are used
   */
  virtual bool isExplicit() const { return false; }

  /**
   * Return the number of states this requires in a linear
   * system setting
   */
  virtual unsigned int numStatesRequired() const { return 1; }

  /**
   * Returns whether mass matrix is lumped
   */
  virtual const bool & isLumped() const { return _is_lumped; }

  /**
   * @returns whether this integrator integrates the given variable
   */
  bool integratesVar(const unsigned int var_num) const;

  /**
   * Record the linear and nonlinear iterations from a just finished solve
   */
  void setNumIterationsLastSolve();

  /**
   * @returns Whether the virtual solve method is overridden
   */
  virtual bool overridesSolve() const = 0;

protected:
  /**
   * Gets the number of nonlinear iterations in the most recent solve.
   */
  unsigned int getNumNonlinearIterationsLastSolve() const;

  /**
   * Gets the number of linear iterations in the most recent solve.
   */
  unsigned int getNumLinearIterationsLastSolve() const;

  /**
   * Copy from one vector into another. If the time integrator has been restricted to a subset of
   * variables, then this will selectively copy their dofs
   */
  void copyVector(const NumericVector<Number> & from, NumericVector<Number> & to);

  /**
   * @returns The \p _du_dot_du multiplicative coefficient, e.g. if \p _du_dot_du is equivalent to
   * 2/dt, then this method returns 2
   */
  virtual Real duDotDuCoeff() const { return 1; }

  /**
   * Compute \p _du_dot_du
   */
  void computeDuDotDu();

  /// Reference to the problem
  FEProblemBase & _fe_problem;

  /// Reference to the system this time integrator operates on
  SystemBase & _sys;

  /// Derivative of time derivative with respect to current solution: \f$ {du^dot}\over{du} \f$ for
  /// the different variables. We will only modify the elements in this vector corresponding to the
  /// variables that we integrate
  std::vector<Real> & _du_dot_du;

  /// @{
  /// Solution vectors for different states and variable restrictions
  const NumericVector<Number> * const & _solution;
  const NumericVector<Number> & _solution_old;
  std::unique_ptr<NumericVector<Number>> & _solution_sub;
  std::unique_ptr<NumericVector<Number>> & _solution_old_sub;
  ///@}

  /// The current time step number
  int & _t_step;

  /// The current time step size
  Real & _dt;

  /// The previous time step size
  Real & _dt_old;

  /// Total number of nonlinear iterations over all stages of the time step
  unsigned int _n_nonlinear_iterations;
  /// Total number of linear iterations over all stages of the time step
  unsigned int _n_linear_iterations;

  /// Boolean flag that is set to true if lumped mass matrix is used
  bool _is_lumped;

  /// Whether the user has requested that the time integrator be applied to a subset of variables
  bool & _var_restriction;

  /// The local degree of freedom indices this time integrator is being applied to. If this
  /// container is empty then the time integrator is applied to all indices
  std::vector<dof_id_type> & _local_indices;

  /// The variables that this time integrator integrates
  std::unordered_set<unsigned int> & _vars;

  /// A vector that is used for creating 'from' subvectors in the \p copyVector() method
  std::unique_ptr<NumericVector<Number>> _from_subvector;
};
