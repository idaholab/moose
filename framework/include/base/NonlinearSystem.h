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

#include "AssemblySystem.h"


// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/nonlinear_implicit_system.h"

// Forward declarations
class FEProblem;

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblem ;-)
 */
class NonlinearSystem : public SystemTempl<TransientNonlinearImplicitSystem>,
                        public AssemblySystem
{
public:
  NonlinearSystem(FEProblem & problem, const std::string & name);
  ~NonlinearSystem();

  virtual void init() override;
  virtual void solve() override;
  virtual void restoreSolutions() override;

  /**
   * Set transient term used by residual and Jacobian evaluation.
   * @param udot transient term
   * @note If the calling sequence for residual evaluation was changed, this could become an explicit argument.
   */
  virtual void setSolutionUDot(const NumericVector<Number> & udot);

  virtual NumericVector<Number> & solutionUDot() override;
  virtual NumericVector<Number> & residualVector(Moose::KernelType type) override;

  virtual const NumericVector<Number> * & currentSolution() override { return _current_solution; }

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution() override;

  virtual NumericVector<Number> & residualCopy() override;
  virtual NumericVector<Number> & residualGhosted() override;

  virtual void augmentSparsity(SparsityPattern::Graph & sparsity,
                               std::vector<dof_id_type> & n_nz,
                               std::vector<dof_id_type> & n_oz) override;


 /**
   * Returns the current nonlinear iteration number.  In libmesh, this is
   * updated during the nonlinear solve, so it should be up-to-date.
   */
  unsigned int getCurrentNonlinearIterationNumber() { return _sys.get_current_nonlinear_iteration_number(); }

  /**
   * The relative L2 norm of the difference between solution and old solution vector.
   */
  virtual Real relativeSolutionDifferenceNorm();

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve();

  void setupFiniteDifferencedPreconditioner();

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
   * Adds a boundary condition
   * @param bc_name The type of the boundary condition
   * @param name The name of the boundary condition
   * @param parameters Boundary condition parameters
   */
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  /**
   * Adds an interface kernel
   * @param interface_kernel_name The type of the interface kernel
   * @param name The name of the interface kernel
   * @param parameters interface kernel parameters
   */
  void addInterfaceKernel(std::string interface_kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Computes residual
   * @param residual Residual is formed in here
   * @param type The type of kernels for which the residual is to be computed.
   */
  void computeResidual(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_ALL);

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
   * Compute the residual
   * @param type The type of kernels for which the residual is to be computed.
   */
  void computeResidualInternal(Moose::KernelType type = Moose::KT_ALL);

  /**
    * Enforces nodal boundary conditions
    * @param residual Residual where nodal BCs are enforced (input/output)
  */
  void computeNodalBCs(NumericVector<Number> & residual);

  void getNodeDofs(unsigned int node_id, std::vector<dof_id_type> & dofs);


  /**
    * Finds the implicit sparsity graph between geometrically related dofs.
    */
  void findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data,
                                              std::map<dof_id_type, std::vector<dof_id_type> > & graph);


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

  void computeJacobianInternal(SparseMatrix<Number> &  jacobian);

  /// set all the global dof indices for a nonlinear variable
  void setVariableGlobalDoFs(const std::string & var_name);

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
    * @param solution The trail solution vector
    * @param update The incremental update to the solution vector
    * @return returns The damping factor
    */
  Real computeDamping(const NumericVector<Number> & solution,
                        const NumericVector<Number> & update);


  void computeDiracContributions(SparseMatrix<Number> * jacobian = NULL);

  virtual void setSolution(const NumericVector<Number> & soln);

  /**
   * Setup damping stuff (called before we actually start)
   */
  void setupDampers();

  ///@{
  /// System Integrity Checks
  void checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains) const;

protected:
  /// ghosted form of the residual
  NumericVector<Number> & _residual_ghosted;

  /// Serialized version of the solution vector
  NumericVector<Number> & _serialized_solution;

  /// Copy of the residual vector
  NumericVector<Number> & _residual_copy;

  /// solution vector for u^dot
  NumericVector<Number> & _u_dot;

  /// residual vector for time contributions
  NumericVector<Number> & _Re_time;
  /// residual vector for non-time contributions
  NumericVector<Number> & _Re_non_time;
  /// The difference of current and old solutions
  NumericVector<Number> & _sln_diff;
};

#endif /* NONLINEARSYSTEM_H */
