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

// libMesh includes
#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"
#include "preconditioner.h"
#include "coupling_matrix.h"

class FEProblem;

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

  // Setup Functions ////
  virtual void initialSetup();
  virtual void initialSetupBCs();
  virtual void initialSetupKernels();
  virtual void timestepSetup();

  void setupFiniteDifferencedPreconditioner();

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  virtual bool converged();

  /**
   * Adds a scalar variable
   * @param var_name The name of the variable
   * @param order The order of the variable
   */
  virtual void addScalarVariable(const std::string & var_name, Order order);

  /**
   * Adds a kernel
   * @param kernel_name The type of the kernel
   * @param name The name of the kernel
   * @param parameters Kernel parameters
   */
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

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
   * @param bc_name The type of the boundary condition
   * @param name The name of the boundary condition
   * @param parameters Boundary condition parameters
   */
  void addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a Dirac kernal
   * @param kernel_name The type of the dirac kernel
   * @param name The name of the Dirac kernel
   * @param parameters Dirac kernel parameters
   */
  void addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Adds a DG kernal
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

  /// Adds a vector to the system
  void addVector(const std::string & vector_name, const bool project, const ParallelType type, bool zero_for_residual);

  void setInitialSolution();

  /**
   * Sets the value of constrained variables in the solution vector.
   */
  void setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced);

  /**
   * Modify the initial solution vector to apply a predictor
   * @param initial_solution The initial solution vector
   */
  void applyPredictor(NumericVector<Number> & initial_solution,
                      NumericVector<Number> & previous_solution);

  /**
   * Add residual contributions from Constraints
   *
   * @param displaced Controls whether to do the displaced Constraints or non-displaced
   */
  void constraintResiduals(NumericVector<Number> & residual, bool displaced);

  /**
   * Computes residual
   * @param residual Residual is formed in here
   */
  void computeResidual(NumericVector<Number> & residual);

  /**
   * Finds the implicit sparsity graph between geometrically related dofs.
   */
  void findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data, std::map<unsigned int, std::vector<unsigned int> > & graph);

  /**
   * Adds entries to the Jacobian in the correct positions for couplings coming from dofs being coupled that
   * are related geometrically (ie near eachother across a gap).
   */
  void addImplicitGeometricCouplingEntries(SparseMatrix<Number> & jacobian, GeometricSearchData & geom_search_data);

  /**
   * Add jacobian contributions from Constraints
   *
   * @param displaced Controls whether to do the displaced Constraints or non-displaced
   */
  void constraintJacobians(SparseMatrix<Number> & jacobian, bool displaced);

  /**
   * Computes Jacobian
   * @param jacobian Jacobian is formed in here
   */
  void computeJacobian(SparseMatrix<Number> &  jacobian);
  /**
   * Computes a Jacobian block. Used by Physics-based preconditioning
   * @param jacobian Where the block is stored
   * @param precond_system libMesh system that is used for the block Jacobian
   * @param ivar number of i-th variable
   * @param jvar number of j-th variable
   */
  void computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar);

  /**
   * Compute damping
   * @param update
   * @return returns The damping factor
   */
  Real computeDamping(const NumericVector<Number>& update);

  /**
   * Print the L2-norm of variable residuals
   */
  void printVarNorms();

  /**
   * Sets the time-stepping scheme
   * @param scheme Time-stepping scheme to be set
   */
  void timeSteppingScheme(Moose::TimeSteppingScheme scheme);

  /**
   * Get the order of used time integration scheme
   */
  Real getTimeSteppingOrder() { return _time_stepping_order; }

  /**
   * Called at the beginning of th time step
   */
  void onTimestepBegin();

  /**
   * Called from assembling when we hit a new subdomain
   * @param subdomain ID of the new subdomain
   * @param tid Thread ID
   */
  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid);

  virtual void set_solution(const NumericVector<Number> & soln);

  virtual NumericVector<Number> & solutionUDot() { return _solution_u_dot; }
  virtual NumericVector<Number> & solutionDuDotDu() { return _solution_du_dot_du; }

  virtual const NumericVector<Number> * & currentSolution() { return _current_solution; }

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution();

  virtual NumericVector<Number> & residualCopy();
  virtual NumericVector<Number> & residualGhosted();

  virtual void augmentSendList(std::vector<unsigned int> & send_list);

  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<unsigned int> & n_nz,
                               std::vector<unsigned int> & n_oz);

  /**
   * Sets a preconditioner
   * @param pc The preconditioner to be set
   */
  void setPreconditioner(Preconditioner<Real> *pc);

  /**
   * If called with true this system will use a finite differenced form of
   * the Jacobian as the preconditioner
   */
  void useFiniteDifferencedPreconditioner(bool use=true) { _use_finite_differenced_preconditioner = use; }

  /**
   * If called with true this will add entries into the jacobian to link together degrees of freedom that are found to
   * be related through the geometric search system.
   *
   * These entries are really only used by the Finite Difference Preconditioner right now.
   */
  void addImplicitGeometricCouplingEntriesToJacobian(bool add=true) { _add_implicit_geometric_coupling_entries_to_jacobian = add; }

  /**
   * Setup damping stuff (called before we actually start)
   */
  void setupDampers();
  /**
   * Reinit dampers. Called before we use damping
   * @param tid Thread ID
   */
  void reinitDampers(THREAD_ID tid);

  /// System Integrity Checks
  void checkKernelCoverage(const std::set<subdomain_id_type> & mesh_subdomains) const;
  void checkBCCoverage() const;
  bool containsTimeKernel();

  /**
   * Return the number of non-linear iterations
   */
  unsigned int nNonlinearIterations() { return _n_iters; }

  /**
   * Return the number of linear iterations
   */
  unsigned int nLinearIterations() { return _n_linear_iters; }

  /**
   * Return the final nonlinear residual
   */
  Real finalNonlinearResidual() { return _final_residual; }

  /**
   * Print n top residuals with variable name and node number
   * @param residual The residual we work with
   * @param n The number of residuals to print
   */
  void printTopResiduals(const NumericVector<Number> & residual, unsigned int n);

  void debuggingResiduals(bool state) { _debugging_residuals = state; }

  void setPredictorScale(Real scale)
  {
    _use_predictor = true;
    _predictor_scale = scale;
  }

