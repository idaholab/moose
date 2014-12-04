/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef NONLINEARSYSTEM_H
#define NONLINEARSYSTEM_H

#include "SystemBase.h"
#include "KernelWarehouse.h"
#include "BCWarehouse.h"
#include "DiracKernelWarehouse.h"
#include "DGKernelWarehouse.h"
#include "DamperWarehouse.h"
#include "ConstraintWarehouse.h"
#include "SplitWarehouse.h"
#include "TimeIntegrator.h"
#include "Predictor.h"

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/coupling_matrix.h"
#include "libmesh/libmesh_common.h"
#include LIBMESH_INCLUDE_UNORDERED_MAP

class FEProblem;
class MoosePreconditioner;
class JacobianBlock;

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblem ;-)
 */
class NonlinearSystem : public SystemTempl<TransientNonlinearImplicitSystem>
{
public:
  NonlinearSystem(FEProblem & problem, const std::string & name);
  virtual ~NonlinearSystem();

  virtual void init();
  virtual void solve();
  virtual void restoreSolutions();

  /**
   * Returns true if this system is currently computing the initial residual for a solve.
   * @return Whether or not we are currently computing the initial residual.
   */
  virtual bool computingInitialResidual() { return _computing_initial_residual; }

  // Setup Functions ////
  virtual void initialSetup();
  virtual void initialSetupBCs();
  virtual void initialSetupKernels();
  virtual void timestepSetup();

  void setupFiniteDifferencedPreconditioner();
  void setupDecomposition();
  void setupSplitBasedPreconditioner();

  bool haveFiniteDifferencedPreconditioner() {return _use_finite_differenced_preconditioner;}
  bool haveSplitBasedPreconditioner()        {return _use_split_based_preconditioner;}
  bool haveDecomposition()                   {return _have_decomposition;}

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  virtual bool converged();

