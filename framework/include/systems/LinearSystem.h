//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolverSystem.h"
#include "PerfGraphInterface.h"

#include "libmesh/transient_system.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/linear_solver.h"

class LinearFVKernel;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
template <typename T>
class SparseMatrix;
template <typename T>
class DiagonalMatrix;
} // namespace libMesh

/**
 * Linear system to be solved
 */
class LinearSystem : public SolverSystem, public PerfGraphInterface
{
public:
  LinearSystem(FEProblemBase & problem, const std::string & name);
  virtual ~LinearSystem();

  virtual void solve() override;

  virtual void initialSetup() override;

  // Overriding these to make sure the linear systems don't do anything during
  // residual/jacobian setup
  virtual void residualSetup() override {}
  virtual void jacobianSetup() override {}

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve(const ExecFlagType & exec_flag) override;

  /**
   * Compute the right hand side and the system matrix of the system for given tags.
   * @param vector_tags The IDs of the vector tags whose right hand side contribution should be
   * included
   * @param matrix_tags The IDs of the matrix tags whose matrix contribution should be included
   * @param compute_gradients A flag to disable the computation of new gradients during the
   * assembly, can be used to lag gradients
   */
  void computeLinearSystemTags(const std::set<TagID> & vector_tags,
                               const std::set<TagID> & matrix_tags,
                               const bool compute_gradients = true);

  /**
   * Return a reference to the stored linear implicit system
   */
  LinearImplicitSystem & linearImplicitSystem() { return _linear_implicit_system; }

  /**
   *  Return a numeric vector that is associated with the time tag.
   */
  NumericVector<Number> & getRightHandSideTimeVector();

  /**
   * Return a numeric vector that is associated with the nontime tag.
   */
  NumericVector<Number> & getRightHandSideNonTimeVector();

  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<dof_id_type> & n_nz,
                               std::vector<dof_id_type> & n_oz) override;

  /**
   * Return the number of linear iterations
   */
  unsigned int nLinearIterations() const { return _n_linear_iters; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  ///@{
  /// Accessors of important tag IDs
  TagID rightHandSideTimeVectorTag() const { return _rhs_time_tag; }
  TagID rightHandSideNonTimeVectorTag() const { return _rhs_non_time_tag; }
  TagID rightHandSideVectorTag() const { return _rhs_tag; }
  virtual TagID systemMatrixTag() const override { return _system_matrix_tag; }
  ///@}

  /**
   * Compute the Green-Gauss gradients
   */
  void computeGradients();

  /**
   * Return a reference to the new (temporary) gradient container vectors
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> & newGradientContainer()
  {
    return _new_gradient;
  }

protected:
  /**
   * Compute the right hand side and system matrix for given tags
   * @param vector_tags The tags of kernels for which the right hand side is to be computed.
   * @param matrix_tags The tags of kernels for which the system matrix is to be computed.
   * @param compute_gradients A flag to disable the computation of new gradients during the
   * assembly, can be used to lag gradients
   */
  void computeLinearSystemInternal(const std::set<TagID> & vector_tags,
                                   const std::set<TagID> & matrix_tags,
                                   const bool compute_gradients = true);

  /// Base class reference to the libmesh system
  System & _sys;

  /// The linear iterations needed for convergence
  unsigned int _current_l_its;

  /// Vector tags to temporarily store all tags associated with the current system.
  std::set<TagID> _vector_tags;

  /// Matrix tags to temporarily store all tags associated with the current system.
  std::set<TagID> _matrix_tags;

  /// Tag for time contribution rhs
  TagID _rhs_time_tag;

  /// right hand side vector for time contributions
  NumericVector<Number> * _rhs_time;

  /// Tag for non-time contribution rhs
  TagID _rhs_non_time_tag;

  /// right hand side vector for non-time contributions
  NumericVector<Number> * _rhs_non_time;

  /// Used for the right hand side vector from PETSc
  TagID _rhs_tag;

  /// Tag for non-time contribution to the system matrix
  TagID _system_matrix_non_time_tag;

  /// Tag for every contribution to system matrix
  TagID _system_matrix_tag;

  /// Number of linear iterations
  unsigned int _n_linear_iters;

  /// The final linear residual
  Real _final_linear_residual;

  /// Base class reference to the linear implicit system in libmesh
  LinearImplicitSystem & _linear_implicit_system;

  /// Vectors to store the new gradients during the computation. This is needed
  /// because the old gradients might still be needed to determine boundary values
  /// (for extrapolated boundary conditions). Once the computation is done, we
  /// move the nev vectors to the original containers.
  std::vector<std::unique_ptr<NumericVector<Number>>> _new_gradient;

private:
  /// The current states of the solution (0 = current, 1 = old, etc)
  std::vector<NumericVector<Number> *> _solution_state;
};
