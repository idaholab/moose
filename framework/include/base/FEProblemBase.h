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

#ifndef FEPROBLEMBASE_H
#define FEPROBLEMBASE_H

// MOOSE includes
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "PostprocessorData.h"
#include "VectorPostprocessorData.h"
#include "Adaptivity.h"
#include "InitialConditionWarehouse.h"
#include "Restartable.h"
#include "SolverParams.h"
#include "PetscSupport.h"
#include "MooseApp.h"
#include "ExecuteMooseObjectWarehouse.h"
#include "AuxGroupExecuteMooseObjectWarehouse.h"
#include "MaterialWarehouse.h"
#include "MooseVariableBase.h"
#include "MultiAppTransfer.h"
#include "Postprocessor.h"

#include "libmesh/enum_quadrature_type.h"
#include "libmesh/equation_systems.h"

#include <unordered_map>

// Forward declarations
class AuxiliarySystem;
class DisplacedProblem;
class FEProblemBase;
class MooseMesh;
class NonlinearSystemBase;
class NonlinearSystem;
class RandomInterface;
class RandomData;
class MeshChangedInterface;
class MultiMooseEnum;
class MaterialPropertyStorage;
class MaterialData;
class MooseEnum;
class Resurrector;
class Assembly;
class JacobianBlock;
class Control;
class MultiApp;
class TransientMultiApp;
class ScalarInitialCondition;
class Indicator;
class InternalSideIndicator;
class Marker;
class Material;
class Transfer;
class XFEMInterface;
class SideUserObject;
class NodalUserObject;
class ElementUserObject;
class InternalSideUserObject;
class GeneralUserObject;
class Function;
class Distribution;
class Sampler;
class KernelBase;
class IntegratedBC;

// libMesh forward declarations
namespace libMesh
{
class CouplingMatrix;
class NonlinearImplicitSystem;
}

template <>
InputParameters validParams<FEProblemBase>();

enum MooseNonlinearConvergenceReason
{
  MOOSE_NONLINEAR_ITERATING = 0,
  MOOSE_CONVERGED_FNORM_ABS = 2,
  MOOSE_CONVERGED_FNORM_RELATIVE = 3,
  MOOSE_CONVERGED_SNORM_RELATIVE = 4,
  MOOSE_DIVERGED_FUNCTION_COUNT = -2,
  MOOSE_DIVERGED_FNORM_NAN = -4,
  MOOSE_DIVERGED_LINE_SEARCH = -6
};