  /**
   * Add a time integrator
   * @param type Type of the integrator
   * @param name The name of the integrator
   * @param parameters Integrator params
   */
  void addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters);

  /**
   * Adds a kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a scalar kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a boundary condition
   * @param bc_name The type of the boundary condition
   * @param name The name of the boundary condition
   * @param parameters Boundary condition parameters
   */
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a Constraint
   * @param c_name The type of the constraint
   * @param name The name of the constraint
   * @param parameters Constraint parameters
   */
  void addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a Dirac kernel
   * @param kernel_name The type of the dirac kernel
   * @param name The name of the Dirac kernel
   * @param parameters Dirac kernel parameters
   */
  void addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a DG kernel
   * @param dg_kernel_name The type of the DG kernel
   * @param name The name of the DG kernel
   * @param parameters DG kernel parameters
   */
  void addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a damper
   * @param damper_name The type of the damper
   * @param name The name of the damper
   * @param parameters Damper parameters
   */
  void addDamper(const std::string & damper_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a split
   * @param split_name The type of the split
   * @param name The name of the split
   * @param parameters Split parameters
   */
  void addSplit(const std::string & split_name, const std::string & name, InputParameters parameters);

  /**
   * Retrieves a split by name
   * @param name The name of the split
   */
  Split* getSplit(const std::string & name);


  /**
   * Adds a solution length vector to the system.
   *
   * @param vector_name The name of the vector.
   * @param project Whether or not to project this vector when doing mesh refinement.
   *                If the vector is just going to be recomputed then there is no need to project it.
   * @param type What type of parallel vector.  This is usually either PARALLEL or GHOSTED.
   *                                            GHOSTED is needed if you are going to be accessing off-processor entries.
   *                                            The ghosting pattern is the same as the solution vector.
   * @param zero_for_residual Whether or not to zero this vector at the beginning of computeResidual.  Useful when
   *                          you are going to accumulate something into this vector during computeResidual
   */
  NumericVector<Number>& addVector(const std::string & vector_name, const bool project, const ParallelType type, bool zero_for_residual = false);

  void setInitialSolution();

  /**
   * Sets the value of constrained variables in the solution vector.
   */
  void setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced);

  /**
   * Add residual contributions from Constraints
   *
   * @param residual - reference to the residual vector where constraint contributions will be computed
   * @param displaced Controls whether to do the displaced Constraints or non-displaced
   */
  void constraintResiduals(NumericVector<Number> & residual, bool displaced);

  /**
   * Computes residual
   * @param residual Residual is formed in here
   * @param type The type of kernels for which the residual is to be computed.
   */
  void computeResidual(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_ALL);

  /**
   * Finds the implicit sparsity graph between geometrically related dofs.
   */
  void findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data, std::map<unsigned int, std::vector<unsigned int> > & graph);

  /**
   * Adds entries to the Jacobian in the correct positions for couplings coming from dofs being coupled that
   * are related geometrically (i.e. near each other across a gap).
   */
  void addImplicitGeometricCouplingEntries(SparseMatrix<Number> & jacobian, GeometricSearchData & geom_search_data);

  /**
   * Add jacobian contributions from Constraints
   *
   * @param jacobian reference to the Jacobian matrix
   * @param displaced Controls whether to do the displaced Constraints or non-displaced
   */
  void constraintJacobians(SparseMatrix<Number> & jacobian, bool displaced);

  /**
   * Computes Jacobian
   * @param jacobian Jacobian is formed in here
   */
  void computeJacobian(SparseMatrix<Number> &  jacobian);

  /**
   * Computes several Jacobian blocks simultaneously, summing their contributions into smaller preconditioning matrices.
   *
   * Used by Physics-based preconditioning
   *
   * @param blocks The blocks to fill in (JacobianBlock is defined in ComputeJacobianBlocksThread)
   */
  void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks);

  /**
   * Compute damping
   * @param update
   * @return returns The damping factor
   */
  Real computeDamping(const NumericVector<Number>& update);

  /**
   * Called at the beginning of the time step
   */
  void onTimestepBegin();

  /**
   * Called from assembling when we hit a new subdomain
   * @param subdomain ID of the new subdomain
   * @param tid Thread ID
   */
  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid);

  virtual void setSolution(const NumericVector<Number> & soln);

  /**
   * Set transient term used by residual and Jacobian evaluation.
   * @param udot transient term
   * @note If the calling sequence for residual evaluation was changed, this could become an explicit argument.
   */
  virtual void setSolutionUDot(const NumericVector<Number> & udot);

  virtual NumericVector<Number> & solutionUDot();
  virtual NumericVector<Number> & residualVector(Moose::KernelType type);

  virtual const NumericVector<Number> * & currentSolution() { return _current_solution; }

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution();

  virtual NumericVector<Number> & residualCopy();
  virtual NumericVector<Number> & residualGhosted();

  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<unsigned int> & n_nz,
                               std::vector<unsigned int> & n_oz);

  /**
   * Sets a preconditioner
   * @param pc The preconditioner to be set
   */
  void setPreconditioner(MooseSharedPointer<MoosePreconditioner> pc);

  /**
   * If called with true this system will use a finite differenced form of
   * the Jacobian as the preconditioner
   */
  void useFiniteDifferencedPreconditioner(bool use = true) { _use_finite_differenced_preconditioner = use; }

  /**
   * If called with a single string, it is used as the name of a the top-level decomposition split.
   * If the array is empty, no decomposition is used.
   * In all other cases an error occurs.
   */
  void setDecomposition(const std::vector<std::string>& decomposition);

  /**
   * If called with true this system will use a split-based preconditioner matrix.
   */
  void useSplitBasedPreconditioner(bool use = true) { _use_split_based_preconditioner = use; }

  /**
   * If called with true this will add entries into the jacobian to link together degrees of freedom that are found to
   * be related through the geometric search system.
   *
   * These entries are really only used by the Finite Difference Preconditioner and the constraint system right now.
   */
  void addImplicitGeometricCouplingEntriesToJacobian(bool add=true) { _add_implicit_geometric_coupling_entries_to_jacobian = add; }

  /**
   * Indicates whether to assemble residual and Jacobian after each constraint application.
   * When true, enables "transitive" constraint application: subsequent constraints can use prior constraints' results.
   */
  void assembleConstraintsSeparately(bool separately=true) {_assemble_constraints_separately = separately;}

  /**
   * Setup damping stuff (called before we actually start)
   */
  void setupDampers();
  /**
   * Reinit dampers. Called before we use damping
   * @param tid Thread ID
   */
  void reinitDampers(THREAD_ID tid);

  ///@{
  /// System Integrity Checks
  void checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains, bool check_kernel_coverage) const;
  bool containsTimeKernel();
  ///@}

  /**
   * Return the number of non-linear iterations
   */
  unsigned int nNonlinearIterations() { return _n_iters; }

  /**
   * Returns the current nonlinear iteration number.  In libmesh, this is
   * updated during the nonlinear solve, so it should be up-to-date.
   */
  unsigned int getCurrentNonlinearIterationNumber() { return _sys.get_current_nonlinear_iteration_number(); }

  /**
   * Return the number of linear iterations
   */
  unsigned int nLinearIterations() { return _n_linear_iters; }

  /**
   * Return the total number of residual evaluations done so far in this calculation
   */
  unsigned int nResidualEvaluations() { return _n_residual_evaluations; }

  /**
   * Return the final nonlinear residual
   */
  Real finalNonlinearResidual() { return _final_residual; }

  /**
   * Return the last nonlinear norm
   * @return A Real containing the last computed residual norm
   */
  Real nonlinearNorm() { return _last_nl_rnorm; }

  /**
   * Force the printing of all variable norms after each solve.
   * \todo{Remove after output update
   */
  void printAllVariableNorms(bool state) { _print_all_var_norms = state; }

  void debuggingResiduals(bool state) { _debugging_residuals = state; }

  unsigned int _num_residual_evaluations;

  void setPredictor(MooseSharedPointer<Predictor> predictor);
  Predictor * getPredictor() { return _predictor.get(); }

  TimeIntegrator * getTimeIntegrator() { return _time_integrator.get(); }

  void setPCSide(MooseEnum pcs);

  Moose::PCSideType getPCSide() { return _pc_side; }

  /**
   * Indicated whether this system needs material properties on boundaries.
   * @return Boolean if IntegratedBCs are active
   */
  bool needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const;

  /**
   * Indicates whether this system needs material properties on internal sides.
   * @return Boolean if DGKernels are active
   */
  bool needMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid) const;

  /**
   * Getter for _doing_dg
   */
  bool doingDG() const;

  //@{
  /**
   * Updates the active kernels/dgkernels in the warehouse for the
   * passed in subdomain_id and thread
   */
  void updateActiveKernels(SubdomainID subdomain_id, THREAD_ID tid);
  void updateActiveDGKernels(Real t, Real dt, THREAD_ID tid);
  //@}

  //@{
  /**
   * Access functions to Warehouses from outside NonlinearSystem
   */
  const KernelWarehouse & getKernelWarehouse(THREAD_ID tid);
  const DGKernelWarehouse & getDGKernelWarehouse(THREAD_ID tid);
  const BCWarehouse & getBCWarehouse(THREAD_ID tid);
  const DiracKernelWarehouse & getDiracKernelWarehouse(THREAD_ID tid);
  const DamperWarehouse & getDamperWarehouse(THREAD_ID tid);
  //@}

