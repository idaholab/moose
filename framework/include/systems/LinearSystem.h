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
// #include "ConstraintWarehouse.h"
#include "MooseObjectWarehouse.h"
#include "MooseObjectTagWarehouse.h"
#include "PerfGraphInterface.h"
// #include "ComputeMortarFunctor.h"
// #include "MooseHashing.h"

#include "libmesh/transient_system.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/linear_solver.h"

// Forward declarations
// class FEProblemBase;
// class MoosePreconditioner;
// class JacobianBlock;
// class TimeIntegrator;
// class Predictor;
// class ElementDamper;
// class NodalDamper;
// class GeneralDamper;
// class GeometricSearchData;
// class IntegratedBCBase;
// class NodalBCBase;
// class DirichletBCBase;
// class ADDirichletBCBase;
class LinearFVKernel;
// class InterfaceKernelBase;
// class ScalarKernelBase;
// class DiracKernelBase;
// class NodalKernelBase;
// class Split;
// class KernelBase;
// class BoundaryCondition;
// class ResidualObject;

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

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() {}

  virtual LinearSolver<Number> * linearSolver(const unsigned int /*sys_num*/) { return nullptr; }

  // virtual KSP getKSP() { return KSP; }

  // Setup Functions ////
  virtual void initialSetup() override;
  virtual void timestepSetup() override;
  virtual void customSetup(const ExecFlagType & exec_type) override;
  virtual void residualSetup() override {}
  virtual void jacobianSetup() override {}

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  virtual bool converged() { return false; }

  /**
   * Add a time integrator
   * @param type Type of the integrator
   * @param name The name of the integrator
   * @param parameters Integrator params
   */
  void addTimeIntegrator(const std::string & type,
                         const std::string & name,
                         InputParameters & parameters) override;
  using SystemBase::addTimeIntegrator;

  void setInitialSolution();

  /**
   * Computes right hand side for a given tag
   * @param rhs Right hand side is formed in here
   * @param tag_id tag of kernels for which the residual is to be computed.
   */
  void computeRightHandSideTag(NumericVector<Number> & rhs, TagID tag_id);

  /**
   * Form multiple tag-associated right hand side vectors for all the given tags
   */
  void computeRightHandSideTags(const std::set<TagID> & tags);

  void computeLinearSystemTags(const std::set<TagID> & vector_tags,
                               const std::set<TagID> & matrix_tags);

  void computeLinearSystemInternal(const std::set<TagID> & vector_tags,
                                   const std::set<TagID> & matrix_tags);

  /**
   * Form a right hand side vector for a given tag
   */
  void computeRightHandSide(NumericVector<Number> & rhs, TagID tag_id);

  /**
   * Computes multiple (tag associated) system matrices
   */
  void computeSystemMatrixTags(const std::set<TagID> & tags);

  /**
   * Associate jacobian to systemMatrixTag, and then form a matrix for all the tags
   */
  void computeSystemMatrix(SparseMatrix<Number> & matrix, const std::set<TagID> & tags);

  /**
   * Take all tags in the system, and form a matrix for all tags in the system
   */
  void computeSystemMatrix(SparseMatrix<Number> & matrix);

  /**
   * Return a reference to the stored linear implicit system
   */
  LinearImplicitSystem & linearImplicitSystem() { return _linear_implicit_system; }

  /**
   * Called at the beginning of the time step
   */
  void onTimestepBegin();

  virtual void setSolution(const NumericVector<Number> & soln);

  /**
   * Update active objects of Warehouses owned by LinearSystem
   */
  void updateActive(THREAD_ID tid);

  /**
   *  Return a numeric vector that is associated with the time tag.
   */
  NumericVector<Number> & getRightHandSideTimeVector();

  /**
   * Return a numeric vector that is associated with the nontime tag.
   */
  NumericVector<Number> & getRightHandSideNonTimeVector();

  /**
   * Return a right hand side vector that is associated with the residual tag.
   */
  NumericVector<Number> & rightHandSideVector(TagID tag);

  const NumericVector<Number> * const & currentSolution() const override
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

  void setPCSide(MooseEnum pcs);

  Moose::PCSideType getPCSide() { return _pc_side; }

  void setMooseKSPNormType(MooseEnum kspnorm);

  Moose::MooseKSPNormType getMooseKSPNormType() { return _ksp_norm; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  TagID rightHandSideTimeVectorTag() const { return _rhs_time_tag; }
  TagID rightHandSideNonTimeVectorTag() const { return _rhs_non_time_tag; }
  TagID rightHandSideVectorTag() const { return _rhs_tag; }
  TagID systemMatrixTag() const override { return _system_matrix_tag; }

  // non-const getters
  NumericVector<Number> * solutionUDot() override { return nullptr; }
  NumericVector<Number> * solutionUDotOld() override { return nullptr; }
  NumericVector<Number> * solutionUDotDot() override { return nullptr; }
  NumericVector<Number> * solutionUDotDotOld() override { return nullptr; }
  // const getters
  const NumericVector<Number> * solutionUDot() const override { return nullptr; }
  const NumericVector<Number> * solutionUDotOld() const override { return nullptr; }
  const NumericVector<Number> * solutionUDotDot() const override { return nullptr; }
  const NumericVector<Number> * solutionUDotDotOld() const override { return nullptr; }

  // serialization
  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution() override;

  FEProblemBase & _fe_problem;
  System & _sys;
  unsigned int _current_l_its;

protected:
  /**
   * Compute the right hand side for a given tag
   * @param tags The tags of kernels for which the residual is to be computed.
   */
  void computeRightHandSideInternal(const std::set<TagID> & tags);

  /**
   * Form multiple matrices for all the tags. Users should not call this func directly.
   */
  void computeSystemMatrixInternal(const std::set<TagID> & tags);

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

  /// residual vector for non-time contributions
  NumericVector<Number> * _rhs_non_time;

  /// Used for the residual vector from PETSc
  TagID _rhs_tag;

  /// Tag for non-time contribution Jacobian
  TagID _system_matrix_non_time_tag;

  /// Tag for every contribution to system matrix
  TagID _system_matrix_tag;

  /// Preconditioning side
  Moose::PCSideType _pc_side;
  /// KSP norm type
  Moose::MooseKSPNormType _ksp_norm;

  unsigned int _n_linear_iters;

  Real _final_linear_residual;

  LinearImplicitSystem & _linear_implicit_system;

private:
  /// The current states of the solution (0 = current, 1 = old, etc)
  std::vector<NumericVector<Number> *> _solution_state;

  /// Serialized version of the solution vector, or nullptr if a
  /// serialized solution is not needed
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  bool _solution_is_invalid;
};
