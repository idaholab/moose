//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NONLINEARSYSTEMBASE_H
#define NONLINEARSYSTEMBASE_H

#include "SystemBase.h"
#include "ConstraintWarehouse.h"
#include "MooseObjectWarehouse.h"
#include "MooseObjectTagWarehouse.h"
#include "PerfGraphInterface.h"

#include "libmesh/transient_system.h"
#include "libmesh/nonlinear_implicit_system.h"

// Forward declarations
class FEProblemBase;
class MoosePreconditioner;
class JacobianBlock;
class TimeIntegrator;
class Predictor;
class ElementDamper;
class NodalDamper;
class GeneralDamper;
class GeometricSearchData;
class IntegratedBCBase;
class NodalBCBase;
class PresetNodalBC;
class DGKernel;
class InterfaceKernel;
class ScalarKernel;
class DiracKernel;
class NodalKernel;
class Split;
class KernelBase;
class BoundaryCondition;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
template <typename T>
class SparseMatrix;
} // namespace libMesh

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblemBase ;-)
 */
class NonlinearSystemBase : public SystemBase,
                            public ConsoleStreamInterface,
                            public PerfGraphInterface
{
public:
  NonlinearSystemBase(FEProblemBase & problem, System & sys, const std::string & name);
  virtual ~NonlinearSystemBase();

  virtual void init() override;

  /**
   * Turn off the Jacobian (must be called before equation system initialization)
   */
  void turnOffJacobian();

  virtual void addExtraVectors() override;
  virtual void solve() override = 0;
  virtual void restoreSolutions() override;

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() = 0;

  virtual NonlinearSolver<Number> * nonlinearSolver() = 0;

  virtual unsigned int getCurrentNonlinearIterationNumber() = 0;

  /**
   * Returns true if this system is currently computing the initial residual for a solve.
   * @return Whether or not we are currently computing the initial residual.
   */
  virtual bool computingInitialResidual() { return _computing_initial_residual; }

  // Setup Functions ////
  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void setupFiniteDifferencedPreconditioner() = 0;
  void setupFieldDecomposition();

  bool haveFiniteDifferencedPreconditioner() const
  {
    return _use_finite_differenced_preconditioner;
  }
  bool haveFieldSplitPreconditioner() const { return _use_field_split_preconditioner; }

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  virtual bool converged() = 0;

  /**
   * Add a time integrator
   * @param type Type of the integrator
   * @param name The name of the integrator
   * @param parameters Integrator params
   */
  void
  addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters);

  /**
   * Adds a kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void
  addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a NodalKernel
   * @param kernel_name The type of the nodal kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void addNodalKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters parameters);

  /**
   * Adds a scalar kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addScalarKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters parameters);

  /**
   * Adds a boundary condition
   * @param bc_name The type of the boundary condition
   * @param name The name of the boundary condition
   * @param parameters Boundary condition parameters
   */
  void addBoundaryCondition(const std::string & bc_name,
                            const std::string & name,
                            InputParameters parameters);

  /**
   * Adds a Constraint
   * @param c_name The type of the constraint
   * @param name The name of the constraint
   * @param parameters Constraint parameters
   */
  void
  addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a Dirac kernel
   * @param kernel_name The type of the dirac kernel
   * @param name The name of the Dirac kernel
   * @param parameters Dirac kernel parameters
   */
  void addDiracKernel(const std::string & kernel_name,
                      const std::string & name,
                      InputParameters parameters);

  /**
   * Adds a DG kernel
   * @param dg_kernel_name The type of the DG kernel
   * @param name The name of the DG kernel
   * @param parameters DG kernel parameters
   */
  void
  addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds an interface kernel
   * @param interface_kernel_name The type of the interface kernel
   * @param name The name of the interface kernel
   * @param parameters interface kernel parameters
   */
  void addInterfaceKernel(std::string interface_kernel_name,
                          const std::string & name,
                          InputParameters parameters);

  /**
   * Adds a damper
   * @param damper_name The type of the damper
   * @param name The name of the damper
   * @param parameters Damper parameters
   */
  void
  addDamper(const std::string & damper_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a split
   * @param split_name The type of the split
   * @param name The name of the split
   * @param parameters Split parameters
   */
  void
  addSplit(const std::string & split_name, const std::string & name, InputParameters parameters);

  /**
   * Retrieves a split by name
   * @param name The name of the split
   */
  std::shared_ptr<Split> getSplit(const std::string & name);

  void zeroVectorForResidual(const std::string & vector_name);

  void setInitialSolution();

  /**
   * Sets the value of constrained variables in the solution vector.
   */
  void setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced);

  /**
   * Add residual contributions from Constraints
   *
   * @param residual - reference to the residual vector where constraint contributions will be
   * computed
   * @param displaced Controls whether to do the displaced Constraints or non-displaced
   */
  void constraintResiduals(NumericVector<Number> & residual, bool displaced);

  /**
   * Computes residual for a given tag
   * @param residual Residual is formed in here
   * @param the tag of kernels for which the residual is to be computed.
   */
  void computeResidualTag(NumericVector<Number> & residual, TagID tag_id);

  /**
   * Form multiple tag-associated residual vectors for all the given tags
   */
  void computeResidualTags(const std::set<TagID> & tags);

  /**
   * Form a residual vector for a given tag
   */
  void computeResidual(NumericVector<Number> & residual, TagID tag_id);

  /**
   * Finds the implicit sparsity graph between geometrically related dofs.
   */
  void
  findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data,
                                       std::map<dof_id_type, std::vector<dof_id_type>> & graph);

  /**
   * Adds entries to the Jacobian in the correct positions for couplings coming from dofs being
   * coupled that
   * are related geometrically (i.e. near each other across a gap).
   */
  void addImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data);

  /**
   * Add jacobian contributions from Constraints
   *
   * @param jacobian reference to the Jacobian matrix
   * @param displaced Controls whether to do the displaced Constraints or non-displaced
   */
  void constraintJacobians(bool displaced);

  /// set all the global dof indices for a nonlinear variable
  void setVariableGlobalDoFs(const std::string & var_name);
  const std::vector<dof_id_type> & getVariableGlobalDoFs() { return _var_all_dof_indices; }

  /**
   * Computes multiple (tag associated) Jacobian matricese
   */
  void computeJacobianTags(const std::set<TagID> & tags);

  /**
   * Associate jacobian to systemMatrixTag, and then form a matrix for all the tags
   */
  void computeJacobian(SparseMatrix<Number> & jacobian, const std::set<TagID> & tags);

  /**
   * Take all tags in the system, and form a matrix for all tags in the system
   */
  void computeJacobian(SparseMatrix<Number> & jacobian);

  /**
   * Computes several Jacobian blocks simultaneously, summing their contributions into smaller
   * preconditioning matrices.
   *
   * Used by Physics-based preconditioning
   *
   * @param blocks The blocks to fill in (JacobianBlock is defined in ComputeJacobianBlocksThread)
   */
  void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks);

  void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks, const std::set<TagID> & tags);

  /**
   * Compute damping
   * @param solution The trail solution vector
   * @param update The incremental update to the solution vector
   * @return returns The damping factor
   */
  Real computeDamping(const NumericVector<Number> & solution, const NumericVector<Number> & update);

  /**
   * Computes the time derivative vector
   */
  void computeTimeDerivatives();

  /**
   * Called at the beginning of the time step
   */
  void onTimestepBegin();

  /**
   * Called from assembling when we hit a new subdomain
   * @param subdomain ID of the new subdomain
   * @param tid Thread ID
   */
  virtual void subdomainSetup(SubdomainID subdomain, THREAD_ID tid);

  virtual void setSolution(const NumericVector<Number> & soln);

  /**
   * Update active objects of Warehouses owned by NonlinearSystemBase
   */
  void updateActive(THREAD_ID tid);

  /**
   * Set transient term used by residual and Jacobian evaluation.
   * @param udot transient term
   * @note If the calling sequence for residual evaluation was changed, this could become an
   * explicit argument.
   */
  virtual void setSolutionUDot(const NumericVector<Number> & udot);

  virtual NumericVector<Number> & solutionUDot() override;

  /**
   *  Return a numeric vector that is associated with the time tag.
   */
  NumericVector<Number> & getResidualTimeVector();

  /**
   * Return a numeric vector that is associated with the nontime tag.
   */
  NumericVector<Number> & getResidualNonTimeVector();

  /**
   * Return a residual vector that is associated with the residual tag.
   */
  NumericVector<Number> & residualVector(TagID tag);

  virtual const NumericVector<Number> *& currentSolution() override { return _current_solution; }

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution() override;

  virtual NumericVector<Number> & residualCopy() override;
  virtual NumericVector<Number> & residualGhosted() override;

  virtual NumericVector<Number> & RHS() = 0;

  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<dof_id_type> & n_nz,
                               std::vector<dof_id_type> & n_oz) override;

  /**
   * Sets a preconditioner
   * @param pc The preconditioner to be set
   */
  void setPreconditioner(std::shared_ptr<MoosePreconditioner> pc);

  /**
   * If called with true this system will use a finite differenced form of
   * the Jacobian as the preconditioner
   */
  void useFiniteDifferencedPreconditioner(bool use = true)
  {
    _use_finite_differenced_preconditioner = use;
  }

  /**
   * If called with a single string, it is used as the name of a the top-level decomposition split.
   * If the array is empty, no decomposition is used.
   * In all other cases an error occurs.
   */
  void setDecomposition(const std::vector<std::string> & decomposition);

  /**
   * If called with true this system will use a field split preconditioner matrix.
   */
  void useFieldSplitPreconditioner(bool use = true) { _use_field_split_preconditioner = use; }

  /**
   * If called with true this will add entries into the jacobian to link together degrees of freedom
   * that are found to
   * be related through the geometric search system.
   *
   * These entries are really only used by the Finite Difference Preconditioner and the constraint
   * system right now.
   */
  void addImplicitGeometricCouplingEntriesToJacobian(bool add = true)
  {
    _add_implicit_geometric_coupling_entries_to_jacobian = add;
  }

  /**
   * Indicates whether to assemble residual and Jacobian after each constraint application.
   * When true, enables "transitive" constraint application: subsequent constraints can use prior
   * constraints' results.
   */
  void assembleConstraintsSeparately(bool separately = true)
  {
    _assemble_constraints_separately = separately;
  }

  /**
   * Setup damping stuff (called before we actually start)
   */
  void setupDampers();
  /**
   * Compute the incremental change in variables at QPs for dampers. Called before we use damping
   * @param tid Thread ID
   * @param damped_vars Set of variables for which increment is to be computed
   */
  void reinitIncrementAtQpsForDampers(THREAD_ID tid, const std::set<MooseVariable *> & damped_vars);

  /**
   * Compute the incremental change in variables at nodes for dampers. Called before we use damping
   * @param tid Thread ID
   * @param damped_vars Set of variables for which increment is to be computed
   */
  void reinitIncrementAtNodeForDampers(THREAD_ID tid,
                                       const std::set<MooseVariable *> & damped_vars);

  ///@{
  /// System Integrity Checks
  void checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains) const;
  bool containsTimeKernel();
  ///@}

  /**
   * Return the number of non-linear iterations
   */
  unsigned int nNonlinearIterations() const { return _n_iters; }

  /**
   * Return the number of linear iterations
   */
  unsigned int nLinearIterations() const { return _n_linear_iters; }

  /**
   * Return the total number of residual evaluations done so far in this calculation
   */
  unsigned int nResidualEvaluations() const { return _n_residual_evaluations; }

  /**
   * Return the final nonlinear residual
   */
  Real finalNonlinearResidual() const { return _final_residual; }

  /**
   * Return the last nonlinear norm
   * @return A Real containing the last computed residual norm
   */
  Real nonlinearNorm() const { return _last_nl_rnorm; }

  /**
   * Force the printing of all variable norms after each solve.
   * \todo{Remove after output update
   */
  void printAllVariableNorms(bool state) { _print_all_var_norms = state; }

  void debuggingResiduals(bool state) { _debugging_residuals = state; }

  unsigned int _num_residual_evaluations;

  void setPredictor(std::shared_ptr<Predictor> predictor);
  Predictor * getPredictor() { return _predictor.get(); }

  TimeIntegrator * getTimeIntegrator() { return _time_integrator.get(); }

  void setPCSide(MooseEnum pcs);

  Moose::PCSideType getPCSide() { return _pc_side; }

  void setMooseKSPNormType(MooseEnum kspnorm);

  Moose::MooseKSPNormType getMooseKSPNormType() { return _ksp_norm; }

  /**
   * Indicated whether this system needs material properties on boundaries.
   * @return Boolean if IntegratedBCs are active
   */
  bool needBoundaryMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const;

  /**
   * Indicated whether this system needs material properties on an interface.
   * @return Boolean if InterafceKernels are active
   */
  bool needBoundaryMaterialOnInterface(BoundaryID bnd_id, THREAD_ID tid) const;

  /**
   * Indicates whether this system needs material properties on internal sides.
   * @return Boolean if DGKernels are active
   */
  bool needSubdomainMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid) const;

  /**
   * Getter for _doing_dg
   */
  bool doingDG() const;

  //@{
  /**
   * Access functions to Warehouses from outside NonlinearSystemBase
   */
  MooseObjectTagWarehouse<KernelBase> & getKernelWarehouse() { return _kernels; }
  const MooseObjectWarehouse<DGKernel> & getDGKernelWarehouse() { return _dg_kernels; }
  const MooseObjectWarehouse<InterfaceKernel> & getInterfaceKernelWarehouse()
  {
    return _interface_kernels;
  }
  const MooseObjectWarehouse<DiracKernel> & getDiracKernelWarehouse() const
  {
    return _dirac_kernels;
  }
  const MooseObjectWarehouse<IntegratedBCBase> & getIntegratedBCWarehouse() const
  {
    return _integrated_bcs;
  }
  const MooseObjectWarehouse<ElementDamper> & getElementDamperWarehouse() const
  {
    return _element_dampers;
  }
  const MooseObjectWarehouse<NodalDamper> & getNodalDamperWarehouse() const
  {
    return _nodal_dampers;
  }
  const ConstraintWarehouse & getConstraintWarehouse() const { return _constraints; };
  //@}

  /**
   * Weather or not the nonlinear system has save-ins
   */
  bool hasSaveIn() const { return _has_save_in || _has_nodalbc_save_in; }

  /**
   * Weather or not the nonlinear system has diagonal Jacobian save-ins
   */
  bool hasDiagSaveIn() const { return _has_diag_save_in || _has_nodalbc_diag_save_in; }

  virtual NumericVector<Number> & solution() override { return *_sys.solution; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  virtual NumericVector<Number> * solutionPreviousNewton() override
  {
    return _solution_previous_nl;
  }

  virtual void setPreviousNewtonSolution(const NumericVector<Number> & soln);

  virtual TagID timeVectorTag() override { return _Re_time_tag; }

  virtual TagID nonTimeVectorTag() override { return _Re_non_time_tag; }

  virtual TagID residualVectorTag() override { return _Re_tag; }

  virtual TagID systemMatrixTag() override { return _Ke_system_tag; }

public:
  FEProblemBase & _fe_problem;
  System & _sys;
  // FIXME: make these protected and create getters/setters
  Real _last_rnorm;
  Real _last_nl_rnorm;
  Real _l_abs_step_tol;
  Real _initial_residual_before_preset_bcs;
  Real _initial_residual_after_preset_bcs;
  std::vector<unsigned int> _current_l_its;
  unsigned int _current_nl_its;
  bool _compute_initial_residual_before_preset_bcs;

protected:
  /**
   * Compute the residual for a given tag
   * @param tags The tags of kernels for which the residual is to be computed.
   */
  void computeResidualInternal(const std::set<TagID> & tags);

  /**
   * Enforces nodal boundary conditions. The boundary condition will be implemented
   * in the residual using all the tags in the system.
   */
  void computeNodalBCs(NumericVector<Number> & residual);

  /**
   * Form a residual for BCs that at least has one of the given tags.
   */
  void computeNodalBCs(NumericVector<Number> & residual, const std::set<TagID> & tags);

  /**
   * Form multiple tag-associated residual vectors for the given tags.
   */
  void computeNodalBCs(const std::set<TagID> & tags);

  /**
   * Form multiple matrices for all the tags. Users should not call this func directly.
   */
  void computeJacobianInternal(const std::set<TagID> & tags);

  void computeDiracContributions(bool is_jacobian);

  void computeScalarKernelsJacobians();

  /**
   * Enforce nodal constraints
   */
  void enforceNodalConstraintsResidual(NumericVector<Number> & residual);
  void enforceNodalConstraintsJacobian();

  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;
  /// ghosted form of the residual
  NumericVector<Number> * _residual_ghosted;

  /// Serialized version of the solution vector
  NumericVector<Number> & _serialized_solution;

  /// Solution vector of the previous nonlinear iterate
  NumericVector<Number> * _solution_previous_nl;

  /// Copy of the residual vector
  NumericVector<Number> & _residual_copy;

  /// Time integrator
  std::shared_ptr<TimeIntegrator> _time_integrator;

  /// solution vector for u^dot
  NumericVector<Number> * _u_dot;
  /// \f$ {du^dot}\over{du} \f$
  Number _du_dot_du;

  /// Tag for time contribution residual
  TagID _Re_time_tag;

  /// Vector tags to temporarily store all tags associated with the current system.
  std::set<TagID> _nl_vector_tags;

  /// Matrix tags to temporarily store all tags associated with the current system.
  std::set<TagID> _nl_matrix_tags;

  /// residual vector for time contributions
  NumericVector<Number> * _Re_time;

  /// Tag for non-time contribution residual
  TagID _Re_non_time_tag;
  /// residual vector for non-time contributions
  NumericVector<Number> * _Re_non_time;

  /// Used for the residual vector from PETSc
  TagID _Re_tag;

  /// Tag for non-time contribution Jacobian
  TagID _Ke_non_time_tag;

  /// Tag for system contribution Jacobian
  TagID _Ke_system_tag;

  ///@{
  /// Kernel Storage
  MooseObjectTagWarehouse<KernelBase> _kernels;
  MooseObjectWarehouse<ScalarKernel> _scalar_kernels;
  MooseObjectWarehouse<ScalarKernel> _time_scalar_kernels;
  MooseObjectWarehouse<ScalarKernel> _non_time_scalar_kernels;
  MooseObjectWarehouse<DGKernel> _dg_kernels;
  MooseObjectWarehouse<InterfaceKernel> _interface_kernels;

  ///@}

  ///@{
  /// BoundaryCondition Warhouses
  MooseObjectWarehouse<IntegratedBCBase> _integrated_bcs;
  MooseObjectTagWarehouse<NodalBCBase> _nodal_bcs;
  MooseObjectWarehouse<PresetNodalBC> _preset_nodal_bcs;
  ///@}

  /// Dirac Kernel storage for each thread
  MooseObjectWarehouse<DiracKernel> _dirac_kernels;

  /// Element Dampers for each thread
  MooseObjectWarehouse<ElementDamper> _element_dampers;

  /// Nodal Dampers for each thread
  MooseObjectWarehouse<NodalDamper> _nodal_dampers;

  /// General Dampers
  MooseObjectWarehouse<GeneralDamper> _general_dampers;

  /// NodalKernels for each thread
  MooseObjectWarehouse<NodalKernel> _nodal_kernels;

  /// Decomposition splits
  MooseObjectWarehouseBase<Split> _splits; // use base b/c there are no setup methods

  /// Constraints storage object
  ConstraintWarehouse _constraints;

protected:
  /// increment vector
  NumericVector<Number> * _increment_vec;
  /// Preconditioner
  std::shared_ptr<MoosePreconditioner> _preconditioner;
  /// Preconditioning side
  Moose::PCSideType _pc_side;
  /// KSP norm type
  Moose::MooseKSPNormType _ksp_norm;

  /// Whether or not to use a finite differenced preconditioner
  bool _use_finite_differenced_preconditioner;
#ifdef LIBMESH_HAVE_PETSC
  MatFDColoring _fdcoloring;
#endif
  /// Whether or not the system can be decomposed into splits
  bool _have_decomposition;
  /// Name of the top-level split of the decomposition
  std::string _decomposition_split;
  /// Whether or not to use a FieldSplitPreconditioner matrix based on the decomposition
  bool _use_field_split_preconditioner;

  /// Whether or not to add implicit geometric couplings to the Jacobian for FDP
  bool _add_implicit_geometric_coupling_entries_to_jacobian;

  /// Whether or not to assemble the residual and Jacobian after the application of each constraint.
  bool _assemble_constraints_separately;

  /// Whether or not a copy of the residual needs to be made
  bool _need_serialized_solution;

  /// Whether or not a copy of the residual needs to be made
  bool _need_residual_copy;
  /// Whether or not a ghosted copy of the residual needs to be made
  bool _need_residual_ghosted;
  /// true if debugging residuals
  bool _debugging_residuals;

  /// true if DG is active (optimization reasons)
  bool _doing_dg;

  /// vectors that will be zeroed before a residual computation
  std::vector<std::string> _vecs_to_zero_for_residual;

  unsigned int _n_iters;
  unsigned int _n_linear_iters;

  /// Total number of residual evaluations that have been performed
  unsigned int _n_residual_evaluations;

  Real _final_residual;

  /// If predictor is active, this is non-NULL
  std::shared_ptr<Predictor> _predictor;

  bool _computing_initial_residual;

  bool _print_all_var_norms;

  /// If there is any Kernel or IntegratedBC having save_in
  bool _has_save_in;

  /// If there is any Kernel or IntegratedBC having diag_save_in
  bool _has_diag_save_in;

  /// If there is a nodal BC having save_in
  bool _has_nodalbc_save_in;

  /// If there is a nodal BC having diag_save_in
  bool _has_nodalbc_diag_save_in;

  void getNodeDofs(dof_id_type node_id, std::vector<dof_id_type> & dofs);

  std::vector<dof_id_type> _var_all_dof_indices;

  /// Timers
  PerfID _compute_residual_tags_timer;
  PerfID _compute_residual_internal_timer;
  PerfID _kernels_timer;
  PerfID _scalar_kernels_timer;
  PerfID _nodal_kernels_timer;
  PerfID _nodal_kernel_bcs_timer;
  PerfID _nodal_bcs_timer;
  PerfID _compute_jacobian_tags_timer;
  PerfID _compute_jacobian_blocks_timer;
  PerfID _compute_dampers_timer;
  PerfID _compute_dirac_timer;
};

#endif /* NONLINEARSYSTEMBASE_H */
