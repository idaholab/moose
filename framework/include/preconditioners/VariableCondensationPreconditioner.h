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

private:
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
  void findZeroDiagonals(SparseMatrix<Number> & mat, std::vector<dof_id_type> & indices);

  /**
   * Reconstruct the equation system
   */
  void condenseSystem();

  /**
   * Preallocate memory for the condensed Jacobian matrix
   */
  void preallocateCondensedJacobian(PetscMatrix<Number> & condensed_mat,
                                    PetscMatrix<Number> & original_mat,
                                    const std::vector<dof_id_type> & rows,
                                    const std::vector<dof_id_type> & cols,
                                    const std::vector<dof_id_type> & grows,
                                    const std::vector<dof_id_type> & gcols,
                                    PetscMatrix<Number> & block_mat);

  /**
   * The condensed Jacobian matrix is computed in this function.
   */
  void computeCondensedJacobian(PetscMatrix<Number> & condensed_mat,
                                PetscMatrix<Number> & original_mat,
                                const std::vector<dof_id_type> & grows,
                                PetscMatrix<Number> & block_mat);
  /**
   * Compute inverse of D using LU. This method is used when _is_lm_coupling_diagonal = false.
   */
  void computeDInverse(Mat & mat);

  /**
   * Compute (approximate) inverse of D by inverting its diagonal entries. This method is used when
   * _is_lm_coupling_diagonal = true.
   */
  void computeDInverseDiag(Mat & mat);

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

  /**
   * Find the common part of arrays \p a and \p b and save it in \p c. The \p na and \p nb are the
   * number of entries for the arrays. This function is used in the preallocateCondensedJacobian
   * step.
   */
  void mergeArrays(const PetscInt * a,
                   const PetscInt * b,
                   const PetscInt & na,
                   const PetscInt & nb,
                   std::vector<PetscInt> & c);

  /// The nonlinear system this PC is associated with (convenience reference)
  NonlinearSystemBase & _nl;

  /// Mesh object for easy reference
  MooseMesh & _mesh;

  /// DofMap for easy reference
  const DofMap & _dofmap;

  /// Whether the coupling is diagonal
  const bool _is_lm_coupling_diagonal;

  /// Whether to condense all specified variable
  const bool _adaptive_condensation;

  /// Number of variables
  const unsigned int _n_vars;

  /// Name and ID of the variables that are to be condensed out (usually the Lagrange multiplier variable)
  const std::vector<std::string> _lm_var_names;
  std::vector<unsigned int> _lm_var_ids;

  /// Name and ID of the corresponding coupled variable
  const std::vector<std::string> _primary_var_names;
  std::vector<unsigned int> _primary_var_ids;

  /// Submatrices that are frequently needed while computing the condensed system
  /// _D: the submatrix that couples the primary variable with the Lagrange multiplier variable
  /// _M: the columns that correspond to the Lagrange multiplier DoFs
  /// _K: the rows that correspond to the primary variable DoFs
  std::unique_ptr<PetscMatrix<Number>> _D, _M, _K;
  /// inverse of _D
  Mat _dinv;

  /// Condensed Jacobian
  std::unique_ptr<PetscMatrix<Number>> _J_condensed;

  /// _x_hat, _y_hat: condensed solution and RHS vectors
  /// _primary_rhs_vec: part of the RHS vector that correspond to the primary variable DoFs
  /// _lm_sol_vec: solution vector that corresponds to the LM variable
  std::unique_ptr<NumericVector<Number>> _x_hat, _y_hat, _primary_rhs_vec, _lm_sol_vec;

  /// The row indices that correspond to the zero diagonal entries in the original Jacobian matrix
  /// This is only used when _adaptive_condensation = true
  std::vector<dof_id_type> _zero_rows;

  /// Whether the DoFs associated the variable are to be condensed. If the DoF list for the variable
  /// to be condensed is empty, we do not carry out static condensation
  mutable bool _need_condense;

  /// Which preconditioner to use for the solve
  PreconditionerType _pre_type;

  /// Holds one Preconditioner object for the condensed system to solve
  std::unique_ptr<Preconditioner<Number>> _preconditioner;

  /// Vectors of DoFs:
  /// indices associated with lagrange multipliers, and its coupled primary variable
  /// _global_<variable_name>_dofs represents the indices that are owned by all processors
  /// _<variable_name>_dofs represents the indices that are owned only by each processor
  ///     lm: the Lagrange multiplier
  ///     primary: the primal variable DoFs that couples with the Lagrange multiplier
  /// Note:
  ///     the global index sets may not be scalable and necessary memory optimization will be investigated later
  std::vector<dof_id_type> _global_lm_dofs, _lm_dofs, _global_primary_dofs, _primary_dofs;

  /// row and column indices for the condensed system
  std::vector<dof_id_type> _global_rows, _rows, _global_cols, _cols;

  /// Maps to keep track of row and col indices from the original Jacobian matrix to the condensed Jacobian matrix
  std::unordered_map<dof_id_type, dof_id_type> _global_rows_to_idx, _rows_to_idx,
      _global_cols_to_idx, _cols_to_idx;

  /// Maps to keep track of the dof orders for keeping nonzero diagonal entries of the condensed system
  /// _map_global_lm_primary: map between _global_lm_dofs and _global_primary_dofs.
  /// _map_global_primary_order: map between _global_primary_dofs and the corresponding row index in _D
  std::unordered_map<dof_id_type, dof_id_type> _map_global_lm_primary, _map_global_primary_order;

  /// Timers
  PerfID _init_timer;
  PerfID _apply_timer;
};
