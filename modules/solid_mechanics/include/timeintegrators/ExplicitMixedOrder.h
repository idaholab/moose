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
class ExplicitMixedOrder : public ExplicitTimeIntegrator
{
public:
  static InputParameters validParams();

  ExplicitMixedOrder(const InputParameters & parameters);

  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;

  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;
  virtual bool overridesSolve() const override { return true; }

  virtual void postSolve() override
  { // Once we have the new solution, we want to adanceState to make sure the
    // coupling between the solution and the computed material properties is kept correctly.
    _fe_problem.advanceState();
  }
  virtual bool advancesProblemState() const override { return true; }

  virtual bool performExplicitSolve(SparseMatrix<Number> & mass_matrix) override;

  void computeADTimeDerivatives(ADReal &, const dof_id_type &, ADReal &) const override
  {
    mooseError("NOT SUPPORTED");
  }
  virtual void init() override;

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
  virtual TagID massMatrixTagID() const override;
  virtual TagID dampingMatrixTagID() const;

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