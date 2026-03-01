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
#include "libmesh/stored_range.h"
#include <memory>

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
  virtual void initialSetup() override;
  virtual void init() override;

  virtual void meshChanged() override;

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
  /// compile the dof indices for first and second order in time variables
  void updateDOFIndices();

  virtual TagID massMatrixTagID() const override;

  /// Evaluate the RHS residual
  virtual void evaluateRHSResidual();

  /// Whether we are reusing the mass matrix
  const bool & _constant_mass;

  /**
   * Must be set to true to use adaptivity with a constant mass
   * matrix. This will recompute the mass matrix when the mesh changes. The user must make sure that
   * the underlying material density stays constant, otherwise simulation results will depend on
   * adaptivity.
   */
  const bool & _recompute_mass_matrix_on_mesh_change;

  /// Whether the mesh changed just before the current solve
  bool _mesh_changed;

  /// Mass matrix name
  const TagName & _mass_matrix_name;

  /// Lumped mass matrix
  NumericVector<Real> * _mass_matrix_lumped;

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

  // mesh blocks over which the first and second order variables are defined
  std::set<SubdomainID> _relevant_blocks;

  // whether _relevant_blocks is equal to all the blocks in the mesh
  bool _all_mesh_is_relevant;

  // whether to constructRanges and then setCurrentAlgebraic{Element,Node}Range
  const bool & _restrict_to_active_blocks;

  // Union element range over all _relevant_blocks
  std::unique_ptr<libMesh::ConstElemRange> _elem_range;
  // Node range derived from _elem_range
  std::unique_ptr<libMesh::ConstNodeRange> _node_range;

  // Backing buffers when constructing StoredRange from packed vectors
  std::vector<const libMesh::Elem *> _elem_buffer;
  std::vector<const libMesh::Node *> _node_buffer;

  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2, typename T3, typename T4>
  void
  computeTimeDerivativeHelper(T & u_dot, T2 & u_dotdot, const T3 & u_old, const T4 & u_older) const;

  void computeICs();

  /**
   * Using _relevant_blocks, construct _elem_buffer, _elem_range, _node_buffer, _node_range
   */
  virtual void constructRanges();

  /*
   * If _restrict_to_active_blocks and !_all_mesh_is_relevant, then using _relevant_blocks, do the
   * following:
   * - calculate and apply appropriate elemental and nodal ranges
   * - update the _nonlinear_implicit_system
   * - apply appropriate ghosting
   */
  virtual void computeAndApplyRanges();

private:
  /**
   * Use _elem_range and _node_range to setcurrentAlgebraicElementRange and
   * setCurrentAlgebraicNodeRange
   */
  void setCurrentAlgebraicRanges();

  /**
   * if _restrict_to_active_blocks, then constructs _relevant_blocks and _all_mesh_is_relevant
   */
  void buildRelevantBlocks(const std::vector<VariableName> & var_names_first,
                           const std::vector<VariableName> & var_names_second);

  // Collects all off-rank DOF indices present in the given element range for use as ghost IDs
  void buildGhostIDs(const libMesh::ConstElemRange & elems,
                     std::vector<dof_id_type> & ghost_ids) const;

  /**
   * Rebuild ghost IDs from the current algebraic element range and re-init ghosted vectors
   * _mass_matrix_diag_inverted and _mass_matrix_lumped
   */
  void reinitGhostedVectorsForCurrentAlgebraicRange();
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
