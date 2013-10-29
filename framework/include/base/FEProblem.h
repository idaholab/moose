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

#ifndef FEPROBLEM_H
#define FEPROBLEM_H

#include "Moose.h"
#include "SubProblem.h"
#include "AuxiliarySystem.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "MaterialWarehouse.h"
#include "MaterialPropertyStorage.h"
#include "PostprocessorWarehouse.h"
#include "PostprocessorData.h"
#include "Output.h"
#include "Adaptivity.h"
#include "Resurrector.h"
#include "IndicatorWarehouse.h"
#include "MarkerWarehouse.h"
#include "MultiAppWarehouse.h"
#include "TransferWarehouse.h"
#include "MooseEnum.h"
#include "RestartableData.h"
#include "Resurrector.h"
#include "UserObjectWarehouse.h"
#include "NonlinearSystem.h"
#include "Restartable.h"
#include "ReportableData.h"

class DisplacedProblem;
class OutputProblem;
class FEProblem;
class MooseMesh;
class NonlinearSystem;
class RandomInterface;
class RandomData;

template<>
InputParameters validParams<FEProblem>();

enum MooseNonlinearConvergenceReason
{
  MOOSE_NONLINEAR_ITERATING      = 0,
  MOOSE_CONVERGED_FNORM_ABS      = 2,
  MOOSE_CONVERGED_FNORM_RELATIVE = 3,
  MOOSE_CONVERGED_SNORM_RELATIVE = 4,
  MOOSE_DIVERGED_FUNCTION_COUNT  = -2,
  MOOSE_DIVERGED_FNORM_NAN       = -4,
  MOOSE_DIVERGED_LINE_SEARCH     = -6
};

// The idea with these enums is to abstract the reasons for
// convergence/divergence, i.e. they could be used with linear algebra
// packages other than PETSc.  They were directly inspired by PETSc,
// though.  This enum could also be combined with the
// MooseNonlinearConvergenceReason enum but there might be some
// confusion (?)
enum MooseLinearConvergenceReason
{
  MOOSE_LINEAR_ITERATING                =  0,
  // MOOSE_CONVERGED_RTOL_NORMAL        =  1,
  // MOOSE_CONVERGED_ATOL_NORMAL        =  9,
  MOOSE_CONVERGED_RTOL                  =  2,
  MOOSE_CONVERGED_ATOL                  =  3,
  MOOSE_CONVERGED_ITS                   =  4
  // MOOSE_CONVERGED_CG_NEG_CURVE       =  5,
  // MOOSE_CONVERGED_CG_CONSTRAINED     =  6,
  // MOOSE_CONVERGED_STEP_LENGTH        =  7,
  // MOOSE_CONVERGED_HAPPY_BREAKDOWN    =  8,
  // MOOSE_DIVERGED_NULL                = -2,
  // MOOSE_DIVERGED_ITS                 = -3,
  // MOOSE_DIVERGED_DTOL                = -4,
  // MOOSE_DIVERGED_BREAKDOWN           = -5,
  // MOOSE_DIVERGED_BREAKDOWN_BICG      = -6,
  // MOOSE_DIVERGED_NONSYMMETRIC        = -7,
  // MOOSE_DIVERGED_INDEFINITE_PC       = -8,
  // MOOSE_DIVERGED_NANORINF            = -9,
  // MOOSE_DIVERGED_INDEFINITE_MAT      = -10
};

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class FEProblem :
  public SubProblem,
  public Restartable
{
public:
  FEProblem(const std::string & name, InputParameters parameters);
  virtual ~FEProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }

  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid);
  virtual void setCoordSystem(const std::vector<SubdomainName> & blocks, const std::vector<MooseEnum> & coord_sys);

  /**
   * Set the coupling between variables
   * TODO: allow user-defined coupling
   * @param type Type of coupling
   */
  void setCoupling(Moose::CouplingType type);

  Moose::CouplingType coupling() { return _coupling; }

  /**
   * Set custom coupling matrix
   * @param cm coupling matrix to be set
   */
  void setCouplingMatrix(CouplingMatrix * cm);
  CouplingMatrix * & couplingMatrix() { return _cm; }

  std::vector<std::pair<unsigned int, unsigned int> > & couplingEntries(THREAD_ID tid) { return _assembly[tid]->couplingEntries(); }

  /**
   * Check for converence of the nonlinear solution
   * @param msg            Error message that gets sent back to the solver
   * @param it             Iteration counter
   * @param xnorm          Norm of the solution vector
   * @param snorm          Norm of the change in the solution vector
   * @param fnorm          Norm of the residual vector
   * @param rtol           Relative residual convergence tolerance
   * @param stol           Solution change convergence tolerance
   * @param abstol         Absolute residual convergence tolerance
   * @param nfuncs         Number of function evaluations
   * @param max_funcs      Maximum Number of function evaluations
   * @param ref_resid      Reference residual to be used in relative convergence check
   * @param div_threshold  Maximum value of residual before triggering divergence check
   */
  virtual MooseNonlinearConvergenceReason checkNonlinearConvergence(std::string &msg,
                                                                    const int it,
                                                                    const Real xnorm,
                                                                    const Real snorm,
                                                                    const Real fnorm,
                                                                    const Real rtol,
                                                                    const Real stol,
                                                                    const Real abstol,
                                                                    const int nfuncs,
                                                                    const int max_funcs,
                                                                    const Real ref_resid,
                                                                    const Real div_threshold);

  /**
   * Check for converence of the linear solution
   * @param msg            Error message that gets sent back to the solver
   * @param n              Iteration counter
   * @param rnorm          Norm of the residual vector
   * @param rtol           Relative residual convergence tolerance
   * @param atol           Absolute residual convergence tolerance
   * @param dtol           Divergence tolerance
   * @param maxits         Maximum number of linear iterations allowed
   */
  virtual MooseLinearConvergenceReason checkLinearConvergence(std::string &msg,
                                                              const int n,
                                                              const Real rnorm,
                                                              const Real rtol,
                                                              const Real atol,
                                                              const Real dtol,
                                                              const int maxits);

