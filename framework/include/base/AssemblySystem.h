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

#ifndef ASSEMBLYSYSTEM_H
#define ASSEMBLYSYSTEM_H

#include "SystemBase.h"
#include "KernelWarehouse.h"
#include "ConstraintWarehouse.h"
#include "MooseObjectWarehouse.h"

// Forward declarations
class FEProblem;
class MoosePreconditioner;
class JacobianBlock;
class TimeIntegrator;
class Predictor;
class ElementDamper;
class GeneralDamper;
class IntegratedBC;
class NodalBC;
class PresetNodalBC;
class DGKernel;
class InterfaceKernel;
class ScalarKernel;
class DiracKernel;
class NodalKernel;
class Split;

// libMesh forward declarations
namespace libMesh
{
template <typename T> class NumericVector;
template <typename T> class SparseMatrix;
}

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblem ;-)
 */
class AssemblySystem : public ConsoleStreamInterface
{
public:
  AssemblySystem(FEProblem & problem, Factory & factory, const std::string & name);
  virtual ~AssemblySystem();

  /**
   * Returns true if this system is currently computing the initial residual for a solve.
   * @return Whether or not we are currently computing the initial residual.
   */
  virtual bool computingInitialResidual() { return _computing_initial_residual; }

  // Setup Functions ////
  virtual void initialSetup();
  virtual void timestepSetup();

  void setupFieldDecomposition();

  bool haveFiniteDifferencedPreconditioner() {return _use_finite_differenced_preconditioner;}
  bool haveFieldSplitPreconditioner()        {return _use_field_split_preconditioner;}


  /**
   * Adds a kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a NodalKernel
   * @param kernel_name The type of the nodal kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  virtual void addNodalKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a scalar kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);


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
  MooseSharedPointer<Split> getSplit(const std::string & name);

  void zeroVectorForResidual(const std::string & vector_name);


  const std::vector<dof_id_type> & getVariableGlobalDoFs() { return _var_all_dof_indices; }

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


  /**
   * Update active objects of Warehouses owned by AssemblySystem
   */
  void updateActive(THREAD_ID tid);


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
   * If called with true this system will use a field split preconditioner matrix.
   */
  void useFieldSplitPreconditioner(bool use = true) { _use_field_split_preconditioner = use; }

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
   * Compute the incremental change in variables for dampers. Called before we use damping
   * @param tid Thread ID
   */
  void reinitIncrementForDampers(THREAD_ID tid);


  bool containsTimeKernel();
  ///@}

  /**
   * Return the number of non-linear iterations
   */
  unsigned int nNonlinearIterations() { return _n_iters; }

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
   * Access functions to Warehouses from outside AssemblySystem
   */
  const KernelWarehouse & getKernelWarehouse() { return _kernels; }
  const MooseObjectWarehouse<KernelBase> & getTimeKernelWarehouse() { return _time_kernels; }
  const MooseObjectWarehouse<KernelBase> & getNonTimeKernelWarehouse() { return _non_time_kernels; }
  const MooseObjectWarehouse<DGKernel> & getDGKernelWarehouse() { return _dg_kernels; }
  const MooseObjectWarehouse<InterfaceKernel> & getInterfaceKernelWarehouse() { return _interface_kernels; }
  const MooseObjectWarehouse<DiracKernel> & getDiracKernelWarehouse() { return _dirac_kernels; }
  const MooseObjectWarehouse<NodalKernel> & getNodalKernelWarehouse(THREAD_ID tid);
  const MooseObjectWarehouse<IntegratedBC> & getIntegratedBCWarehouse() { return _integrated_bcs; }
  const MooseObjectWarehouse<ElementDamper> & getElementDamperWarehouse() { return _element_dampers; }
  //@}

  /**
   * Weather or not the nonlinear system has save-ins
   */
  bool hasSaveIn() const { return _has_save_in || _has_nodalbc_save_in; }

  /**
   * Weather or not the nonlinear system has diagonal Jacobian save-ins
   */
  bool hasDiagSaveIn() const { return _has_diag_save_in || _has_nodalbc_diag_save_in; }


public:
  FEProblem & _fe_problem;
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

  void computeScalarKernelsJacobians(SparseMatrix<Number> & jacobian);

  /**
   * Enforce nodal constraints
   */
  void enforceNodalConstraintsResidual(NumericVector<Number> & residual);
  void enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian);


  /// solution vector from nonlinear solver
  const NumericVector<Number> * _current_solution;

  /// Time integrator
  MooseSharedPointer<TimeIntegrator> _time_integrator;

  /// \f$ {du^dot}\over{du} \f$
  Number _du_dot_du;

  ///@{
  /// Kernel Storage
  KernelWarehouse _kernels;
  MooseObjectWarehouse<ScalarKernel> _scalar_kernels;
  MooseObjectWarehouse<ScalarKernel> _time_scalar_kernels;
  MooseObjectWarehouse<ScalarKernel> _non_time_scalar_kernels;
  MooseObjectWarehouse<DGKernel> _dg_kernels;
  MooseObjectWarehouse<InterfaceKernel> _interface_kernels;
  MooseObjectWarehouse<KernelBase> _time_kernels;
  MooseObjectWarehouse<KernelBase> _non_time_kernels;

  ///@}

  ///@{
  /// BoundaryCondition Warhouses
  MooseObjectWarehouse<IntegratedBC> _integrated_bcs;
  MooseObjectWarehouse<NodalBC> _nodal_bcs;
  MooseObjectWarehouse<PresetNodalBC> _preset_nodal_bcs;
  ///@}

  /// Dirac Kernel storage for each thread
  MooseObjectWarehouse<DiracKernel> _dirac_kernels;

  /// Element Dampers for each thread
  MooseObjectWarehouse<ElementDamper> _element_dampers;

  /// General Dampers
  MooseObjectWarehouse<GeneralDamper> _general_dampers;

  /// NodalKernels for each thread
  MooseObjectWarehouse<NodalKernel> _nodal_kernels;

  /// Decomposition splits
  MooseObjectWarehouseBase<Split> _splits; // use base b/c there are no setup methods

  /// Constraints storage object
  ConstraintWarehouse _constraints;


protected:
  Factory  & _factory_assbly;
  MooseMesh & _mesh_assbly;
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
  MooseSharedPointer<Predictor> _predictor;

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



  std::vector<dof_id_type> _var_all_dof_indices;
};

#endif /* ASSEMBLYSYSTEM_H */