// The idea with these enums is to abstract the reasons for
// convergence/divergence, i.e. they could be used with linear algebra
// packages other than PETSc.  They were directly inspired by PETSc,
// though.  This enum could also be combined with the
// MooseNonlinearConvergenceReason enum but there might be some
// confusion (?)
enum MooseLinearConvergenceReason
{
  MOOSE_LINEAR_ITERATING = 0,
  // MOOSE_CONVERGED_RTOL_NORMAL        =  1,
  // MOOSE_CONVERGED_ATOL_NORMAL        =  9,
  MOOSE_CONVERGED_RTOL = 2,
  MOOSE_CONVERGED_ATOL = 3,
  MOOSE_CONVERGED_ITS = 4,
  // MOOSE_CONVERGED_CG_NEG_CURVE       =  5,
  // MOOSE_CONVERGED_CG_CONSTRAINED     =  6,
  // MOOSE_CONVERGED_STEP_LENGTH        =  7,
  // MOOSE_CONVERGED_HAPPY_BREAKDOWN    =  8,
  MOOSE_DIVERGED_NULL = -2,
  // MOOSE_DIVERGED_ITS                 = -3,
  // MOOSE_DIVERGED_DTOL                = -4,
  // MOOSE_DIVERGED_BREAKDOWN           = -5,
  // MOOSE_DIVERGED_BREAKDOWN_BICG      = -6,
  // MOOSE_DIVERGED_NONSYMMETRIC        = -7,
  // MOOSE_DIVERGED_INDEFINITE_PC       = -8,
  MOOSE_DIVERGED_NANORINF = -9,
  // MOOSE_DIVERGED_INDEFINITE_MAT      = -10
  MOOSE_DIVERGED_PCSETUP_FAILED = -11
};

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class FEProblemBase : public SubProblem, public Restartable
{
public:
  FEProblemBase(const InputParameters & parameters);
  virtual ~FEProblemBase();

  virtual EquationSystems & es() override { return _eq; }
  virtual MooseMesh & mesh() override { return _mesh; }

  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) override;
  virtual void setCoordSystem(const std::vector<SubdomainName> & blocks,
                              const MultiMooseEnum & coord_sys);
  void setAxisymmetricCoordAxis(const MooseEnum & rz_coord_axis);

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
  void setCouplingMatrix(std::unique_ptr<CouplingMatrix> cm);

  // DEPRECATED METHOD
  void setCouplingMatrix(CouplingMatrix * cm);

  const CouplingMatrix * couplingMatrix() { return _cm.get(); }

  /// Set custom coupling matrix for variables requiring nonlocal contribution
  void setNonlocalCouplingMatrix();

  bool areCoupled(unsigned int ivar, unsigned int jvar);

  std::vector<std::pair<MooseVariable *, MooseVariable *>> & couplingEntries(THREAD_ID tid);
  std::vector<std::pair<MooseVariable *, MooseVariable *>> & nonlocalCouplingEntries(THREAD_ID tid);

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
   * @param initial_residual_before_preset_bcs      Residual norm prior to imposition of PresetBC
   * values on solution vector
   * @param div_threshold  Maximum value of residual before triggering divergence check
   */
  virtual MooseNonlinearConvergenceReason
  checkNonlinearConvergence(std::string & msg,
                            const PetscInt it,
                            const Real xnorm,
                            const Real snorm,
                            const Real fnorm,
                            const Real rtol,
                            const Real stol,
                            const Real abstol,
                            const PetscInt nfuncs,
                            const PetscInt max_funcs,
                            const Real initial_residual_before_preset_bcs,
                            const Real div_threshold);

  /**
   * Check for convergence of the linear solution
   * @param msg            Error message that gets sent back to the solver
   * @param n              Iteration counter
   * @param rnorm          Norm of the residual vector
   * @param rtol           Relative residual convergence tolerance
   * @param atol           Absolute residual convergence tolerance
   * @param dtol           Divergence tolerance
   * @param maxits         Maximum number of linear iterations allowed
   */
  virtual MooseLinearConvergenceReason checkLinearConvergence(std::string & msg,
                                                              const PetscInt n,
                                                              const Real rnorm,
                                                              const Real rtol,
                                                              const Real atol,
                                                              const Real dtol,
                                                              const PetscInt maxits);

  virtual bool hasVariable(const std::string & var_name) override;
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) override;
  virtual bool hasScalarVariable(const std::string & var_name) override;
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual System & getSystem(const std::string & var_name) override;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars,
                                                THREAD_ID tid) override;

  /**
   * Get the MOOSE variables to be reinited on each element.
   *
   * @param tid The thread id
   */
  virtual const std::set<MooseVariable *> &
  getActiveElementalMooseVariables(THREAD_ID tid) override;

  /**
   * Whether or not a list of active elemental moose variables has been set.
   *
   * @return True if there has been a list of active elemental moose variables set, False otherwise
   */
  virtual bool hasActiveElementalMooseVariables(THREAD_ID tid) override;

  /**
   * Clear the active elemental MooseVariable.  If there are no active variables then they will all
   * be reinited.
   * Call this after finishing the computation that was using a restricted set of MooseVariables
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(THREAD_ID tid) override;

  /**
   * Record and set the material properties required by the current computing thread.
   * @param mat_prop_ids The set of material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual void setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids,
                                           THREAD_ID tid) override;

  /**
   * Get the material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual const std::set<unsigned int> & getActiveMaterialProperties(THREAD_ID tid) override;

  /**
   * Method to check whether or not a list of active material roperties has been set. This method
   * is called by reinitMaterials to determine whether Material computeProperties methods need to be
   * called. If the return is False, this check prevents unnecessary material property computation
   * @param tid The thread id
   *
   * @return True if there has been a list of active material properties set, False otherwise
   */
  virtual bool hasActiveMaterialProperties(THREAD_ID tid) override;

  /**
   * Clear the active material properties. Should be called at the end of every computing thread
   *
   * @param tid The thread id
   */
  virtual void clearActiveMaterialProperties(THREAD_ID tid) override;

  virtual void createQRules(QuadratureType type,
                            Order order,
                            Order volume_order = INVALID_ORDER,
                            Order face_order = INVALID_ORDER);

  /**
   * @return The maximum number of quadrature points in use on any element in this problem.
   */
  unsigned int getMaxQps() const;

  /**
   * @return The maximum number of quadrature points in use on any element in this problem.
   */
  unsigned int getMaxShapeFunctions() const;

  /**
   * @return The maximum order for all scalar variables in this problem's systems.
   */
  Order getMaxScalarOrder() const;

  /**
   * @return Flag indicating nonlocal coupling exists or not.
   */
  void checkNonlocalCoupling();
  void checkUserObjectJacobianRequirement(THREAD_ID tid);
  void setVariableAllDoFMap(const std::vector<MooseVariable *> moose_vars);

  const std::vector<MooseVariable *> & getUserObjectJacobianVariables(THREAD_ID tid) const
  {
    return _uo_jacobian_moose_vars[tid];
  }

  virtual Assembly & assembly(THREAD_ID tid) override { return *_assembly[tid]; }

  /**
   * Returns a list of all the variables in the problem (both from the NL and Aux systems.
   */
  virtual std::vector<VariableName> getVariableNames();

  /**
   * A place to add extra vectors to the simulation. It is called early during initialSetup.
   */
  virtual void addExtraVectors();
  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void prepare(const Elem * elem, THREAD_ID tid) override;
  virtual void prepareFace(const Elem * elem, THREAD_ID tid) override;
  virtual void prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       THREAD_ID tid) override;

  virtual void setCurrentSubdomainID(const Elem * elem, THREAD_ID tid) override;
  virtual void setNeighborSubdomainID(const Elem * elem, unsigned int side, THREAD_ID tid) override;
  virtual void setNeighborSubdomainID(const Elem * elem, THREAD_ID tid);
  virtual void prepareAssembly(THREAD_ID tid) override;

  virtual void addGhostedElem(dof_id_type elem_id) override;
  virtual void addGhostedBoundary(BoundaryID boundary_id) override;
  virtual void ghostGhostedBoundaries() override;

  virtual void sizeZeroes(unsigned int size, THREAD_ID tid);
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid) override;
  virtual void reinitElem(const Elem * elem, THREAD_ID tid) override;
  virtual void
  reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid) override;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitNode(const Node * node, THREAD_ID tid) override;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid) override;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid) override;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) override;
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid) override;
  virtual void reinitScalars(THREAD_ID tid) override;
  virtual void reinitOffDiagScalars(THREAD_ID tid) override;

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems) override;
  virtual void clearDiracInfo() override;

  virtual void subdomainSetup(SubdomainID subdomain, THREAD_ID tid);
  virtual void neighborSubdomainSetup(SubdomainID subdomain, THREAD_ID tid);

  virtual void newAssemblyArray(NonlinearSystemBase & nl);
  virtual void deleteAssemblyArray();
  virtual void initNullSpaceVectors(const InputParameters & parameters, NonlinearSystemBase & nl);

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache) override;

  virtual void init() override;
  virtual void solve() override;

  /**
   * Set an exception.  Usually this should not be directly called - but should be called through
   * the mooseException() macro.
   *
   * @param message The error message about the exception.
   */
  virtual void setException(const std::string & message);

  /**
   * Whether or not an exception has occurred.
   */
  virtual bool hasException() { return _has_exception; }

  /**
   * Check to see if an exception has occurred on any processor and stop the solve.
   *
   * Note: Collective on MPI!  Must be called simultaneously by all processors!
   *
   * Also: This will throw a MooseException!
   *
   * Note: DO NOT CALL THIS IN A THREADED REGION!  This is meant to be called just after a threaded
   * section.
   */
  virtual void checkExceptionAndStopSolve();

  virtual bool converged() override;
  virtual unsigned int nNonlinearIterations() override;
  virtual unsigned int nLinearIterations() override;
  virtual Real finalNonlinearResidual() override;
  virtual bool computingInitialResidual() override;

  /**
   * Returns true if we are currently computing Jacobian
   */
  virtual bool currentlyComputingJacobian() { return _currently_computing_jacobian; }

  /**
   * Returns true if we are in or beyond the initialSetup stage
   */
  virtual bool startedInitialSetup() { return _started_initial_setup; }

  virtual void onTimestepBegin() override;
  virtual void onTimestepEnd() override;

  virtual Real & time() const { return _time; }
  virtual Real & timeOld() const { return _time_old; }
  virtual int & timeStep() const { return _t_step; }
  virtual Real & dt() const { return _dt; }
  virtual Real & dtOld() const { return _dt_old; }

  virtual void transient(bool trans) { _transient = trans; }
  virtual bool isTransient() const override { return _transient; }

  virtual void
  addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters);
  virtual void
  addPredictor(const std::string & type, const std::string & name, InputParameters parameters);

  virtual void copySolutionsBackwards();

  /**
   * Advance all of the state holding vectors / datastructures so that we can move to the next
   * timestep.
   */
  virtual void advanceState();

  virtual void restoreSolutions();

  /**
   * Allocate vectors and save old solutions into them.
   */
  virtual void saveOldSolutions();

  /**
   * Restore old solutions from the backup vectors and deallocate them.
   */
  virtual void restoreOldSolutions();

  /**
   * Output the current step.
   * Will ensure that everything is in the proper state to be outputted.
   * Then tell the OutputWarehouse to do its thing
   * @param type The type execution flag (see Moose.h)
   */
  virtual void outputStep(ExecFlagType type);

  /**
   * Method called at the end of the simulation.
   */
  virtual void postExecute();

  ///@{
  /**
   * Ability to enable/disable all output calls
   *
   * This is needed by Multiapps and applications to disable output for cases when
   * executioners call other executions and when Multiapps are sub cycling.
   */
  void allowOutput(bool state);
  template <typename T>
  void allowOutput(bool state);
  ///@}

  /**
   * Indicates that the next call to outputStep should be forced
   *
   * This is needed by the MultiApp system, if forceOutput is called the next call to outputStep,
   * regardless of the type supplied to the call, will be executed with EXEC_FORCED.
   *
   * Forced output will NOT override the allowOutput flag.
   */
  void forceOutput();

  /**
   * Reinitialize petsc output for proper linear/nonlinear iteration display
   */
  void initPetscOutput();