#ifdef LIBMESH_HAVE_PETSC
  void storePetscOptions(const std::vector<MooseEnum> & petsc_options,
                         const std::vector<std::string> & petsc_options_inames,
                         const std::vector<std::string> & petsc_options_values);
#endif

  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual bool hasScalarVariable(const std::string & var_name);
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars, THREAD_ID tid);

  /**
   * Get the MOOSE variables to be reinited on each element.
   *
   * @param tid The thread id
   */
  virtual const std::set<MooseVariable *> & getActiveElementalMooseVariables(THREAD_ID tid);

  /**
   * Whether or not a list of active elemental moose variables has been set.
   *
   * @return True if there has been a list of active elemental moose variables set, False otherwise
   */
  virtual bool hasActiveElementalMooseVariables(THREAD_ID tid);

  /**
   * Clear the active elemental MooseVariable.  If there are no active variables then they will all be reinited.
   * Call this after finishing the computation that was using a restricted set of MooseVariables
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(THREAD_ID tid);

  virtual void createQRules(QuadratureType type, Order order);
  virtual Order getQuadratureOrder() { return _quadrature_order; }
  virtual Assembly & assembly(THREAD_ID tid) { return *_assembly[tid]; }

  /**
   * Returns a list of all the variables in the problem (both from the NL and Aux systems.
   */
  std::vector<VariableName> getVariableNames();


  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual void prepareFace(const Elem * elem, THREAD_ID tid);
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid);

  virtual void prepareAssembly(THREAD_ID tid);

  virtual void addGhostedElem(unsigned int elem_id);
  virtual void addGhostedBoundary(BoundaryID boundary_id);
  virtual void ghostGhostedBoundaries();

  virtual void sizeZeroes(unsigned int size, THREAD_ID tid);
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid);
  virtual void reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid);
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid);
  virtual void reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid);
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid);
  virtual void reinitScalars(THREAD_ID tid);
  virtual void reinitOffDiagScalars(THREAD_ID tid);

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual void subdomainSetup(SubdomainID subdomain, THREAD_ID tid);
  virtual void subdomainSetupSide(SubdomainID subdomain, THREAD_ID tid);

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache);

  virtual void init();
  virtual void init2();
  virtual void solve();
  virtual bool converged();
  virtual unsigned int nNonlinearIterations() { return _nl.nNonlinearIterations(); }
  virtual unsigned int nLinearIterations() { return _nl.nLinearIterations(); }
  virtual Real finalNonlinearResidual() { return _nl.finalNonlinearResidual(); }

  virtual bool computingInitialResidual() { return _nl.computingInitialResidual(); }

  /**
   * The relative (both to solution size and dt) change in the L2 norm of the solution vector.
   * Call just after a converged solve.
   */
  virtual Real solutionChangeNorm();

  virtual void onTimestepBegin();
  virtual void onTimestepEnd();

  virtual Real & time() const { return _time; }
  virtual Real & timeOld() const { return _time_old; }
  virtual int & timeStep() const { return _t_step; }
  virtual Real & dt() const { return _dt; }
  virtual Real & dtOld() const { return _dt_old; }

  virtual void transient(bool trans) { _transient = trans; }
  virtual bool isTransient() const { return _transient; }

  virtual void addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters);
  virtual void addPredictor(const std::string & type, const std::string & name, InputParameters parameters);

  virtual void copySolutionsBackwards();
  // Update backward time solution vectors
  virtual void copyOldSolutions();
  virtual void restoreSolutions();

  virtual const std::vector<MooseObject *> & getObjectsByName(const std::string & name, THREAD_ID tid);

  // Function /////
  virtual void addFunction(std::string type, const std::string & name, InputParameters parameters);
  virtual bool hasFunction(const std::string & name, THREAD_ID tid = 0);
  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  // NL /////
  NonlinearSystem & getNonlinearSystem() { return _nl; }
  void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL);
  void addScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);
  void addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);


  // Aux /////
  void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains = NULL);
  void addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);
  void addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addAuxScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  AuxiliarySystem & getAuxiliarySystem() { return _aux; }

  // Dirac /////
  void addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // DG /////
  void addDGKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // IC /////
  void addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters);

  void projectSolution();

  // Materials /////
  void addMaterial(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  /**
   * Get list of materials with specified name
   * @param name The name of the material
   * @param tid Thread ID
   * @return The list of materials with the name 'name'
   */
  virtual const std::vector<Material*> & getMaterialsByName(const std::string & name, THREAD_ID tid);
  virtual const std::vector<Material*> & getMaterials(SubdomainID block_id, THREAD_ID tid);
  virtual const std::vector<Material*> & getFaceMaterials(SubdomainID block_id, THREAD_ID tid);
  virtual const std::vector<Material*> & getBndMaterials(BoundaryID block_id, THREAD_ID tid);
  virtual const std::vector<Material*> & getNeighborMaterials(SubdomainID block_id, THREAD_ID tid);
  virtual void updateMaterials();

  /**
   * Add the MooseVariables that the current materials depend on to the dependency list.
   *
   * This MUST be done after the dependency list has been set for all the other objects!
   */
  virtual void prepareMaterials(SubdomainID blk_id, THREAD_ID tid);

  virtual void reinitMaterials(SubdomainID blk_id, THREAD_ID tid);
  virtual void reinitMaterialsFace(SubdomainID blk_id, THREAD_ID tid);
  virtual void reinitMaterialsNeighbor(SubdomainID blk_id, THREAD_ID tid);
  virtual void reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid);
  /*
   * Swap back underlying data storing stateful material properties
   */
  virtual void swapBackMaterials(THREAD_ID tid);
  virtual void swapBackMaterialsFace(THREAD_ID tid);
  virtual void swapBackMaterialsNeighbor(THREAD_ID tid);

  // Postprocessors /////
  virtual void addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters);

  void clearPostprocessorTables();

  // UserObjects /////
  virtual void addUserObject(std::string user_object_name, const std::string & name, InputParameters parameters);

  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @param tid The thread ID
   * @return Const reference to the user object
   */
  template <class T>
  const T & getUserObject(const std::string & name)
  {
    ExecFlagType types[] = { EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_INITIAL, EXEC_JACOBIAN, EXEC_RESIDUAL, EXEC_CUSTOM };
    for (unsigned int i = 0; i < LENGTHOF(types); i++)
      if (_user_objects(types[i])[0].hasUserObject(name))
      {
        UserObject * user_object = _user_objects(types[i])[0].getUserObjectByName(name);
        return dynamic_cast<const T &>(*user_object);
      }

    mooseError("Unable to find user object with name '" + name + "'");
  }
  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @param tid The thread ID
   * @return Const reference to the user object
   */
  const UserObject & getUserObjectBase(const std::string & name);

  /**
   * Check if there if a user object of given name
   * @param name The name of the user object being checked for
   * @param tid  The thread ID
   * @return true if the user object exists, false otherwise
   */
  bool hasUserObject(const std::string & name);

  /**
   * Check existence of the postprocessor.
   * @param name The name of the post-processor
   * @param tid Thread ID
   * @return true if it exists, otherwise false
   */
  bool hasPostprocessor(const std::string & name, THREAD_ID tid = 0);

  /**
   * Get a reference to the value associated with the postprocessor.
   */
  Real & getPostprocessorValue(const PostprocessorName & name, THREAD_ID tid = 0);

  /**
   * Get the reference to the old value of a post-processor
   * @param name The name of the post-processor
   * @param tid Thread ID
   * @return The reference to the old value
   */
  Real & getPostprocessorValueOld(const std::string & name, THREAD_ID tid = 0);

  /**
   * Get a reference to the PostprocessorWarehouse ExecStore object
   */
  ExecStore<PostprocessorWarehouse> & getPostprocessorWarehouse();

  virtual void computeUserObjects(ExecFlagType type = EXEC_TIMESTEP, UserObjectWarehouse::GROUP group = UserObjectWarehouse::ALL);
  virtual void computeAuxiliaryKernels(ExecFlagType type = EXEC_RESIDUAL);
  virtual void outputPostprocessors(bool force = false);

  // Dampers /////
  void addDamper(std::string damper_name, const std::string & name, InputParameters parameters);
  void setupDampers();

  /**
   * Whether or not this system has dampers.
   */
  bool hasDampers() { return _has_dampers; }

  // Indicators /////
  void addIndicator(std::string indicator_name, const std::string & name, InputParameters parameters);

  // Markers //////
  void addMarker(std::string marker_name, const std::string & name, InputParameters parameters);

  /**
   * Add a MultiApp to the problem.
   */
  void addMultiApp(const std::string & multi_app_name, const std::string & name, InputParameters parameters);

  /**
   * Get a MultiApp object by name.
   */
  MultiApp * getMultiApp(const std::string & multi_app_name);

  /**
   * Execute the MultiApps associated with the ExecFlagType
   */
  void execMultiApps(ExecFlagType type);

  /**
   * Find the smallest timestep over all MultiApps
   */
  Real computeMultiAppsDT(ExecFlagType type);

  /**
   * Add a Transfer to the problem.
   */
  void addTransfer(const std::string & transfer_name, const std::string & name, InputParameters parameters);

  /**
   * Execute the Transfers associated with the ExecFlagType
   *
   * Note: This does _not_ execute MultiApp Transfers!
   * Those are executed automatically when MultiApps are executed.
   */
  void execTransfers(ExecFlagType type);

  /// Evaluates transient residual G in canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitResidual(Real time, const NumericVector<Number>& u, const NumericVector<Number>& udot, NumericVector<Number>& residual);

  /// Evaluates transient Jacobian J_a = dG/dU + a*dG/dUdot from canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitJacobian(Real time, const NumericVector<Number>& u, const NumericVector<Number>& udot, Real shift, SparseMatrix<Number> &jacobian);

  ////
  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual );
  virtual void computeResidualType(const NumericVector<Number> & soln, NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_ALL);
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> &  jacobian);
  virtual void computeJacobianBlock(SparseMatrix<Number> &  jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar);
  virtual Real computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update);

  /**
   * Check to see whether the problem should update the solution
   * @return true if the problem should update the solution, false otherwise
   */
  virtual bool shouldUpdateSolution();

  /**
   * Update the solution
   * @param vec_solution      Local solution vector that gets modified by this method
   * @param ghosted_solution  Ghosted solution vector
   * @return true if the solution was modified, false otherwise
   */
  virtual bool updateSolution(NumericVector<Number>& vec_solution, NumericVector<Number>& ghosted_solution);

  /**
   * Perform cleanup tasks after application of predictor to solution vector
   * @param ghosted_solution  Ghosted solution vector
   */
  virtual void predictorCleanup(NumericVector<Number>& ghosted_solution);

  virtual void computeBounds(NonlinearImplicitSystem & sys, NumericVector<Number> & lower, NumericVector<Number> & upper);
  virtual void computeNearNullSpace(NonlinearImplicitSystem & sys, std::vector<NumericVector<Number>*> &sp);
  virtual void computeNullSpace(NonlinearImplicitSystem & sys, std::vector<NumericVector<Number>*> &sp);

  virtual void computeIndicatorsAndMarkers();

  virtual NumericVector<Number> & residualVector(Moose::KernelType type);

  virtual void addResidual(THREAD_ID tid);
  virtual void addResidualNeighbor(THREAD_ID tid);
  virtual void addResidualScalar(THREAD_ID tid = 0);

  virtual void cacheResidual(THREAD_ID tid);
  virtual void cacheResidualNeighbor(THREAD_ID tid);
  virtual void addCachedResidual(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid);
  virtual void addJacobianScalar(SparseMatrix<Number> & jacobian, THREAD_ID tid = 0);
  virtual void addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian, unsigned int ivar, THREAD_ID tid = 0);

  virtual void cacheJacobian(THREAD_ID tid);
  virtual void cacheJacobianNeighbor(THREAD_ID tid);
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);

  virtual void prepareShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid);

  // Displaced problem /////
  virtual void initDisplacedProblem(MooseMesh * displaced_mesh, InputParameters params);
  virtual DisplacedProblem * & getDisplacedProblem() { return _displaced_problem; }

  virtual void updateGeomSearch();

  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  // Output /////
  virtual Output & out() { return _out; }
  virtual void output(bool force = false);
  virtual void outputDisplaced(bool state = true) { _output_displaced = state; }
  virtual void outputSolutionHistory(bool state = true) { _output_solution_history = state; }
  virtual void outputESInfo(bool state = true) { _output_es_info = state; }


  /**
   * Whether or not we should be printing the linear residuals.
   * @param state True to print linear residuals.
   */
  virtual void printLinearResiduals(bool state) { _print_linear_residuals = state; }

  /**
   * Whether or not we should be printing the linear residuals.
   */
  virtual bool shouldPrintLinearResiduals() { return _print_linear_residuals; }

  /**
   * Set which variables will be written in ouput files
   * @param output_variables The list of variable names to write in the ouput files
   */
  void setOutputVariables();
  void hideVariableFromOutput(const VariableName & var_name);
  void hideVariableFromOutput(const std::vector<VariableName> & var_names);
  void showVariableInOutput(const VariableName & var_name);
  void showVariableInOutput(const std::vector<VariableName> & var_names);

  OutputProblem & getOutputProblem(unsigned int refinements, MeshFileName file = "");
  void setMaxPPSRowsScreen(unsigned int n) { _pps_output_table_max_rows = n; }
  void setPPSFitScreen(MooseEnum m) { _pps_fit_to_screen = m; }

  /**
   * Set (or reset) the output position of the problem.
   */
  void setOutputPosition(Point p);

  // Restart //////

  /**
   * Set a file we will restart from
   * @param file_name The file name we will restart from
   */
  virtual void setRestartFile(const std::string & file_name);

  /**
   * Set the number of restart files to save
   * @param num_files Number of files to keep around
   */
  virtual void setNumRestartFiles(unsigned int num_files);

  /**
   * Gets the number of restart files to save
   * @return the number of files to keep around
   */
  virtual unsigned int getNumRestartFiles();

  /**
   * Was this subproblem initialized from a restart file
   * @return true if we restarted form a file, otherwise false
   */
  virtual bool isRestarting();

  /**
   * Are we recovering a previous simulation??
   * @return true if recovering form a file, otherwise false
   */
  virtual bool isRecovering();

  /**
   * Register a piece of restartable data.  This is data that will get
   * written / read to / from a restart file.
   *
   * @param name The full (unique) name.
   * @param data The actual data object.
   * @param tid The thread id of the object.  Use 0 if the object is not threaded.
   */
  virtual void registerRestartableData(std::string name, RestartableDataValue * data, THREAD_ID tid);

  /**
   * Determines the solver mode based on existin options and returns it.
   * In MOOSE we default to PJFNK if no solver type is explicitly set!
   */
  std::string solverMode();

