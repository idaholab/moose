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
#include "MoosePreconditioner.h"

// libMesh includes
#include "libmesh/preconditioner.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/enum_preconditioner_type.h"
#include "libmesh/mesh_base.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/parallel_object.h"

// C++ includes
#include <vector>

// Forward declarations
class NonlinearSystemBase;
class VariableCondensationPreconditioner;

/**
 * Interface for condensing out LMs for the dual mortar approach.
 */
class VariableCondensationPreconditioner : public MoosePreconditioner, public Preconditioner<Number>
{
public:
  static InputParameters validParams();

  VariableCondensationPreconditioner(const InputParameters & params);
  virtual ~VariableCondensationPreconditioner();

  /**
   * Initialize data structures if not done so already.
   */
  virtual void init();

  /**
   * This is called every time the "operator might have changed".
   *
   * This is essentially where you need to fill in your preconditioning matrix.
   */
  virtual void setup();

  /**
   * Computes the preconditioned vector "x" based on input "y".
   * Usually by solving Px=y to get the action of P^-1 y.
   */
  virtual void apply(const NumericVector<Number> & y, NumericVector<Number> & x);

  /**
   * Release all memory and clear data structures.
   */
  virtual void clear();

protected:
  /**
   * Get dofs for the variable to be condensed out
   */
  void getDofToCondense();

  /**
   * Get row and col dofs for the condensed system
   */
  void getDofColRow();

  /**
   * Check if the original jacobian has zero diagonal entries and save the row indices
   */
  void findZeroDiagonals(SparseMatrix<Number> & mat, std::vector<numeric_index_type> & indices);

  /**
   * Reconstruct the equation system
   */
  void condenseSystem();

  /**
   * Compute inverse of D
   */
  void computeDInverse();

  void computeDInverseDiag();

  /**
   * Get condensed x and y
   */
  void getCondensedXY(const NumericVector<Number> & y, NumericVector<Number> & x);

  /**
   * Compute condensed variables (Lagrange multipliers) values using updated solution vector
   */
  void computeCondensedVariables();

  /**
   * Assemble the full solution vector
   */
  void getFullSolution(const NumericVector<Number> & y, NumericVector<Number> & x);

  /// The nonlinear system this PC is associated with (convenience reference)
  NonlinearSystemBase & _nl;

  /// Mesh object for easy reference
  MooseMesh & _mesh;

  /// DofMap for easy reference
  DofMap & _dofmap;

  /// Whether the coupling is diagonal
  const bool _is_lm_coupling_diagonal;

  /// Whether to condense all specified variable
  const bool _adaptive_condensation;

  /// Number of variables
  unsigned int _n_vars;

  /// Name and ID of the variables that are to be condensed out (usually the Lagrange multiplier variable)
  const std::vector<std::string> _lm_var_names;
  std::vector<unsigned int> _lm_var_ids;

  // Name and ID of the corresponding coupled variable
  const std::vector<std::string> _primary_var_names;
  std::vector<unsigned int> _primary_var_ids;

  /// Submatrices that are frequently needed while computing the condensed system
  std::unique_ptr<PetscMatrix<Number>> _D, _M, _D_inv, _MDinv, _u2c_rows;

  /// Condensed Jacobian
  std::unique_ptr<PetscMatrix<Number>> _J_condensed;

  /// Condensed x, y;
  /// Vector for lambda, and part of RHS that is associated with lambda
  std::unique_ptr<NumericVector<Number>> _x_hat, _y_hat, _r2c, _lambda;

  // check zero diagonal entries in the original Jacobian matrix and save the indices
  std::vector<numeric_index_type> _zero_rows;

  /// Whether DOFs info has been saved, make sure only saving the dof info when needed
  mutable bool _save_dofs;

  /// Whether the DOFs associated the variable are to be condensed
  mutable bool _need_condense;

  /// Which preconditioner to use for the solve
  PreconditionerType _pre_type;

  /// Holds one Preconditioner object for the condensed system to solve
  std::unique_ptr<Preconditioner<Number>> _preconditioner;

  /// indices associated with lagrange multipliers, and primary variable on the interface
  /// _g+<variable_name> represents the indices that are owned by all processors
  /// _<variable_name> represents the indices that are owned only by each processor
  /// lm denotes the Lagrange multiplier
  /// u2c denotes the primal variable DoFs that couples with the Lagrange multiplier
  /// Note:
  ///     the global index sets may not be scalable and necessary memory optimization will be investigated later
  std::vector<numeric_index_type> _glm, _lm, _gu2c, _u2c;

  /// row and column indices for the condensed system
  std::vector<numeric_index_type> _grows, _rows, _gcols, _cols;

  /// Maps to keep track of the dof orders for keeping nonzero diagonal entries of the condensed system
  /// map between glm and gu2c.
  std::map<numeric_index_type, numeric_index_type> _map_glm_gu2c, _map_gu2c_glm;
  // gu2c index as keys, the corresponding row index in _D as the value
  std::map<numeric_index_type, numeric_index_type> _map_gu2c_to_order;

  /// Timers
  PerfID _init_timer;
  PerfID _apply_timer;
};