#ifdef LIBMESH_HAVE_PETSC
  /**
   * Retrieve a writable reference the PETSc options (used by PetscSupport)
   */
  Moose::PetscSupport::PetscOptions & getPetscOptions() { return _petsc_options; }
#endif // LIBMESH_HAVE_PETSC

  // Function /////
  virtual void addFunction(std::string type, const std::string & name, InputParameters parameters);
  virtual bool hasFunction(const std::string & name, THREAD_ID tid = 0);
  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  /**
   * The following functions will enable MOOSE to have the capability to import distributions
   */
  virtual void
  addDistribution(std::string type, const std::string & name, InputParameters parameters);
  virtual Distribution & getDistribution(const std::string & name);

  /**
   * The following functions will enable MOOSE to have the capability to import Samplers
   */
  virtual void addSampler(std::string type, const std::string & name, InputParameters parameters);
  virtual Sampler & getSampler(const std::string & name, THREAD_ID tid = 0);

  // NL /////
  NonlinearSystemBase & getNonlinearSystemBase() { return *_nl; }
  const NonlinearSystemBase & getNonlinearSystemBase() const { return *_nl; }

  virtual NonlinearSystem & getNonlinearSystem();

  void addVariable(const std::string & var_name,
                   const FEType & type,
                   Real scale_factor,
                   const std::set<SubdomainID> * const active_subdomains = NULL);
  void addScalarVariable(const std::string & var_name,
                         Order order,
                         Real scale_factor = 1.,
                         const std::set<SubdomainID> * const active_subdomains = NULL);
  void
  addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addNodalKernel(const std::string & kernel_name,
                      const std::string & name,
                      InputParameters parameters);
  void addScalarKernel(const std::string & kernel_name,
                       const std::string & name,
                       InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name,
                            const std::string & name,
                            InputParameters parameters);
  void
  addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);

  virtual void setInputParametersFEProblem(InputParameters & parameters)
  {
    parameters.set<FEProblemBase *>("_fe_problem_base") = this;
  }

  // Aux /////
  void addAuxVariable(const std::string & var_name,
                      const FEType & type,
                      const std::set<SubdomainID> * const active_subdomains = NULL);
  void addAuxScalarVariable(const std::string & var_name,
                            Order order,
                            Real scale_factor = 1.,
                            const std::set<SubdomainID> * const active_subdomains = NULL);
  void addAuxKernel(const std::string & kernel_name,
                    const std::string & name,
                    InputParameters parameters);
  void addAuxScalarKernel(const std::string & kernel_name,
                          const std::string & name,
                          InputParameters parameters);

  AuxiliarySystem & getAuxiliarySystem() { return *_aux; }

  // Dirac /////
  void addDiracKernel(const std::string & kernel_name,
                      const std::string & name,
                      InputParameters parameters);

  // DG /////
  void addDGKernel(const std::string & kernel_name,
                   const std::string & name,
                   InputParameters parameters);

  // Interface /////
  void addInterfaceKernel(const std::string & kernel_name,
                          const std::string & name,
                          InputParameters parameters);

  // IC /////
  void addInitialCondition(const std::string & ic_name,
                           const std::string & name,
                           InputParameters parameters);

  void projectSolution();

  // Materials /////
  void addMaterial(const std::string & kernel_name,
                   const std::string & name,
                   InputParameters parameters);

  /**
   * Add the MooseVariables that the current materials depend on to the dependency list.
   *
   * This MUST be done after the dependency list has been set for all the other objects!
   */
  virtual void prepareMaterials(SubdomainID blk_id, THREAD_ID tid);

  virtual void reinitMaterials(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful = true);
  virtual void reinitMaterialsFace(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful = true);
  virtual void
  reinitMaterialsNeighbor(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful = true);
  virtual void
  reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid, bool swap_stateful = true);
  /*
   * Swap back underlying data storing stateful material properties
   */
  virtual void swapBackMaterials(THREAD_ID tid);
  virtual void swapBackMaterialsFace(THREAD_ID tid);
  virtual void swapBackMaterialsNeighbor(THREAD_ID tid);

  // Postprocessors /////
  virtual void
  addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters);

  // VectorPostprocessors /////
  virtual void
  addVectorPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters);

  /**
   * Initializes the postprocessor data
   * @see SetupPostprocessorDataAction
   */
  void initPostprocessorData(const std::string & name);

  // UserObjects /////
  virtual void
  addUserObject(std::string user_object_name, const std::string & name, InputParameters parameters);

  /**
   * Return the storage of all UserObjects.
   *
   * @see AdvancedOutput::initPostprocessorOrVectorPostprocessorLists
   */
  const ExecuteMooseObjectWarehouse<UserObject> & getUserObjects() { return _all_user_objects; }

  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @return Const reference to the user object
   */
  template <class T>
  const T & getUserObject(const std::string & name, unsigned int tid = 0)
  {
    if (_all_user_objects.hasActiveObject(name, tid))
    {
      auto uo_ptr = std::dynamic_pointer_cast<T>(_all_user_objects.getActiveObject(name, tid));
      if (uo_ptr == nullptr)
        mooseError("User object with name '" + name + "' is of wrong type");
      return *uo_ptr;
    }
    mooseError("Unable to find user object with name '" + name + "'");
  }
  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @return Const reference to the user object
   */
  const UserObject & getUserObjectBase(const std::string & name);

  /**
   * Check if there if a user object of given name
   * @param name The name of the user object being checked for
   * @return true if the user object exists, false otherwise
   */
  bool hasUserObject(const std::string & name);

  /**
   * Check existence of the postprocessor.
   * @param name The name of the post-processor
   * @return true if it exists, otherwise false
   */
  bool hasPostprocessor(const std::string & name);

  /**
   * Get a reference to the value associated with the postprocessor.
   */
  PostprocessorValue & getPostprocessorValue(const PostprocessorName & name);

  /**
   * Get the reference to the old value of a post-processor
   * @param name The name of the post-processor
   * @return The reference to the old value
   */
  PostprocessorValue & getPostprocessorValueOld(const std::string & name);

  /**
   * Get the reference to the older value of a post-processor
   * @param name The name of the post-processor
   * @return The reference to the old value
   */
  PostprocessorValue & getPostprocessorValueOlder(const std::string & name);

  /**
   * Returns whether or not the current simulation has any multiapps
   */
  bool hasMultiApps() const { return _multi_apps.hasActiveObjects(); }
  bool hasMultiApp(const std::string & name);

  /**
   * Check existence of the VectorPostprocessor.
   * @param name The name of the post-processor
   * @return true if it exists, otherwise false
   */
  bool hasVectorPostprocessor(const std::string & name);

  /**
   * Get a reference to the value associated with the VectorPostprocessor.
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the current value
   */
  VectorPostprocessorValue & getVectorPostprocessorValue(const VectorPostprocessorName & name,
                                                         const std::string & vector_name);

  /**
   * Get the reference to the old value of a post-processor
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the old value
   */
  VectorPostprocessorValue & getVectorPostprocessorValueOld(const std::string & name,
                                                            const std::string & vector_name);

  /**
   * Declare a new VectorPostprocessor vector
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the vector declared
   */
  VectorPostprocessorValue & declareVectorPostprocessorVector(const VectorPostprocessorName & name,
                                                              const std::string & vector_name);

  /**
   * Whether or not the specified VectorPostprocessor has declared any vectors
   */
  bool vectorPostprocessorHasVectors(const std::string & vpp_name)
  {
    return _vpps_data.hasVectors(vpp_name);
  }

  /**
   * Get the vectors for a specific VectorPostprocessor.
   * @param vpp_name The name of the VectorPostprocessor
   */
  const std::vector<std::pair<std::string, VectorPostprocessorData::VectorPostprocessorState>> &
  getVectorPostprocessorVectors(const std::string & vpp_name);

  // Dampers /////
  void addDamper(std::string damper_name, const std::string & name, InputParameters parameters);
  void setupDampers();

  /**
   * Whether or not this system has dampers.
   */
  bool hasDampers() { return _has_dampers; }

  // Indicators /////
  void
  addIndicator(std::string indicator_name, const std::string & name, InputParameters parameters);

  // Markers //////
  void addMarker(std::string marker_name, const std::string & name, InputParameters parameters);

  /**
   * Add a MultiApp to the problem.
   */
  void addMultiApp(const std::string & multi_app_name,
                   const std::string & name,
                   InputParameters parameters);

  /**
   * Get a MultiApp object by name.
   */
  std::shared_ptr<MultiApp> getMultiApp(const std::string & multi_app_name);

  /**
   * Get Transfers by ExecFlagType and direction
   */
  std::vector<std::shared_ptr<Transfer>> getTransfers(ExecFlagType type,
                                                      MultiAppTransfer::DIRECTION direction) const;

  /**
   * Execute MultiAppTransfers associate with execution flag and direction.
   * @param type The execution flag to execute.
   * @param direction The direction (to or from) to transfer.
   */
  void execMultiAppTransfers(ExecFlagType type, MultiAppTransfer::DIRECTION direction);

  /**
   * Execute the MultiApps associated with the ExecFlagType
   */
  bool execMultiApps(ExecFlagType type, bool auto_advance = true);

  /**
   * Advance the MultiApps associated with the ExecFlagType
   */
  void advanceMultiApps(ExecFlagType type);

  /**
   * Backup the MultiApps associated with the ExecFlagType
   */
  void backupMultiApps(ExecFlagType type);

  /**
   * Restore the MultiApps associated with the ExecFlagType
   * @param force Force restoration because something went wrong with the solve
   */
  void restoreMultiApps(ExecFlagType type, bool force = false);

  /**
   * Find the smallest timestep over all MultiApps
   */
  Real computeMultiAppsDT(ExecFlagType type);

  /**
   * Add a Transfer to the problem.
   */
  void addTransfer(const std::string & transfer_name,
                   const std::string & name,
                   InputParameters parameters);

  /**
   * Execute the Transfers associated with the ExecFlagType
   *
   * Note: This does _not_ execute MultiApp Transfers!
   * Those are executed automatically when MultiApps are executed.
   */
  void execTransfers(ExecFlagType type);

  /// Evaluates transient residual G in canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitResidual(Real time,
                                        const NumericVector<Number> & u,
                                        const NumericVector<Number> & udot,
                                        NumericVector<Number> & residual);

  /// Evaluates transient Jacobian J_a = dG/dU + a*dG/dUdot from canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitJacobian(Real time,
                                        const NumericVector<Number> & u,
                                        const NumericVector<Number> & udot,
                                        Real shift,
                                        SparseMatrix<Number> & jacobian);

  ////

  /**
   * Computes the residual using whatever is sitting in the current solution vector then returns the
   * L2 norm.
   *
   * @return The L2 norm of the residual
   */
  virtual Real computeResidualL2Norm();

  virtual void computeResidual(NonlinearImplicitSystem & sys,
                               const NumericVector<Number> & soln,
                               NumericVector<Number> & residual);
  virtual void computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual);
  virtual void computeResidualType(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual,
                                   Moose::KernelType type = Moose::KT_ALL);
  virtual void computeJacobian(NonlinearImplicitSystem & sys,
                               const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian);
  virtual void computeJacobian(const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian,
                               Moose::KernelType kernel_type = Moose::KT_ALL);
  /**
   * Computes several Jacobian blocks simultaneously, summing their contributions into smaller
   * preconditioning matrices.
   *
   * Used by Physics-based preconditioning
   *
   * @param blocks The blocks to fill in (JacobianBlock is defined in ComputeJacobianBlocksThread)
   */
  virtual void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks);

  /**
   * Really not a good idea to use this.
   *
   * It computes just one block of the Jacobian into a smaller matrix.  Calling this in a loop is
   * EXTREMELY ineffecient!
   * Try to use computeJacobianBlocks() instead!
   *
   * @param jacobian The matrix you want to fill
   * @param precond_system The libMesh::system of the preconditioning system
   * @param ivar the block-row of the Jacobian
   * @param jvar the block-column of the Jacobian
   *
   */
  virtual void computeJacobianBlock(SparseMatrix<Number> & jacobian,
                                    libMesh::System & precond_system,
                                    unsigned int ivar,
                                    unsigned int jvar);

  virtual Real computeDamping(const NumericVector<Number> & soln,
                              const NumericVector<Number> & update);

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
  virtual bool updateSolution(NumericVector<Number> & vec_solution,
                              NumericVector<Number> & ghosted_solution);

  /**
   * Perform cleanup tasks after application of predictor to solution vector
   * @param ghosted_solution  Ghosted solution vector
   */
  virtual void predictorCleanup(NumericVector<Number> & ghosted_solution);

  virtual void computeBounds(NonlinearImplicitSystem & sys,
                             NumericVector<Number> & lower,
                             NumericVector<Number> & upper);
  virtual void computeNearNullSpace(NonlinearImplicitSystem & sys,
                                    std::vector<NumericVector<Number> *> & sp);
  virtual void computeNullSpace(NonlinearImplicitSystem & sys,
                                std::vector<NumericVector<Number> *> & sp);
  virtual void computeTransposeNullSpace(NonlinearImplicitSystem & sys,
                                         std::vector<NumericVector<Number> *> & sp);
  virtual void computePostCheck(NonlinearImplicitSystem & sys,
                                const NumericVector<Number> & old_soln,
                                NumericVector<Number> & search_direction,
                                NumericVector<Number> & new_soln,
                                bool & changed_search_direction,
                                bool & changed_new_soln);

  virtual void computeIndicatorsAndMarkers();
  virtual void computeIndicators();
  virtual void computeMarkers();

  virtual NumericVector<Number> & residualVector(Moose::KernelType type);

  virtual void addResidual(THREAD_ID tid) override;
  virtual void addResidualNeighbor(THREAD_ID tid) override;
  virtual void addResidualScalar(THREAD_ID tid = 0);

  virtual void cacheResidual(THREAD_ID tid) override;
  virtual void cacheResidualNeighbor(THREAD_ID tid) override;
  virtual void addCachedResidual(THREAD_ID tid) override;

  /**
   * Allows for all the residual contributions that are currently cached to be added directly into
   * the vector passed in.
   *
   * @param residual The vector to add the cached contributions to.
   * @param tid The thread id.
   */
  virtual void addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid) override;
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) override;

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) override;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid) override;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid) override;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   THREAD_ID tid) override;
  virtual void addJacobianScalar(SparseMatrix<Number> & jacobian, THREAD_ID tid = 0);
  virtual void
  addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian, unsigned int ivar, THREAD_ID tid = 0);

  virtual void cacheJacobian(THREAD_ID tid) override;
  virtual void cacheJacobianNeighbor(THREAD_ID tid) override;
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) override;

  virtual void prepareShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) override;

  // Displaced problem /////
  virtual void addDisplacedProblem(std::shared_ptr<DisplacedProblem> displaced_problem);
  virtual std::shared_ptr<DisplacedProblem> getDisplacedProblem() { return _displaced_problem; }

  virtual void updateGeomSearch(
      GeometricSearchData::GeometricSearchType type = GeometricSearchData::ALL) override;

  virtual void possiblyRebuildGeomSearchPatches();

  virtual GeometricSearchData & geomSearchData() override { return _geometric_search_data; }

  /**
   * Communicate to the Resurector the name of the restart filer
   * @param file_name The file name for restarting from
   */
  void setRestartFile(const std::string & file_name);

  ///@{
  /**
   * Return a reference to the material property storage
   * @return A const reference to the material property storage
   */
  const MaterialPropertyStorage & getMaterialPropertyStorage() { return _material_props; }
  const MaterialPropertyStorage & getBndMaterialPropertyStorage() { return _bnd_material_props; }
  ///@}

  ///@{
  /**
   * Return indicator/marker storage.
   */
  const MooseObjectWarehouse<Indicator> & getIndicatorWarehouse() { return _indicators; }
  const MooseObjectWarehouse<InternalSideIndicator> & getInternalSideIndicatorWarehouse()
  {
    return _internal_side_indicators;
  }
  const MooseObjectWarehouse<Marker> & getMarkerWarehouse() { return _markers; }
  ///@}

  /**
   * Return InitialCondition storage
   */
  const InitialConditionWarehouse & getInitialConditionWarehouse() const { return _ics; }

  /**
   * Get the solver parameters
   */
  SolverParams & solverParams();

