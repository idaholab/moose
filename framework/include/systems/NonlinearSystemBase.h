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
#include "ConstraintWarehouse.h"
#include "MooseObjectWarehouse.h"
#include "MooseObjectTagWarehouse.h"
#include "PerfGraphInterface.h"
#include "ComputeMortarFunctor.h"
#include "MooseHashing.h"

#include "libmesh/transient_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/linear_solver.h"

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
class DirichletBCBase;
class ADDirichletBCBase;
class DGKernelBase;
class InterfaceKernelBase;
class ScalarKernelBase;
class DiracKernelBase;
class NodalKernelBase;
class Split;
class KernelBase;
class BoundaryCondition;
class ResidualObject;

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
 * Nonlinear system to be solved
 *
 * It is a part of FEProblemBase ;-)
 */
class NonlinearSystemBase : public SystemBase, public PerfGraphInterface
{
public:
  NonlinearSystemBase(FEProblemBase & problem, System & sys, const std::string & name);
  virtual ~NonlinearSystemBase();

  virtual void init() override;

  bool computedScalingJacobian() const { return _computed_scaling; }

  /**
   * Turn off the Jacobian (must be called before equation system initialization)
   */
  virtual void turnOffJacobian();

  virtual void solve() override = 0;
  virtual void restoreSolutions() override;

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() = 0;

  virtual NonlinearSolver<Number> * nonlinearSolver() = 0;

  virtual SNES getSNES() = 0;

  virtual unsigned int getCurrentNonlinearIterationNumber() = 0;

  /**
   * Returns true if this system is currently computing the initial residual for a solve.
   * @return Whether or not we are currently computing the initial residual.
   */
  virtual bool computingInitialResidual() { return _computing_initial_residual; }

  // Setup Functions ////
  virtual void initialSetup() override;
  virtual void timestepSetup() override;
  virtual void customSetup(const ExecFlagType & exec_type) override;
  virtual void residualSetup() override;
  virtual void jacobianSetup() override;

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
  void addTimeIntegrator(const std::string & type,
                         const std::string & name,
                         InputParameters & parameters) override;
  using SystemBase::addTimeIntegrator;

  /**
   * Add u_dot, u_dotdot, u_dot_old and u_dotdot_old
   * vectors if requested by the time integrator
   */
  void addDotVectors();

  /**
   * Adds a kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void addKernel(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters);

  /**
   * Adds a NodalKernel
   * @param kernel_name The type of the nodal kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void addNodalKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters);

  /**
   * Adds a scalar kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addScalarKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters & parameters);

  /**
   * Adds a boundary condition
   * @param bc_name The type of the boundary condition
   * @param name The name of the boundary condition
   * @param parameters Boundary condition parameters
   */
  void addBoundaryCondition(const std::string & bc_name,
                            const std::string & name,
                            InputParameters & parameters);

  /**
   * Adds a Constraint
   * @param c_name The type of the constraint
   * @param name The name of the constraint
   * @param parameters Constraint parameters
   */
  void
  addConstraint(const std::string & c_name, const std::string & name, InputParameters & parameters);

  /**
   * Adds a Dirac kernel
   * @param kernel_name The type of the dirac kernel
   * @param name The name of the Dirac kernel
   * @param parameters Dirac kernel parameters
   */
  void addDiracKernel(const std::string & kernel_name,
                      const std::string & name,
                      InputParameters & parameters);