public:
  FEProblem & _mproblem;
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
   * @param residual[out] Residual is formed here
   */
  void computeResidualInternal(NumericVector<Number> & residual);

  /**
   * Completes the assembly of residual
   * @param residual[out] Residual is formed here
   */
  void finishResidual(NumericVector<Number> & residual);

  void computeDiracContributions(NumericVector<Number> * residual, SparseMatrix<Number> * jacobian = NULL);

  /**
   * Enforce nodal constraints
   */
  void enforceNodalConstraintsResidual(NumericVector<Number> & residual);
  void enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian);

  const NumericVector<Number> * _current_solution;      ///< solution vector from nonlinear solver
  NumericVector<Number> & _older_solution;              ///< solution vector from step prior to previous step
  NumericVector<Number> & _solution_u_dot;              ///< solution vector for u^dot
  NumericVector<Number> & _solution_du_dot_du;          ///< solution vector for {du^dot}\over{du}
  NumericVector<Number> * _residual_old;                ///< residual evaluated at the old time step (need for Crank-Nicolson)
  NumericVector<Number> & _residual_ghosted;            ///< ghosted form of the residual

  NumericVector<Number> & _serialized_solution;         ///< Serialized version of the solution vector

  NumericVector<Number> & _residual_copy;               ///< Copy of the residual vector

  Real & _t;                                            ///< time
  Real & _dt;                                           ///< size of the time step
  Real & _dt_old;                                       ///< previous time step size
  int & _t_step;                                        ///< time step (number)
  std::vector<Real> & _time_weight;                     ///< Coefficients (weights) for the time discretization
  Moose::TimeSteppingScheme _time_stepping_scheme;      ///< Time stepping scheme used for time discretization
  Real _time_stepping_order;                            ///< The order of the time stepping scheme

  // holders
  std::vector<KernelWarehouse> _kernels;                ///< Kernel storage for each thread
  std::vector<BCWarehouse> _bcs;                        ///< BC storage for each thread
  std::vector<DiracKernelWarehouse> _dirac_kernels;     ///< Dirac Kernel storage for each thread
  std::vector<DGKernelWarehouse> _dg_kernels;           ///< DG Kernel storage for each thread
  std::vector<DamperWarehouse> _dampers;                ///< Dampers for each thread

public:
  std::vector<ConstraintWarehouse> _constraints;        ///< Constraints for each thread

protected:
  NumericVector<Number> * _increment_vec;               ///< increment vector

  Preconditioner<Real> * _preconditioner;               ///< Preconditioner

  bool _use_finite_differenced_preconditioner;          /// Whether or not to use a finite differenced preconditioner

  bool _add_implicit_geometric_coupling_entries_to_jacobian; /// Whether or not to add implicit geometric couplings to the Jacobian for FDP

  bool _need_serialized_solution;                       ///< Whether or not a copy of the residual needs to be made

  bool _need_residual_copy;                             ///< Whether or not a copy of the residual needs to be made
  bool _need_residual_ghosted;                          ///< Whether or not a ghosted copy of the residual needs to be made
  bool _debugging_residuals;                            ///< true if debugging residuals

  bool _doing_dg;                                       ///< true if DG is active (optimization reasons)

  std::vector<NumericVector<Number> *> _vecs_to_zero_for_residual;   ///< NumericVectors that will be zeroed before a residual computation

  unsigned int _n_iters;
  unsigned int _n_linear_iters;
  Real _final_residual;

  bool _use_predictor;                                   ///< true if predictor is active
  Real _predictor_scale;                                 ///< Scale factor to use with predictor

  friend class ComputeResidualThread;
  friend class ComputeJacobianThread;
  friend class ComputeFullJacobianThread;
  friend class ComputeDiracThread;
  friend class ComputeDampingThread;
};

#endif /* NONLINEARSYSTEM_H */
