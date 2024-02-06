//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SystemBase.h"
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
class LinearSystem : public SystemBase, public PerfGraphInterface
{
public:
  LinearSystem(FEProblemBase & problem, const std::string & name);
  virtual ~LinearSystem();

  virtual void init() override;
  virtual void solve() override;
  virtual void restoreSolutions() override;

  virtual void addTimeIntegrator(const std::string & type,
                                 const std::string & name,
                                 InputParameters & parameters) override;
  using SystemBase::addTimeIntegrator;

  /**
   * Compute the right hand side of the system for given tags.
   * @param tags The IDs of the vector tags whose contribution should be included
   */
  void computeRightHandSideTags(const std::set<TagID> & tags);

  /**
   * Compute the right hand side and the system matrix of the system for given tags.
   * @param vector_tags The IDs of the vector tags whose right hand side contribution should be
   * included
   * @param matrix_tags The IDs of the matrix tags whose matrix contribution should be included
   */
  void computeLinearSystemTags(const std::set<TagID> & vector_tags,
                               const std::set<TagID> & matrix_tags);

  /**
   * Compute the system matrix of the system for given tags.
   * @param matrix_tags The IDs of the matrix tags whose contribution should be included
   */
  void computeSystemMatrixTags(const std::set<TagID> & tags);

  /**
   * Return a reference to the stored linear implicit system
   */
  LinearImplicitSystem & linearImplicitSystem() { return _linear_implicit_system; }

  /**
   * Set the solution to a given vector.
   * @param soln The vector which should be treated as the solution.
   */
  void setSolution(const NumericVector<Number> & soln);

  /**
   *  Return a numeric vector that is associated with the time tag.
   */
  NumericVector<Number> & getRightHandSideTimeVector();

  /**
   * Return a numeric vector that is associated with the nontime tag.
   */
  NumericVector<Number> & getRightHandSideNonTimeVector();

  virtual const NumericVector<Number> * const & currentSolution() const override
  {
    return _current_solution;
  }

  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<dof_id_type> & n_nz,
                               std::vector<dof_id_type> & n_oz) override;

  ///@{
  /// System Integrity Checks
  void checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains) const;
  ///@}

  /**
   * Return the number of linear iterations
   */
  unsigned int nLinearIterations() const { return _n_linear_iters; }

  /**
   * Set the side on which the preconditioner is applied to.
   * @param pcs The required preconditioning side
   */
  void setPCSide(MooseEnum pcs);

  /**
   * Get the current preconditioner side.
   */
  Moose::PCSideType getPCSide() { return _pc_side; }

  /**
   * Set the norm in which the linear convergence will be measured.
   * @param kspnorm The required norm
   */
  void setMooseKSPNormType(MooseEnum kspnorm);

  /**
   * Get the norm in which the linear convergence is measured.
   */
  Moose::MooseKSPNormType getMooseKSPNormType() { return _ksp_norm; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  ///@{
  /// Accersors of important tag IDs
  TagID rightHandSideTimeVectorTag() const { return _rhs_time_tag; }
  TagID rightHandSideNonTimeVectorTag() const { return _rhs_non_time_tag; }
  TagID rightHandSideVectorTag() const { return _rhs_tag; }
  TagID systemMatrixTag() const override { return _system_matrix_tag; }
  ///@}

  /// Serialize the distributed solution vector
  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution() override;

  /// Reference to the problem
  FEProblemBase & _fe_problem;

  /// Base class reference to the libmesh system
  System & _sys;

  /// The linear iterations needed for convergence
  unsigned int _current_l_its;

protected:
  /**
   * Compute the right hand side for given tags.
   * @param tags The tags of kernels for which the right hand side is to be computed.
   */
  void computeRightHandSideInternal(const std::set<TagID> & tags);

  /**
   * Compute the system matrix for given tags.
   * @param tags The tags of kernels for which the matrix is to be computed.
   */
  void computeSystemMatrixInternal(const std::set<TagID> & tags);

  /**
   * Compute the right hand side and system matrix for given tags
   * @param vector_tags The tags of kernels for which the right hand side is to be computed.
   * @param matrix_tags The tags of kernels for which the system matrix is to be computed.
   */
  void computeLinearSystemInternal(const std::set<TagID> & vector_tags,
                                   const std::set<TagID> & matrix_tags);

  NumericVector<Number> & solutionInternal() const override { return *_sys.solution; }

  /// solution vector from linear solver
  const NumericVector<Number> * _current_solution;

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

  /// Tag for non-time contribution Jacobian
  TagID _system_matrix_non_time_tag;

  /// Tag for every contribution to system matrix
  TagID _system_matrix_tag;

  /// Preconditioning side
  Moose::PCSideType _pc_side;
  /// KSP norm type
  Moose::MooseKSPNormType _ksp_norm;

  /// Number of linear iterations
  unsigned int _n_linear_iters;

  /// The final linear residual
  Real _final_linear_residual;

  /// Base class reference to the linear implicit system in libmesh
  LinearImplicitSystem & _linear_implicit_system;

private:
  /// The current states of the solution (0 = current, 1 = old, etc)
  std::vector<NumericVector<Number> *> _solution_state;

  /// Serialized version of the solution vector, or nullptr if a
  /// serialized solution is not needed
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Boolean to see if solution is invalid
  bool _solution_is_invalid;
};