  /**
   * Adds a DG kernel
   * @param dg_kernel_name The type of the DG kernel
   * @param name The name of the DG kernel
   * @param parameters DG kernel parameters
   */
  void
  addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters & parameters);

  /**
   * Adds an interface kernel
   * @param interface_kernel_name The type of the interface kernel
   * @param name The name of the interface kernel
   * @param parameters interface kernel parameters
   */
  void addInterfaceKernel(std::string interface_kernel_name,
                          const std::string & name,
                          InputParameters & parameters);

  /**
   * Adds a damper
   * @param damper_name The type of the damper
   * @param name The name of the damper
   * @param parameters Damper parameters
   */
  void addDamper(const std::string & damper_name,
                 const std::string & name,
                 InputParameters & parameters);

  /**
   * Adds a split
   * @param split_name The type of the split
   * @param name The name of the split
   * @param parameters Split parameters
   */
  void
  addSplit(const std::string & split_name, const std::string & name, InputParameters & parameters);

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
  void setConstraintSecondaryValues(NumericVector<Number> & solution, bool displaced);

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
   * Form possibly multiple tag-associated vectors and matrices
   */
  void computeResidualAndJacobianTags(const std::set<TagID> & vector_tags,
                                      const std::set<TagID> & matrix_tags);

  /**
   * Compute residual and Jacobian from contributions not related to constraints, such as nodal
   * boundary conditions
   */
  void computeResidualAndJacobianInternal(const std::set<TagID> & vector_tags,
                                          const std::set<TagID> & matrix_tags);

  /**
   * Form a residual vector for a given tag
   */
  void computeResidual(NumericVector<Number> & residual, TagID tag_id);

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
   * Method used to obtain scaling factors for variables
   */
  void computeScaling();

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
  void computeTimeDerivatives(bool jacobian_calculation = false);

  /**
   * Called at the beginning of the time step
   */
  void onTimestepBegin();

  using SystemBase::subdomainSetup;
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

  /**
   * Set transient term used by residual and Jacobian evaluation.
   * @param udotdot transient term
   * @note If the calling sequence for residual evaluation was changed, this could become an
   * explicit argument.
   */
  virtual void setSolutionUDotDot(const NumericVector<Number> & udotdot);

  NumericVector<Number> * solutionUDot() override { return _u_dot; }
  NumericVector<Number> * solutionUDotDot() override { return _u_dotdot; }
  NumericVector<Number> * solutionUDotOld() override { return _u_dot_old; }
  NumericVector<Number> * solutionUDotDotOld() override { return _u_dotdot_old; }
  const NumericVector<Number> * solutionUDot() const override { return _u_dot; }
  const NumericVector<Number> * solutionUDotDot() const override { return _u_dotdot; }
  const NumericVector<Number> * solutionUDotOld() const override { return _u_dot_old; }
  const NumericVector<Number> * solutionUDotDotOld() const override { return _u_dotdot_old; }

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

  const NumericVector<Number> * const & currentSolution() const override
  {
    return _current_solution;
  }

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
  MoosePreconditioner const * getPreconditioner() const;

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
   * Attach a customized preconditioner that requires physics knowledge.
   * Generic preconditioners should be implemented in PETSc, instead.
   */
  virtual void attachPreconditioner(Preconditioner<Number> * preconditioner) = 0;

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
   * Indicated whether this system needs material properties on interfaces.
   * @return Boolean if IntegratedBCs are active
   */
  bool needInterfaceMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const;

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
  MooseObjectTagWarehouse<DGKernelBase> & getDGKernelWarehouse() { return _dg_kernels; }
  MooseObjectTagWarehouse<InterfaceKernelBase> & getInterfaceKernelWarehouse()
  {
    return _interface_kernels;
  }
  MooseObjectTagWarehouse<DiracKernelBase> & getDiracKernelWarehouse() { return _dirac_kernels; }
  MooseObjectTagWarehouse<IntegratedBCBase> & getIntegratedBCWarehouse() { return _integrated_bcs; }
  const MooseObjectWarehouse<ElementDamper> & getElementDamperWarehouse() const
  {
    return _element_dampers;
  }
  const MooseObjectWarehouse<NodalDamper> & getNodalDamperWarehouse() const
  {
    return _nodal_dampers;
  }
  const ConstraintWarehouse & getConstraintWarehouse() const { return _constraints; }

  /**
   * Return the NodalBCBase warehouse
   */
  const MooseObjectTagWarehouse<NodalBCBase> & getNodalBCWarehouse() const { return _nodal_bcs; }

  /**
   * Return the IntegratedBCBase warehouse
   */
  const MooseObjectTagWarehouse<IntegratedBCBase> & getIntegratedBCWarehouse() const
  {
    return _integrated_bcs;
  }

  //@}

  /**
   * Weather or not the nonlinear system has save-ins
   */
  bool hasSaveIn() const { return _has_save_in || _has_nodalbc_save_in; }

  /**
   * Weather or not the nonlinear system has diagonal Jacobian save-ins
   */
  bool hasDiagSaveIn() const { return _has_diag_save_in || _has_nodalbc_diag_save_in; }

  virtual System & system() override { return _sys; }
  virtual const System & system() const override { return _sys; }

  virtual void setSolutionUDotOld(const NumericVector<Number> & u_dot_old);

  virtual void setSolutionUDotDotOld(const NumericVector<Number> & u_dotdot_old);

  virtual void setPreviousNewtonSolution(const NumericVector<Number> & soln);

  TagID timeVectorTag() const override { return _Re_time_tag; }
  TagID nonTimeVectorTag() const override { return _Re_non_time_tag; }
  TagID residualVectorTag() const override { return _Re_tag; }
  TagID systemMatrixTag() const override { return _Ke_system_tag; }

  /**
   * Call this method if you want the residual and Jacobian to be computed simultaneously
   */
  virtual void residualAndJacobianTogether() = 0;

  bool computeScalingOnce() const { return _compute_scaling_once; }
  void computeScalingOnce(bool compute_scaling_once)
  {
    _compute_scaling_once = compute_scaling_once;
  }

  /**
   * Sets the param that indicates the weighting of the residual vs the Jacobian in determining
   * variable scaling parameters. A value of 1 indicates pure residual-based scaling. A value of 0
   * indicates pure Jacobian-based scaling
   */
  void autoScalingParam(Real resid_vs_jac_scaling_param)
  {
    _resid_vs_jac_scaling_param = resid_vs_jac_scaling_param;
  }

  void scalingGroupVariables(const std::vector<std::vector<std::string>> & scaling_group_variables)
  {
    _scaling_group_variables = scaling_group_variables;
  }

  void
  ignoreVariablesForAutoscaling(const std::vector<std::string> & ignore_variables_for_autoscaling)
  {
    _ignore_variables_for_autoscaling = ignore_variables_for_autoscaling;
  }

  bool offDiagonalsInAutoScaling() const { return _off_diagonals_in_auto_scaling; }
  void offDiagonalsInAutoScaling(bool off_diagonals_in_auto_scaling)
  {
    _off_diagonals_in_auto_scaling = off_diagonals_in_auto_scaling;
  }

  FEProblemBase & _fe_problem;
  System & _sys;
  // FIXME: make these protected and create getters/setters
  Real _last_nl_rnorm;
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
   * compute the residual and Jacobian for nodal boundary conditions
   */
  void computeNodalBCsResidualAndJacobian();

  /**
   * Form multiple matrices for all the tags. Users should not call this func directly.
   */
  void computeJacobianInternal(const std::set<TagID> & tags);

  void computeDiracContributions(const std::set<TagID> & tags, bool is_jacobian);

  void computeScalarKernelsJacobians(const std::set<TagID> & tags);

  /**
   * Enforce nodal constraints
   */
  void enforceNodalConstraintsResidual(NumericVector<Number> & residual);
  void enforceNodalConstraintsJacobian();

  /**
   * Do mortar constraint residual/jacobian computations
   */
  void mortarConstraints(Moose::ComputeType compute_type);

  /**
   * Compute a "Jacobian" for automatic scaling purposes
   */
  virtual void computeScalingJacobian() = 0;

  /**
   * Compute a "residual" for automatic scaling purposes
   */
  virtual void computeScalingResidual() = 0;

  /**
   * Assemble the numeric vector of scaling factors such that it can be used during assembly of the
   * system matrix
   */
  void assembleScalingVector();

  /**
   * Called after any ResidualObject-derived objects are added
   * to the system.
   */
  virtual void postAddResidualObject(ResidualObject &) {}

  NumericVector<Number> & solutionInternal() const override { return *_sys.solution; }

  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;
  /// ghosted form of the residual
  NumericVector<Number> * _residual_ghosted;

  /// Serialized version of the solution vector, or nullptr if a
  /// serialized solution is not needed
  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Copy of the residual vector, or nullptr if a copy is not needed
  std::unique_ptr<NumericVector<Number>> _residual_copy;

  /// solution vector for u^dot
  NumericVector<Number> * _u_dot;
  /// solution vector for u^dotdot
  NumericVector<Number> * _u_dotdot;

  /// old solution vector for u^dot
  NumericVector<Number> * _u_dot_old;
  /// old solution vector for u^dotdot
  NumericVector<Number> * _u_dotdot_old;

  /// \f$ {du^dot}\over{du} \f$
  Number _du_dot_du;
  /// \f$ {du^dotdot}\over{du} \f$
  Number _du_dotdot_du;

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
  MooseObjectTagWarehouse<ScalarKernelBase> _scalar_kernels;
  MooseObjectTagWarehouse<DGKernelBase> _dg_kernels;
  MooseObjectTagWarehouse<InterfaceKernelBase> _interface_kernels;

  ///@}

  ///@{
  /// BoundaryCondition Warhouses
  MooseObjectTagWarehouse<IntegratedBCBase> _integrated_bcs;
  MooseObjectTagWarehouse<NodalBCBase> _nodal_bcs;
  MooseObjectWarehouse<DirichletBCBase> _preset_nodal_bcs;
  MooseObjectWarehouse<ADDirichletBCBase> _ad_preset_nodal_bcs;
  ///@}

  /// Dirac Kernel storage for each thread
  MooseObjectTagWarehouse<DiracKernelBase> _dirac_kernels;

  /// Element Dampers for each thread
  MooseObjectWarehouse<ElementDamper> _element_dampers;

  /// Nodal Dampers for each thread
  MooseObjectWarehouse<NodalDamper> _nodal_dampers;

  /// General Dampers
  MooseObjectWarehouse<GeneralDamper> _general_dampers;

  /// NodalKernels for each thread
  MooseObjectTagWarehouse<NodalKernelBase> _nodal_kernels;

  /// Decomposition splits
  MooseObjectWarehouseBase<Split> _splits; // use base b/c there are no setup methods

  /// Constraints storage object
  ConstraintWarehouse _constraints;

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

  MatFDColoring _fdcoloring;

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

  /// Flag used to indicate whether we have already computed the scaling Jacobian
  bool _computed_scaling;

  /// Whether the scaling factors should only be computed once at the beginning of the simulation
  /// through an extra Jacobian evaluation. If this is set to false, then the scaling factors will
  /// be computed during an extra Jacobian evaluation at the beginning of every time step.
  bool _compute_scaling_once;

  /// The param that indicates the weighting of the residual vs the Jacobian in determining
  /// variable scaling parameters. A value of 1 indicates pure residual-based scaling. A value of 0
  /// indicates pure Jacobian-based scaling
  Real _resid_vs_jac_scaling_param;

  /// A container of variable groupings that can be used in scaling calculations. This can be useful
  /// for simulations in which vector-like variables are split into invidual scalar-field components
  /// like for solid/fluid mechanics
  std::vector<std::vector<std::string>> _scaling_group_variables;

  /// Container to hold flag if variable is to participate in autoscaling
  std::vector<bool> _variable_autoscaled;

  /// A container for variables that do not partipate in autoscaling
  std::vector<std::string> _ignore_variables_for_autoscaling;

  /// Whether to include off diagonals when determining automatic scaling factors
  bool _off_diagonals_in_auto_scaling;

  /// A diagonal matrix used for computing scaling
  std::unique_ptr<DiagonalMatrix<Number>> _scaling_matrix;

private:
  /**
   * Finds the implicit sparsity graph between geometrically related dofs.
   */
  void findImplicitGeometricCouplingEntries(
      GeometricSearchData & geom_search_data,
      std::unordered_map<dof_id_type, std::vector<dof_id_type>> & graph);

  /**
   * Setup group scaling containers
   */
  void setupScalingData();

  /// Functors for computing undisplaced mortar constraints
  std::unordered_map<std::pair<BoundaryID, BoundaryID>, ComputeMortarFunctor>
      _undisplaced_mortar_functors;

  /// Functors for computing displaced mortar constraints
  std::unordered_map<std::pair<BoundaryID, BoundaryID>, ComputeMortarFunctor>
      _displaced_mortar_functors;

  /// The current states of the solution (0 = current, 1 = old, etc)
  std::vector<NumericVector<Number> *> _solution_state;

  /// Whether we've initialized the automatic scaling data structures
  bool _auto_scaling_initd;

  /// A map from variable index to group variable index and it's associated (inverse) scaling factor
  std::unordered_map<unsigned int, unsigned int> _var_to_group_var;

  /// The number of scaling groups
  std::size_t _num_scaling_groups;
};