#ifdef LIBMESH_ENABLE_AMR
  // Adaptivity /////
  Adaptivity & adaptivity() { return _adaptivity; }
  virtual void initialAdaptMesh();
  virtual void adaptMesh();
  /**
   * @return The number of adaptivity cycles completed.
   */
  unsigned int getNumCyclesCompleted() { return _cycles_completed; }

  /**
   * Return a Boolean indicating whether initial AMR is turned on.
   */
  bool hasInitialAdaptivity() const { return _adaptivity.getInitialSteps() > 0; }
#else
  /**
   * Return a Boolean indicating whether initial AMR is turned on.
   */
  bool hasInitialAdaptivity() const { return false; }
#endif // LIBMESH_ENABLE_AMR

  /// Create XFEM controller object
  void initXFEM(std::shared_ptr<XFEMInterface> xfem);

  /// Get a pointer to the XFEM controller object
  std::shared_ptr<XFEMInterface> getXFEM() { return _xfem; }

  /// Find out whether the current analysis is using XFEM
  bool haveXFEM() { return _xfem != NULL; }

  /// Update the mesh due to changing XFEM cuts
  virtual bool updateMeshXFEM();

  virtual void meshChanged() override;

  virtual bool haveAugLM();

  virtual bool updateLagMul();

  virtual void initLagMul();

  /**
   * Register an object that derives from MeshChangedInterface
   * to be notified when the mesh changes.
   */
  void notifyWhenMeshChanges(MeshChangedInterface * mci);

  virtual void checkProblemIntegrity();

  void serializeSolution();

  // debugging iface /////

  void setKernelTypeResidual(Moose::KernelType kt) { _kernel_type = kt; }

  /**
   * Set flag that Jacobian is constant (for optimization purposes)
   * @param state True if the Jacobian is constant, false otherwise
   */
  void setConstJacobian(bool state) { _const_jacobian = state; }

  void registerRandomInterface(RandomInterface & random_interface, const std::string & name);

  void setKernelCoverageCheck(bool flag) { _kernel_coverage_check = flag; }

  void setMaterialCoverageCheck(bool flag) { _material_coverage_check = flag; }

  /**
   * Calls parentOutputPositionChanged() on all sub apps.
   */
  void parentOutputPositionChanged();

  ///@{
  /**
   * These methods are used to determine whether stateful material properties need to be stored on
   * internal sides.  There are four situations where this may be the case: 1) DGKernels
   * 2) IntegratedBCs 3)InternalSideUserObjects 4)ElementalAuxBCs
   *
   * Method 1:
   * @param bnd_id the boundary id for which to see if stateful material properties need to be
   * stored
   * @param tid the THREAD_ID of the caller
   * @return Boolean indicating whether material properties need to be stored
   *
   * Method 2:
   * @param subdomain_id the subdomain id for which to see if stateful material properties need to
   * be stored
   * @param tid the THREAD_ID of the caller
   * @return Boolean indicating whether material properties need to be stored
   */
  bool needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid);
  bool needMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid);
  ///@}

  /**
   * Dimension of the subspace spanned by vectors with a given prefix.
   * @param prefix Prefix of the vectors spanning the subspace.
   */
  unsigned int subspaceDim(const std::string & prefix) const
  {
    if (_subspace_dim.count(prefix))
      return _subspace_dim.find(prefix)->second;
    else
      return 0;
  }

  /*
   * Return a reference to the material warehouse of *all* Material objects.
   */
  const MaterialWarehouse & getMaterialWarehouse() { return _all_materials; }

  /*
   * Return a reference to the material warehouse of Material objects to be computed.
   */
  const MaterialWarehouse & getComputeMaterialWarehouse() { return _materials; }
  const MaterialWarehouse & getDiscreteMaterialWarehouse() { return _discrete_materials; }

  /**
   * Return a pointer to a Material object.  If no_warn is true, suppress
   * warning about retrieving a material reference potentially during the
   * material's calculation.
   *
   * This will return enabled or disabled objects, the main purpose is for iterative materials.
   */
  std::shared_ptr<Material> getMaterial(std::string name,
                                        Moose::MaterialDataType type,
                                        THREAD_ID tid = 0,
                                        bool no_warn = false);

  /*
   * Return a pointer to the MaterialData
   */
  std::shared_ptr<MaterialData> getMaterialData(Moose::MaterialDataType type, THREAD_ID tid = 0);

  /**
   * Will return True if the user wants to get an error when
   * a nonzero is reallocated in the Jacobian by PETSc
   */
  bool errorOnJacobianNonzeroReallocation() { return _error_on_jacobian_nonzero_reallocation; }

  void setErrorOnJacobianNonzeroReallocation(bool state)
  {
    _error_on_jacobian_nonzero_reallocation = state;
  }

  bool ignoreZerosInJacobian() { return _ignore_zeros_in_jacobian; }

  void setIgnoreZerosInJacobian(bool state) { _ignore_zeros_in_jacobian = state; }

  /// Returns whether or not this Problem has a TimeIntegrator
  bool hasTimeIntegrator() const { return _has_time_integrator; }

  /**
   * Return the current execution flag.
   *
   * Returns EXEC_NONE when not being executed.
   * @see FEProblemBase::execute
   */
  const ExecFlagType & getCurrentExecuteOnFlag() const;

  /**
   * Convenience function for performing execution of MOOSE systems.
   */
  void execute(const ExecFlagType & exec_type);

  /**
   * Call compute methods on UserObjects.
   */
  virtual void computeUserObjects(const ExecFlagType & type, const Moose::AuxGroup & group);
  template <typename T>
  void initializeUserObjects(const MooseObjectWarehouse<T> & warehouse);
  template <typename T>
  void finalizeUserObjects(const MooseObjectWarehouse<T> & warehouse);

  /**
   * Call compute methods on AuxKernels
   */
  virtual void computeAuxiliaryKernels(const ExecFlagType & type);

  /**
   * Set a flag that indicated that user required values for the previous Newton iterate
   */
  void needsPreviousNewtonIteration(bool state);

  /**
   * Check to see whether we need to compute the variable values of the previous Newton iterate
   * @return true if the user required values of the previous Newton iterate
   */
  bool needsPreviousNewtonIteration();

  /**
   * Whether or not to skip loading the additional data when restarting
   */
  bool skipAdditionalRestartData() const { return _skip_additional_restart_data; }