public:
  FEProblem & _fe_problem;
  // FIXME: make these protected and create getters/setters
  Real _last_rnorm;
  Real _last_nl_rnorm;
  Real _l_abs_step_tol;
  Real _initial_residual;
  std::vector<unsigned int> _current_l_its;
  unsigned int _current_nl_its;

protected:
  /**
   * Computes the time derivative vector
   */
  void computeTimeDerivatives();

  /**
   * Compute the residual
   * @param type The type of kernels for which the residual is to be computed.
   */
  void computeResidualInternal(Moose::KernelType type = Moose::KT_ALL);

  /**
   * Enforces nodal boundary conditions
   * @param residual Residual where nodal BCs are enforced (input/output)
   */
  void computeNodalBCs(NumericVector<Number> & residual);

  void computeJacobianInternal(SparseMatrix<Number> &  jacobian);

  void computeDiracContributions(SparseMatrix<Number> * jacobian = NULL);

  void computeScalarKernelsJacobians(SparseMatrix<Number> & jacobian);

  /**
   * Enforce nodal constraints
   */
  void enforceNodalConstraintsResidual(NumericVector<Number> & residual);
  void enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian);


  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;
  /// ghosted form of the residual
  NumericVector<Number> & _residual_ghosted;

  /// Serialized version of the solution vector
  NumericVector<Number> & _serialized_solution;

  /// Copy of the residual vector
  NumericVector<Number> & _residual_copy;

  /// Time integrator
  MooseSharedPointer<TimeIntegrator> _time_integrator;
  /// solution vector for u^dot
  NumericVector<Number> & _u_dot;
  /// \f$ {du^dot}\over{du} \f$
  Number _du_dot_du;
  /// residual vector for time contributions
  NumericVector<Number> & _Re_time;
  /// residual vector for non-time contributions
  NumericVector<Number> & _Re_non_time;

  // holders
  /// Kernel storage for each thread
  std::vector<KernelWarehouse> _kernels;
  /// BC storage for each thread
  std::vector<BCWarehouse> _bcs;
  /// Dirac Kernel storage for each thread
  std::vector<DiracKernelWarehouse> _dirac_kernels;
  /// DG Kernel storage for each thread
  std::vector<DGKernelWarehouse> _dg_kernels;
  /// Dampers for each thread
  std::vector<DamperWarehouse> _dampers;

  /// Decomposition splits
  SplitWarehouse _splits;

public:
  /// Constraints for each thread
  std::vector<ConstraintWarehouse> _constraints;


protected:
  /// increment vector
  NumericVector<Number> * _increment_vec;
  /// Preconditioner
  MooseSharedPointer<MoosePreconditioner> _preconditioner;
  /// Preconditioning side
  Moose::PCSideType _pc_side;

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
  bool _use_split_based_preconditioner;

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

  /// NumericVectors that will be zeroed before a residual computation
  std::vector<NumericVector<Number> *> _vecs_to_zero_for_residual;

  unsigned int _n_iters;
  unsigned int _n_linear_iters;

  /// Total number of residual evaluations that have been performed
  unsigned int _n_residual_evaluations;

  Real _final_residual;

  /// If predictor is active, this is non-NULL
  MooseSharedPointer<Predictor> _predictor;

  bool _computing_initial_residual;

  bool _print_all_var_norms;

  void getNodeDofs(unsigned int node_id, std::vector<dof_id_type> & dofs);
};

#endif /* NONLINEARSYSTEM_H */