#ifdef LIBMESH_ENABLE_AMR
  // Adaptivity /////
  Adaptivity & adaptivity() { return _adaptivity; }
  virtual void adaptMesh();
#endif //LIBMESH_ENABLE_AMR
  virtual void meshChanged();

  void printMaterialMap();
  void checkProblemIntegrity();

  void serializeSolution();

  inline void setEarlyPerfLogPrint(bool val) { _output_setup_log_early = val; }

  // debugging iface /////

  /**
   * Set the number of top residual to be printed out (0 = no output)
   */
  void setDebugTopResiduals(unsigned int n) { _dbg_top_residuals = n; }

  void setDebugPrintVarResidNorms(bool should_print) { _dbg_print_var_rnorms = should_print; }

  void setKernelTypeResidual(Moose::KernelType kt) { _kernel_type = kt; }

  /**
   * Set flag that Jacobian is constant (for optimization purposes)
   * @param state True if the Jacobian is constant, false otherwise
   */
  void setConstJacobian(bool state) { _const_jacobian = state; }

  /**
   * Access to the ReportableData storage class
   * @return Reference to the ReportableData object
   */
  ReportableData & getReportableData();

  void registerRandomInterface(RandomInterface & random_interface, const std::string & name);

protected:
  MooseMesh & _mesh;
  EquationSystems _eq;
  bool _initialized;
  Moose::KernelType _kernel_type;

  /// Whether or not to actually solve the nonlinear system
  bool _solve;

  bool _transient;
  Real & _time;
  Real & _time_old;
  int & _t_step;
  Real & _dt;
  Real & _dt_old;

  /// Objects by names, indexing: [thread][name]->array of moose objects with name 'name'
  std::vector<std::map<std::string, std::vector<MooseObject *> > > _objects_by_name;

  NonlinearSystem _nl;
  AuxiliarySystem _aux;

  Moose::CouplingType _coupling;                        ///< Type of variable coupling
  CouplingMatrix * _cm;                                 ///< Coupling matrix for variables. It is diagonal, since we do only block diagonal preconditioning.

  // Dimension of the subspace spanned by the vectors with a given prefix
  std::map<std::string,unsigned int> _subspace_dim;

  // quadrature
  Order _quadrature_order;                              ///< Quadrature order required by all variables to integrated over them.
  std::vector<Assembly *> _assembly;

  /// functions
  std::vector<std::map<std::string, Function *> > _functions;

  /// Initial condition warehouses (one for each thread)
  std::vector<InitialConditionWarehouse> _ics;

  // material properties
  MaterialPropertyStorage _material_props;
  MaterialPropertyStorage _bnd_material_props;

  std::vector<MaterialData *> _material_data;
  std::vector<MaterialData *> _bnd_material_data;
  std::vector<MaterialData *> _neighbor_material_data;

  // materials
  std::vector<MaterialWarehouse> _materials;

  // indicators
  std::vector<IndicatorWarehouse> _indicators;

  // markers
  std::vector<MarkerWarehouse> _markers;

  // postprocessors
  std::vector<PostprocessorData*> _pps_data;
  ExecStore<PostprocessorWarehouse> _pps;

  // user objects
  ExecStore<UserObjectWarehouse> _user_objects;

  ExecStore<MultiAppWarehouse> _multi_apps;

  /// Normal Transfers
  ExecStore<TransferWarehouse> _transfers;

  /// Transfers executed just before MultiApps to transfer data to them
  ExecStore<TransferWarehouse> _to_multi_app_transfers;

  /// Transfers executed just after MultiApps to transfer data from them
  ExecStore<TransferWarehouse> _from_multi_app_transfers;

  /// A map of objects that consume random numbers
  std::map<std::string, RandomData *> _random_data_objects;

  /// Table with postprocessors that will go into files
  FormattedTable _pps_output_table_file;
  /// Table with postprocessors that will go on screen
  FormattedTable _pps_output_table_screen;
  unsigned int _pps_output_table_max_rows;
  MooseEnum _pps_fit_to_screen;

  bool _print_linear_residuals;

  void computeUserObjectsInternal(std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group);