public:
  ///@{
  /**
   * Convenience zeros
   * @see ZeroInterface
   */
  std::vector<Real> _real_zero;
  std::vector<VariableValue> _zero;
  std::vector<VariableGradient> _grad_zero;
  std::vector<VariableSecond> _second_zero;
  std::vector<VariablePhiSecond> _second_phi_zero;
  ///@}

  /**
   * Reference to the control logic warehouse.
   */
  ExecuteMooseObjectWarehouse<Control> & getControlWarehouse() { return _control_warehouse; }

  /**
   * Performs setup and execute calls for Control objects.
   */
  void executeControls(const ExecFlagType & exec_type);

  /**
   * Performs setup and execute calls for Sampler objects.
   */
  void executeSamplers(const ExecFlagType & exec_type);

  /**
   * Update the active objects in the warehouses
   */
  void updateActiveObjects();

  /**
   * Register a MOOSE object dependency so we can either order
   * operations properly or report when we cannot.
   * a -> b (a depends on b)
   */
  void reportMooseObjectDependency(MooseObject * a, MooseObject * b);

  ExecuteMooseObjectWarehouse<MultiApp> & getMultiAppWarehouse() { return _multi_apps; }

protected:
  ///@{
  /**
   *
   */
  VectorPostprocessorData & getVectorPostprocessorData();
  ///@}

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

  std::shared_ptr<NonlinearSystemBase> _nl;
  std::shared_ptr<AuxiliarySystem> _aux;

  Moose::CouplingType _coupling;       ///< Type of variable coupling
  std::unique_ptr<CouplingMatrix> _cm; ///< Coupling matrix for variables.

  // Dimension of the subspace spanned by the vectors with a given prefix
  std::map<std::string, unsigned int> _subspace_dim;

  std::vector<Assembly *> _assembly;

  /// functions
  MooseObjectWarehouse<Function> _functions;

  /// distributions
  MooseObjectWarehouseBase<Distribution> _distributions;

  /// Samplers
  ExecuteMooseObjectWarehouse<Sampler> _samplers;

  /// nonlocal kernels
  MooseObjectWarehouse<KernelBase> _nonlocal_kernels;

  /// nonlocal integrated_bcs
  MooseObjectWarehouse<IntegratedBC> _nonlocal_integrated_bcs;

  ///@{
  /// Initial condition storage
  InitialConditionWarehouse _ics;
  MooseObjectWarehouseBase<ScalarInitialCondition> _scalar_ics; // use base b/c of setup methods
  ///@}

  // material properties
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;

  std::vector<std::shared_ptr<MaterialData>> _material_data;
  std::vector<std::shared_ptr<MaterialData>> _bnd_material_data;
  std::vector<std::shared_ptr<MaterialData>> _neighbor_material_data;

  ///@{
  // Material Warehouses
  MaterialWarehouse _materials;          // Traditional materials that MOOSE computes
  MaterialWarehouse _discrete_materials; // Materials that the user must compute
  MaterialWarehouse _all_materials; // All materials for error checking and MaterialData storage
  ///@}

  ///@{
  // Indicator Warehouses
  MooseObjectWarehouse<Indicator> _indicators;
  MooseObjectWarehouse<InternalSideIndicator> _internal_side_indicators;
  ///@}

  // Marker Warehouse
  MooseObjectWarehouse<Marker> _markers;

  // postprocessors
  PostprocessorData _pps_data;

  // VectorPostprocessors
  VectorPostprocessorData _vpps_data;

  ///@{
  /// Storage for UserObjects
  ExecuteMooseObjectWarehouse<UserObject> _all_user_objects;
  AuxGroupExecuteMooseObjectWarehouse<GeneralUserObject> _general_user_objects;
  AuxGroupExecuteMooseObjectWarehouse<NodalUserObject> _nodal_user_objects;
  AuxGroupExecuteMooseObjectWarehouse<ElementUserObject> _elemental_user_objects;
  AuxGroupExecuteMooseObjectWarehouse<SideUserObject> _side_user_objects;
  AuxGroupExecuteMooseObjectWarehouse<InternalSideUserObject> _internal_side_user_objects;
  ///@}

  /// MultiApp Warehouse
  ExecuteMooseObjectWarehouse<MultiApp> _multi_apps;

  /// Storage for TransientMultiApps (only needed for calling 'computeDT')
  ExecuteMooseObjectWarehouse<TransientMultiApp> _transient_multi_apps;

  /// Normal Transfers
  ExecuteMooseObjectWarehouse<Transfer> _transfers;

  /// Transfers executed just before MultiApps to transfer data to them
  ExecuteMooseObjectWarehouse<Transfer> _to_multi_app_transfers;

  /// Transfers executed just after MultiApps to transfer data from them
  ExecuteMooseObjectWarehouse<Transfer> _from_multi_app_transfers;

  /// A map of objects that consume random numbers
  std::map<std::string, std::unique_ptr<RandomData>> _random_data_objects;

  /// Cache for calculating materials on side
  std::vector<std::unordered_map<SubdomainID, bool>> _block_mat_side_cache;

  /// Cache for calculating materials on side
  std::vector<std::unordered_map<BoundaryID, bool>> _bnd_mat_side_cache;

  /// Objects to be notified when the mesh changes
  std::vector<MeshChangedInterface *> _notify_when_mesh_changes;

  /// Helper to check for duplicate variable names across systems or within a single system
  bool duplicateVariableCheck(const std::string & var_name, const FEType & type, bool is_aux);

  /// Verify that SECOND order mesh uses SECOND order displacements.
  void checkDisplacementOrders();

  void checkUserObjects();

  /**
   * Helper method for checking Material object dependency.
   *
   * @see checkProblemIntegrity
   */
  void checkDependMaterialsHelper(
      const std::map<SubdomainID, std::vector<std::shared_ptr<Material>>> & materials_map);

  /// Verify that there are no element type/coordinate type conflicts
  void checkCoordinateSystems();

  /**
   * Call when it is possible that the needs for ghosted elements has changed.
   */
  void reinitBecauseOfGhostingOrNewGeomObjects();

#ifdef LIBMESH_ENABLE_AMR
  Adaptivity _adaptivity;
  unsigned int _cycles_completed;
#endif

  /// Pointer to XFEM controller
  std::shared_ptr<XFEMInterface> _xfem;

  // Displaced mesh /////
  MooseMesh * _displaced_mesh;
  std::shared_ptr<DisplacedProblem> _displaced_problem;
  GeometricSearchData _geometric_search_data;

  bool _reinit_displaced_elem;
  bool _reinit_displaced_face;

  /// whether input file has been written
  bool _input_file_saved;

  /// Whether or not this system has any Dampers associated with it.
  bool _has_dampers;

  /// Whether or not this system has any Constraints.
  bool _has_constraints;

  /// Whether nor not stateful materials have been initialized
  bool _has_initialized_stateful;

  /// Object responsible for restart (read/write)
  std::unique_ptr<Resurrector> _resurrector;

  /// true if the Jacobian is constant
  bool _const_jacobian;

  /// Indicates if the Jacobian was computed
  bool _has_jacobian;

  /// Indicates that we need to compute variable values for previous Newton iteration
  bool _needs_old_newton_iter;

  /// Indicates if nonlocal coupling is required/exists
  bool _has_nonlocal_coupling;
  bool _calculate_jacobian_in_uo;

  std::vector<std::vector<MooseVariable *>> _uo_jacobian_moose_vars;

  SolverParams _solver_params;

  /// Determines whether a check to verify an active kernel on every subdomain
  bool _kernel_coverage_check;

  /// Determines whether a check to verify an active material on every subdomain
  bool _material_coverage_check;

  /// Maximum number of quadrature points used in the problem
  unsigned int _max_qps;

  /// Maximum number of shape functions on any element in the problem
  unsigned int _max_shape_funcs;

  /// Maximum scalar variable order
  Order _max_scalar_order;

  /// Indicates whether or not this executioner has a time integrator (during setup)
  bool _has_time_integrator;

  /// Whether or not an exception has occurred
  bool _has_exception;

  /// The error message to go with an exception
  std::string _exception_message;

  /// Current execute_on flag
  ExecFlagType _current_execute_on_flag;

  /// The control logic warehouse
  ExecuteMooseObjectWarehouse<Control> _control_warehouse;