public:
  /**
   * Dimension of the subspace spanned by vectors with a given prefix.
   * @param prefix Prefix of the vectors spanning the subspace.
   */
  unsigned int subspaceDim(const std::string& prefix) const {if(_subspace_dim.count(prefix)) return _subspace_dim.find(prefix)->second; else return 0;}

  bool _postprocessor_screen_output;
  bool _postprocessor_csv_output;
  bool _postprocessor_gnuplot_output;
  std::string _gnuplot_format;

protected:
  void checkUserObjects();

  /// Verify that there are no element type/coordinate type conflicts
  void checkCoordinateSystems();

  /**
   * Add postprocessor values to the output table
   * @param type type of PPS to add to the table
   */
  void addPPSValuesToTable(ExecFlagType type);

  /**
   * Call when it is possible that the needs for ghosted elements has changed.
   */
  void reinitBecauseOfGhosting();

  // Output system
  Output _out;
  OutputProblem * _out_problem;

#ifdef LIBMESH_ENABLE_AMR
  Adaptivity _adaptivity;
#endif

  // Displaced mesh /////
  MooseMesh * _displaced_mesh;
  DisplacedProblem * _displaced_problem;
  GeometricSearchData _geometric_search_data;

  bool _reinit_displaced_elem;
  bool _reinit_displaced_face;
  /// true for outputting displaced problem
  bool _output_displaced;
  /// true for outputting solution history
  bool _output_solution_history;
  /// true for outputting equations systems information
  bool _output_es_info;

  /// whether input file has been written
  bool _input_file_saved;

  /// Whether or not this system has any Dampers associated with it.
  bool _has_dampers;

  /// Whether or not this system has any Constraints.
  bool _has_constraints;

  /// Whether nor not stateful materials have been initialized
  bool _has_initialized_stateful;

  /// Object responsible for restart (read/write)
  Resurrector * _resurrector;

//  PerfLog _solve_only_perf_log;                         ///< Only times the solve
  /// Determines if the setup log is printed before the first time step
  bool _output_setup_log_early;

  std::vector<VariableName> _variable_white_list;
  std::vector<VariableName> _variable_black_list;

  /// Number of top residual to print out
  unsigned int _dbg_top_residuals;

  // Should we print out residuals of individaul variables at NL iterations?
  bool _dbg_print_var_rnorms;

  /// true if the Jacobian is constant
  bool _const_jacobian;
  /// Indicates if the Jacobian was computed
  bool _has_jacobian;

  std::string _solver_mode;

  /// True if we're doing a _restart_ (note: this is _not_ true when recovering!)
  bool _restarting;

  /// Storage facility for Reportable values
  ReportableData _reportable_data;

public:
  /// number of instances of FEProblem (to distinguish Systems when coupling problems together)
  static unsigned int _n;

  friend class AuxiliarySystem;
  friend class NonlinearSystem;
  friend class Resurrector;
  friend class MaterialPropertyIO;
  friend class RestartableDataIO;
  friend class ComputeInitialConditionThread;
  friend class ComputeBoundaryInitialConditionThread;
};

#endif /* FEPROBLEM_H */