#ifdef LIBMESH_HAVE_PETSC
  /// PETSc option storage
  Moose::PetscSupport::PetscOptions _petsc_options;
#endif // LIBMESH_HAVE_PETSC

private:
  bool _error_on_jacobian_nonzero_reallocation;
  bool _ignore_zeros_in_jacobian;
  bool _force_restart;
  bool _skip_additional_restart_data;
  bool _fail_next_linear_convergence_check;

  /// Whether or not the system is currently computing the Jacobian matrix
  bool _currently_computing_jacobian;

  /// At or beyond initialSteup stage
  bool _started_initial_setup;

  friend class AuxiliarySystem;
  friend class NonlinearSystemBase;
  friend class MooseEigenSystem;
  friend class Resurrector;
  friend class RestartableDataIO;
  friend class Restartable;
  friend class DisplacedProblem;
};

template <typename T>
void
FEProblemBase::allowOutput(bool state)
{
  _app.getOutputWarehouse().allowOutput<T>(state);
}

template <typename T>
void
FEProblemBase::initializeUserObjects(const MooseObjectWarehouse<T> & warehouse)
{
  if (warehouse.hasActiveObjects())
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      const auto & objects = warehouse.getActiveObjects(tid);
      for (const auto & object : objects)
        object->initialize();
    }
  }
}

template <typename T>
void
FEProblemBase::finalizeUserObjects(const MooseObjectWarehouse<T> & warehouse)
{
  if (warehouse.hasActiveObjects())
  {
    const auto & objects = warehouse.getActiveObjects(0);

    // Join them down to processor 0
    for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
    {
      const auto & other_objects = warehouse.getActiveObjects(tid);

      for (unsigned int i = 0; i < objects.size(); ++i)
        objects[i]->threadJoin(*(other_objects[i]));
    }

    // Finalize them and save off PP values
    for (auto & object : objects)
    {
      object->finalize();

      auto pp = std::dynamic_pointer_cast<Postprocessor>(object);

      if (pp)
        _pps_data.storeValue(pp->PPName(), pp->getValue());
    }
  }
}

#endif /* FEPROBLEMBASE_H */
