//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "MortarData.h"
#include "PostprocessorData.h"
#include "VectorPostprocessorData.h"
#include "Adaptivity.h"
#include "InitialConditionWarehouse.h"
#include "ScalarInitialConditionWarehouse.h"
#include "Restartable.h"
#include "SolverParams.h"
#include "PetscSupport.h"
#include "MooseApp.h"
#include "ExecuteMooseObjectWarehouse.h"
#include "MaterialWarehouse.h"
#include "MooseVariableFE.h"
#include "MultiAppTransfer.h"
#include "Postprocessor.h"
#include "HashMap.h"
#include "VectorPostprocessor.h"
#include "PerfGraphInterface.h"
#include "Attributes.h"

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
class RestartableDataIO;
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
class InterfaceUserObject;
class GeneralUserObject;
class Function;
class Distribution;
class Sampler;
class KernelBase;
class IntegratedBCBase;
class LineSearch;
class UserObject;
class AutomaticMortarGeneration;

// libMesh forward declarations
namespace libMesh
{
class CouplingMatrix;
class NonlinearImplicitSystem;
} // namespace libMesh

template <>
InputParameters validParams<FEProblemBase>();

/// Enumeration for nonlinear convergence reasons
enum class MooseNonlinearConvergenceReason
{
  ITERATING = 0,
  CONVERGED_FNORM_ABS = 2,
  CONVERGED_FNORM_RELATIVE = 3,
  CONVERGED_SNORM_RELATIVE = 4,
  DIVERGED_FUNCTION_COUNT = -2,
  DIVERGED_FNORM_NAN = -4,
  DIVERGED_LINE_SEARCH = -6,
  DIVERGED_DTOL = -9
};

// The idea with these enums is to abstract the reasons for
// convergence/divergence, i.e. they could be used with linear algebra
// packages other than PETSc.  They were directly inspired by PETSc,
// though.  This enum could also be combined with the
// MooseNonlinearConvergenceReason enum but there might be some
// confusion (?)
enum class MooseLinearConvergenceReason
{
  ITERATING = 0,
  // CONVERGED_RTOL_NORMAL        =  1,
  // CONVERGED_ATOL_NORMAL        =  9,
  CONVERGED_RTOL = 2,
  CONVERGED_ATOL = 3,
  CONVERGED_ITS = 4,
  // CONVERGED_CG_NEG_CURVE       =  5,
  // CONVERGED_CG_CONSTRAINED     =  6,
  // CONVERGED_STEP_LENGTH        =  7,
  // CONVERGED_HAPPY_BREAKDOWN    =  8,
  DIVERGED_NULL = -2,
  // DIVERGED_ITS                 = -3,
  // DIVERGED_DTOL                = -4,
  // DIVERGED_BREAKDOWN           = -5,
  // DIVERGED_BREAKDOWN_BICG      = -6,
  // DIVERGED_NONSYMMETRIC        = -7,
  // DIVERGED_INDEFINITE_PC       = -8,
  DIVERGED_NANORINF = -9,
  // DIVERGED_INDEFINITE_MAT      = -10
  DIVERGED_PCSETUP_FAILED = -11
};

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class FEProblemBase : public SubProblem, public Restartable
{
public:
  static InputParameters validParams();

  FEProblemBase(const InputParameters & parameters);
  virtual ~FEProblemBase();

  virtual EquationSystems & es() override { return _eq; }
  virtual MooseMesh & mesh() override { return _mesh; }
  virtual const MooseMesh & mesh() const override { return _mesh; }

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

  const CouplingMatrix * couplingMatrix() const override { return _cm.get(); }

  /// Set custom coupling matrix for variables requiring nonlocal contribution
  void setNonlocalCouplingMatrix();

  bool areCoupled(unsigned int ivar, unsigned int jvar);

  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> &
  couplingEntries(THREAD_ID tid);
  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> &
  nonlocalCouplingEntries(THREAD_ID tid);

  /**
   * Check for converence of the nonlinear solution
   * @param msg            Error message that gets sent back to the solver
   * @param it             Iteration counter
   * @param xnorm          Norm of the solution vector
   * @param snorm          Norm of the change in the solution vector
   * @param fnorm          Norm of the residual vector
   * @param rtol           Relative residual convergence tolerance
   * @param divtol           Relative residual divergence tolerance
   * @param stol           Solution change convergence tolerance
   * @param abstol         Absolute residual convergence tolerance
   * @param nfuncs         Number of function evaluations
   * @param max_funcs      Maximum Number of function evaluations
   * @param initial_residual_before_preset_bcs      Residual norm prior to imposition of preset BCs
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
                            const Real divtol,
                            const Real stol,
                            const Real abstol,
                            const PetscInt nfuncs,
                            const PetscInt max_funcs,
                            const PetscBool force_iteration,
                            const Real initial_residual_before_preset_bcs,
                            const Real div_threshold);

  virtual bool hasVariable(const std::string & var_name) const override;
  virtual MooseVariableFEBase & getVariable(
      THREAD_ID tid,
      const std::string & var_name,
      Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
      Moose::VarFieldType expected_var_field_type = Moose::VarFieldType::VAR_FIELD_ANY) override;
  virtual MooseVariable & getStandardVariable(THREAD_ID tid, const std::string & var_name) override;
  virtual VectorMooseVariable & getVectorVariable(THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual ArrayMooseVariable & getArrayVariable(THREAD_ID tid,
                                                const std::string & var_name) override;

  virtual bool hasScalarVariable(const std::string & var_name) const override;
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual System & getSystem(const std::string & var_name) override;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void setActiveElementalMooseVariables(const std::set<MooseVariableFEBase *> & moose_vars,
                                                THREAD_ID tid) override;

  /**
   * Clear the active elemental MooseVariableFEBase.  If there are no active variables then they
   * will all be reinited. Call this after finishing the computation that was using a restricted set
   * of MooseVariableFEBases
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(THREAD_ID tid) override;

  virtual void clearActiveFEVariableCoupleableMatrixTags(THREAD_ID tid) override;

  virtual void clearActiveFEVariableCoupleableVectorTags(THREAD_ID tid) override;

  virtual void setActiveFEVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                       THREAD_ID tid) override;

  virtual void setActiveFEVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                       THREAD_ID tid) override;

  virtual void clearActiveScalarVariableCoupleableMatrixTags(THREAD_ID tid) override;

  virtual void clearActiveScalarVariableCoupleableVectorTags(THREAD_ID tid) override;

  virtual void setActiveScalarVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                           THREAD_ID tid) override;

  virtual void setActiveScalarVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                           THREAD_ID tid) override;

  /**
   * Record and set the material properties required by the current computing thread.
   * @param mat_prop_ids The set of material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual void setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids,
                                           THREAD_ID tid) override;

  /**
   * Clear the active material properties. Should be called at the end of every computing thread
   *
   * @param tid The thread id
   */
  virtual void clearActiveMaterialProperties(THREAD_ID tid) override;

  virtual void createQRules(QuadratureType type,
                            Order order,
                            Order volume_order = INVALID_ORDER,
                            Order face_order = INVALID_ORDER,
                            SubdomainID block = Moose::ANY_BLOCK_ID);

  /**
   * Increases the elemennt/volume quadrature order for the specified mesh
   * block if and only if the current volume quadrature order is lower.  This
   * can only cause the quadrature level to increase.  If volume_order is
   * lower than or equal to the current volume/elem quadrature rule order,
   * then nothing is done (i.e. this function is idempotent).
   */
  void bumpVolumeQRuleOrder(Order order, SubdomainID block);

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
  void setVariableAllDoFMap(const std::vector<const MooseVariableFEBase *> & moose_vars);

  const std::vector<const MooseVariableFEBase *> &
  getUserObjectJacobianVariables(THREAD_ID tid) const
  {
    return _uo_jacobian_moose_vars[tid];
  }

  Assembly & assembly(THREAD_ID tid) override
  {
    mooseAssert(tid < _assembly.size(), "Assembly objects not initialized");
    return *_assembly[tid];
  }
  const Assembly & assembly(THREAD_ID tid) const override
  {
    mooseAssert(tid < _assembly.size(), "Assembly objects not initialized");
    return *_assembly[tid];
  }

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
  virtual void reinitElemPhys(const Elem * elem,
                              const std::vector<Point> & phys_points_in_elem,
                              THREAD_ID tid,
                              bool suppress_displaced_init = false) override;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitLowerDElem(const Elem * lower_d_elem,
                                THREAD_ID tid,
                                const std::vector<Point> * const pts = nullptr,
                                const std::vector<Real> * const weights = nullptr) override;
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
  virtual void reinitScalars(THREAD_ID tid, bool reinit_for_derivative_reordering = false) override;
  virtual void reinitOffDiagScalars(THREAD_ID tid) override;

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems) override;
  virtual void clearDiracInfo() override;

  virtual void subdomainSetup(SubdomainID subdomain, THREAD_ID tid);
  virtual void neighborSubdomainSetup(SubdomainID subdomain, THREAD_ID tid);

  virtual void newAssemblyArray(NonlinearSystemBase & nl);
  virtual void initNullSpaceVectors(const InputParameters & parameters, NonlinearSystemBase & nl);

  virtual void init() override;
  virtual void solve() override;

  const ConstElemRange & getEvaluableElementRange();

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
  virtual void checkExceptionAndStopSolve(bool print_message = true);

  virtual bool converged() override;
  virtual unsigned int nNonlinearIterations() const override;
  virtual unsigned int nLinearIterations() const override;
  virtual Real finalNonlinearResidual() const override;
  virtual bool computingInitialResidual() const override;

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

  virtual void addTimeIntegrator(const std::string & type,
                                 const std::string & name,
                                 InputParameters & parameters);
  virtual void
  addPredictor(const std::string & type, const std::string & name, InputParameters & parameters);

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
  virtual void
  addFunction(const std::string & type, const std::string & name, InputParameters & parameters);
  virtual bool hasFunction(const std::string & name, THREAD_ID tid = 0);
  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  /**
   * add a MOOSE line search
   */
  virtual void addLineSearch(const InputParameters & /*parameters*/)
  {
    mooseError("Line search not implemented for this problem type yet.");
  }

  /**
   * execute MOOSE line search
   */
  virtual void lineSearch();

  /**
   * getter for the MOOSE line search
   */
  LineSearch * getLineSearch() override { return _line_search.get(); }

  /**
   * The following functions will enable MOOSE to have the capability to import distributions
   */
  virtual void
  addDistribution(const std::string & type, const std::string & name, InputParameters & parameters);
  virtual Distribution & getDistribution(const std::string & name);

  /**
   * The following functions will enable MOOSE to have the capability to import Samplers
   */
  virtual void
  addSampler(const std::string & type, const std::string & name, InputParameters & parameters);
  virtual Sampler & getSampler(const std::string & name, THREAD_ID tid = 0);

  // NL /////
  NonlinearSystemBase & getNonlinearSystemBase() { return *_nl; }
  const NonlinearSystemBase & getNonlinearSystemBase() const { return *_nl; }

  virtual const SystemBase & systemBaseNonlinear() const override;
  virtual SystemBase & systemBaseNonlinear() override;

  virtual const SystemBase & systemBaseAuxiliary() const override;
  virtual SystemBase & systemBaseAuxiliary() override;

  virtual NonlinearSystem & getNonlinearSystem();

  /**
   * Canonical method for adding a non-linear variable
   * @param var_type the type of the variable, e.g. MooseVariableScalar
   * @param var_name the variable name, e.g. 'u'
   * @param params the InputParameters from which to construct the variable
   */
  virtual void
  addVariable(const std::string & var_type, const std::string & var_name, InputParameters & params);

  virtual void addVariable(const std::string & var_name,
                           const FEType & type,
                           Real scale_factor,
                           const std::set<SubdomainID> * const active_subdomains = nullptr);
  virtual void addArrayVariable(const std::string & var_name,
                                const FEType & type,
                                unsigned int components,
                                const std::vector<Real> & scale_factor,
                                const std::set<SubdomainID> * const active_subdomains = nullptr);
  virtual void addScalarVariable(const std::string & var_name,
                                 Order order,
                                 Real scale_factor = 1.,
                                 const std::set<SubdomainID> * const active_subdomains = nullptr);

  virtual void addKernel(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters);
  virtual void addNodalKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters);
  virtual void addScalarKernel(const std::string & kernel_name,
                               const std::string & name,
                               InputParameters & parameters);
  virtual void addBoundaryCondition(const std::string & bc_name,
                                    const std::string & name,
                                    InputParameters & parameters);
  virtual void
  addConstraint(const std::string & c_name, const std::string & name, InputParameters & parameters);

  virtual void setInputParametersFEProblem(InputParameters & parameters)
  {
    parameters.set<FEProblemBase *>("_fe_problem_base") = this;
  }

  // Aux /////

  /**
   * Canonical method for adding an auxiliary variable
   * @param var_type the type of the variable, e.g. MooseVariableScalar
   * @param var_name the variable name, e.g. 'u'
   * @param params the InputParameters from which to construct the variable
   */
  virtual void addAuxVariable(const std::string & var_type,
                              const std::string & var_name,
                              InputParameters & params);

  virtual void addAuxVariable(const std::string & var_name,
                              const FEType & type,
                              const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxArrayVariable(const std::string & var_name,
                                   const FEType & type,
                                   unsigned int components,
                                   const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxScalarVariable(const std::string & var_name,
                                    Order order,
                                    Real scale_factor = 1.,
                                    const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxKernel(const std::string & kernel_name,
                            const std::string & name,
                            InputParameters & parameters);
  virtual void addAuxScalarKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters);

  AuxiliarySystem & getAuxiliarySystem() { return *_aux; }

  // Dirac /////
  virtual void addDiracKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters);

  // DG /////
  virtual void addDGKernel(const std::string & kernel_name,
                           const std::string & name,
                           InputParameters & parameters);

  virtual void addFVKernel(const std::string & kernel_name,
                           const std::string & name,
                           InputParameters & parameters);

  virtual void
  addFVBC(const std::string & fv_bc_name, const std::string & name, InputParameters & parameters);

  // Interface /////
  virtual void addInterfaceKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters);

  // IC /////
  virtual void addInitialCondition(const std::string & ic_name,
                                   const std::string & name,
                                   InputParameters & parameters);

  void projectSolution();

  // Materials /////
  virtual void addMaterial(const std::string & kernel_name,
                           const std::string & name,
                           InputParameters & parameters);
  virtual void addMaterialHelper(std::vector<MaterialWarehouse *> warehouse,
                                 const std::string & kernel_name,
                                 const std::string & name,
                                 InputParameters & parameters);
  virtual void addInterfaceMaterial(const std::string & kernel_name,
                                    const std::string & name,
                                    InputParameters & parameters);

  /**
   * Add the MooseVariables that the current materials depend on to the dependency list.
   *
   * This MUST be done after the dependency list has been set for all the other objects!
   */
  virtual void prepareMaterials(SubdomainID blk_id, THREAD_ID tid);

  virtual void reinitMaterials(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful = true);

  /**
   * reinit materials on element faces
   * @param blk_id The subdomain on which the element owning the face lives
   * @param tid The thread id
   * @param swap_stateful Whether to swap stateful material properties between \p MaterialData and
   * \p MaterialPropertyStorage
   * @param execute_stateful Whether to execute material objects that have stateful properties. This
   * should be \p false when for example executing material objects for mortar contexts in which
   * stateful properties don't make sense
   */
  virtual void reinitMaterialsFace(SubdomainID blk_id,
                                   THREAD_ID tid,
                                   bool swap_stateful = true,
                                   bool execute_stateful = true);

  /**
   * reinit materials on the neighboring element face
   * @param blk_id The subdomain on which the neighbor element lives
   * @param tid The thread id
   * @param swap_stateful Whether to swap stateful material properties between \p MaterialData and
   * \p MaterialPropertyStorage
   * @param execute_stateful Whether to execute material objects that have stateful properties. This
   * should be \p false when for example executing material objects for mortar contexts in which
   * stateful properties don't make sense
   */
  virtual void reinitMaterialsNeighbor(SubdomainID blk_id,
                                       THREAD_ID tid,
                                       bool swap_stateful = true,
                                       bool execute_stateful = true);

  /**
   * For finite volume bcs, we need to be able to initialize and compute
   * materials on ghost elements (elements that don't exist on the mesh - on
   * the outside side of a boundary face).  To be able to reinit materials
   * under these circumstances without crashing, we provide a special function
   * here that reinits the neighbor material properties using the elem element
   * instead of the neighbor element.
   */
  virtual void
  reinitMaterialsNeighborGhost(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful = true);

  /**
   * reinit materials on a boundary
   * @param boundary_id The boundary on which to reinit corresponding materials
   * @param tid The thread id
   * @param swap_stateful Whether to swap stateful material properties between \p MaterialData and
   * \p MaterialPropertyStorage
   * @param execute_stateful Whether to execute material objects that have stateful properties. This
   * should be \p false when for example executing material objects for mortar contexts in which
   * stateful properties don't make sense
   */
  virtual void reinitMaterialsBoundary(BoundaryID boundary_id,
                                       THREAD_ID tid,
                                       bool swap_stateful = true,
                                       bool execute_stateful = true);

  virtual void
  reinitMaterialsInterface(BoundaryID boundary_id, THREAD_ID tid, bool swap_stateful = true);
  /*
   * Swap back underlying data storing stateful material properties
   */
  virtual void swapBackMaterials(THREAD_ID tid);
  virtual void swapBackMaterialsFace(THREAD_ID tid);
  virtual void swapBackMaterialsNeighbor(THREAD_ID tid);
  /**
   * This is the special materials swap-back function to be paired with
   * reinitMaterialsNeighborGhost.  Always use it (and only it) to swap back whenever you
   * reinit with that function.
   */
  virtual void swapBackMaterialsNeighborGhost(THREAD_ID tid);

  // Postprocessors /////
  virtual void addPostprocessor(const std::string & pp_name,
                                const std::string & name,
                                InputParameters & parameters);

  // VectorPostprocessors /////
  virtual void addVectorPostprocessor(const std::string & pp_name,
                                      const std::string & name,
                                      InputParameters & parameters);

  /**
   * Initializes the postprocessor data
   * @see SetupPostprocessorDataAction
   */
  void initPostprocessorData(const std::string & name);

  /// Initialize the VectorPostprocessor data
  void initVectorPostprocessorData(const std::string & name);

  // UserObjects /////
  virtual void addUserObject(const std::string & user_object_name,
                             const std::string & name,
                             InputParameters & parameters);

  // TODO: delete this function after apps have been updated to not call it
  const ExecuteMooseObjectWarehouse<UserObject> & getUserObjects() const
  {
    mooseDeprecated(
        "This function is deprecated, use theWarehouse().query() to construct a query instead");
    return _all_user_objects;
  }

  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @return Const reference to the user object
   */
  template <class T>
  T & getUserObject(const std::string & name, unsigned int tid = 0) const
  {
    std::vector<T *> objs;
    theWarehouse().query().condition<AttribThread>(tid).condition<AttribName>(name).queryInto(objs);
    if (objs.empty())
      mooseError("Unable to find user object with name '" + name + "'");
    return *(objs[0]);
  }
  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @return Const reference to the user object
   */
  const UserObject & getUserObjectBase(const std::string & name) const;

  /**
   * Check if there if a user object of given name
   * @param name The name of the user object being checked for
   * @return true if the user object exists, false otherwise
   */
  bool hasUserObject(const std::string & name) const;

  /**
   * Check existence of the postprocessor.
   * @param name The name of the post-processor
   * @return true if it exists, otherwise false
   */
  bool hasPostprocessor(const std::string & name);

  /**
   * Get a reference to the value associated with the postprocessor.
   * @param name The name of the post-processor
   * @return The reference to the old value
   *
   * Note: This method cannot be marked const. It calls another interface, which creates maps space
   * in a map on demand.
   */
  PostprocessorValue & getPostprocessorValue(const PostprocessorName & name);

  /**
   * Get the reference to the old value of a post-processor
   * @param name The name of the post-processor
   * @return The reference to the old value
   *
   * Note: This method cannot be marked const. It calls another interface, which creates maps space
   * in a map on demand.
   */
  PostprocessorValue & getPostprocessorValueOld(const std::string & name);

  /**
   * Get the reference to the older value of a post-processor
   * @param name The name of the post-processor
   * @return The reference to the old value
   *
   * Note: This method cannot be marked const. It calls another interface, which creates maps space
   * in a map on demand.
   */
  PostprocessorValue & getPostprocessorValueOlder(const std::string & name);

  ///@{
  /**
   * Returns whether or not the current simulation has any multiapps
   */
  bool hasMultiApps() const { return _multi_apps.hasActiveObjects(); }
  bool hasMultiApps(ExecFlagType type) const;
  bool hasMultiApp(const std::string & name) const;
  ///@}

  /**
   * Check existence of the VectorPostprocessor.
   * @param name The name of the post-processor
   * @return true if it exists, otherwise false
   */
  bool hasVectorPostprocessor(const std::string & name);

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Get a reference to the value associated with the VectorPostprocessor.
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the current value
   */
  VectorPostprocessorValue & getVectorPostprocessorValue(const VectorPostprocessorName & name,
                                                         const std::string & vector_name);

  /**
   * DEPRECATED: Use the new version where you need to specify whether or
   * not the vector must be broadcast
   *
   * Get the reference to the old value of a post-processor
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the old value
   */
  VectorPostprocessorValue & getVectorPostprocessorValueOld(const std::string & name,
                                                            const std::string & vector_name);

  /**
   * Get a reference to the value associated with the VectorPostprocessor.
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the current value
   */
  VectorPostprocessorValue & getVectorPostprocessorValue(const VectorPostprocessorName & name,
                                                         const std::string & vector_name,
                                                         bool needs_broadcast);

  /**
   * Get the reference to the old value of a post-processor
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @return The reference to the old value
   */
  VectorPostprocessorValue & getVectorPostprocessorValueOld(const std::string & name,
                                                            const std::string & vector_name,
                                                            bool needs_broadcast);

  /**
   * Return the scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's value
   * from that vector
   *
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the current scatter value
   */
  ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValue(const VectorPostprocessorName & vpp_name,
                                     const std::string & vector_name);

  /**
   * Return the scatter value for the post processor
   *
   * This is only valid when you expec the vector to be of lenghth "num_procs"
   * In that case - this will return a reference to a value that will be _this_ processor's value
   * from that vector
   *
   * @param vpp_name The name of the VectorPostprocessor
   * @param vector_name The name of the vector
   * @return The reference to the old scatter value
   */
  ScatterVectorPostprocessorValue &
  getScatterVectorPostprocessorValueOld(const VectorPostprocessorName & vpp_name,
                                        const std::string & vector_name);

  /**
   * Declare a new VectorPostprocessor vector
   * @param name The name of the post-processor
   * @param vector_name The name of the post-processor
   * @param contains_complete_history True if the vector will naturally contain the complete time
   * history of the values
   * @param is_broadcast True if the vector will already be replicated by the VPP.  This prevents
   * unnecessary broadcasting by MOOSE.
   * @return The reference to the vector declared
   */
  VectorPostprocessorValue & declareVectorPostprocessorVector(const VectorPostprocessorName & name,
                                                              const std::string & vector_name,
                                                              bool contains_complete_history,
                                                              bool is_broadcast,
                                                              bool is_distributed);

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
  virtual void addDamper(const std::string & damper_name,
                         const std::string & name,
                         InputParameters & parameters);
  void setupDampers();

  /**
   * Whether or not this system has dampers.
   */
  bool hasDampers() { return _has_dampers; }

  // Indicators /////
  virtual void addIndicator(const std::string & indicator_name,
                            const std::string & name,
                            InputParameters & parameters);

  // Markers //////
  virtual void addMarker(const std::string & marker_name,
                         const std::string & name,
                         InputParameters & parameters);

  /**
   * Add a MultiApp to the problem.
   */
  virtual void addMultiApp(const std::string & multi_app_name,
                           const std::string & name,
                           InputParameters & parameters);

  /**
   * Get a MultiApp object by name.
   */
  std::shared_ptr<MultiApp> getMultiApp(const std::string & multi_app_name) const;

  /**
   * Get Transfers by ExecFlagType and direction
   */
  std::vector<std::shared_ptr<Transfer>> getTransfers(ExecFlagType type,
                                                      Transfer::DIRECTION direction) const;
  std::vector<std::shared_ptr<Transfer>> getTransfers(Transfer::DIRECTION direction) const;

  /**
   * Return the complete warehouse for MultiAppTransfer object for the given direction
   */
  const ExecuteMooseObjectWarehouse<Transfer> &
  getMultiAppTransferWarehouse(Transfer::DIRECTION direction) const;

  /**
   * Execute MultiAppTransfers associate with execution flag and direction.
   * @param type The execution flag to execute.
   * @param direction The direction (to or from) to transfer.
   */
  void execMultiAppTransfers(ExecFlagType type, Transfer::DIRECTION direction);

  /**
   * Execute the MultiApps associated with the ExecFlagType
   */
  bool execMultiApps(ExecFlagType type, bool auto_advance = true);

  void finalizeMultiApps();

  /**
   * Advance the MultiApps t_step (incrementStepOrReject) associated with the ExecFlagType
   */
  void incrementMultiAppTStep(ExecFlagType type);

  /**
   * Deprecated method; use finishMultiAppStep and/or incrementMultiAppTStep depending
   * on your purpose
   */
  void advanceMultiApps(ExecFlagType type)
  {
    mooseDeprecated("Deprecated method; use finishMultiAppStep and/or incrementMultiAppTStep "
                    "depending on your purpose");
    finishMultiAppStep(type);
  }

  /**
   * Finish the MultiApp time step (endStep, postStep) associated with the ExecFlagType. Optionally
   * recurse through all multi-app levels
   */
  void finishMultiAppStep(ExecFlagType type, bool recurse_through_multiapp_levels = false);

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
  virtual void addTransfer(const std::string & transfer_name,
                           const std::string & name,
                           InputParameters & parameters);

  /**
   * Execute the Transfers associated with the ExecFlagType
   *
   * Note: This does _not_ execute MultiApp Transfers!
   * Those are executed automatically when MultiApps are executed.
   */
  void execTransfers(ExecFlagType type);

  /// Evaluates transient residual G in canonical semidiscrete form G(t,U,Udot,Udotdot) = F(t,U)
  void computeTransientImplicitResidual(Real time,
                                        const NumericVector<Number> & u,
                                        const NumericVector<Number> & udot,
                                        const NumericVector<Number> & udotdot,
                                        NumericVector<Number> & residual);

  /// Evaluates transient Jacobian J_a = dG/dU + a*dG/dUdot from canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitJacobian(Real time,
                                        const NumericVector<Number> & u,
                                        const NumericVector<Number> & udot,
                                        const NumericVector<Number> & udotdot,
                                        Real duDotDu_shift,
                                        Real duDotDotDu_shift,
                                        SparseMatrix<Number> & jacobian);

  ////

  /**
   * Computes the residual using whatever is sitting in the current solution vector then returns the
   * L2 norm.
   *
   * @return The L2 norm of the residual
   */
  virtual Real computeResidualL2Norm();

  /**
   * This function is called by Libmesh to form a residual.
   */
  virtual void computeResidualSys(NonlinearImplicitSystem & sys,
                                  const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual);
  /**
   * This function is called by Libmesh to form a residual. This is deprecated.
   * We should remove this as soon as RattleSnake is fixed.
   */
  void computeResidual(NonlinearImplicitSystem & sys,
                       const NumericVector<Number> & soln,
                       NumericVector<Number> & residual);

  /**
   * Form a residual with default tags (nontime, time, residual).
   */
  virtual void computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual);
  /**
   * Form a residual vector for a given tag
   */
  virtual void computeResidualTag(const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual,
                                  TagID tag);
  /**
   * Form a residual vector for a given tag and "residual" tag
   */
  virtual void computeResidualType(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual,
                                   TagID tag);

  /**
   * Form a residual vector for a set of tags. It should not be called directly
   * by users.
   */
  virtual void computeResidualInternal(const NumericVector<Number> & soln,
                                       NumericVector<Number> & residual,
                                       const std::set<TagID> & tags);
  /**
   * Form multiple residual vectors and each is associated with one tag
   */
  virtual void computeResidualTags(const std::set<TagID> & tags);

  /**
   * Form a Jacobian matrix. It is called by Libmesh.
   */
  virtual void computeJacobianSys(NonlinearImplicitSystem & sys,
                                  const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian);
  /**
   * Form a Jacobian matrix with the default tag (system).
   */
  virtual void computeJacobian(const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian);

  /**
   * Form a Jacobian matrix for a given tag.
   */
  virtual void computeJacobianTag(const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian,
                                  TagID tag);

  /**
   * Form a Jacobian matrix for multiple tags. It should not be called directly by users.
   */
  virtual void computeJacobianInternal(const NumericVector<Number> & soln,
                                       SparseMatrix<Number> & jacobian,
                                       const std::set<TagID> & tags);

  /**
   * Form multiple matrices, and each is associated with a tag.
   */
  virtual void computeJacobianTags(const std::set<TagID> & tags);

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

  virtual void addJacobian(THREAD_ID tid) override;
  virtual void addJacobianNeighbor(THREAD_ID tid) override;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid) override;
  virtual void addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                                    unsigned int ivar,
                                    unsigned int jvar,
                                    const DofMap & dof_map,
                                    std::vector<dof_id_type> & dof_indices,
                                    const std::set<TagID> & tags,
                                    THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   THREAD_ID tid) override;
  virtual void addJacobianScalar(THREAD_ID tid = 0);
  virtual void addJacobianOffDiagScalar(unsigned int ivar, THREAD_ID tid = 0);

  virtual void cacheJacobian(THREAD_ID tid) override;
  virtual void cacheJacobianNeighbor(THREAD_ID tid) override;
  virtual void addCachedJacobian(THREAD_ID tid) override;

  virtual void prepareShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) override;

  // Displaced problem /////
  virtual void addDisplacedProblem(std::shared_ptr<DisplacedProblem> displaced_problem);
  virtual std::shared_ptr<const DisplacedProblem> getDisplacedProblem() const
  {
    return _displaced_problem;
  }
  virtual std::shared_ptr<DisplacedProblem> getDisplacedProblem() { return _displaced_problem; }

  virtual void updateGeomSearch(
      GeometricSearchData::GeometricSearchType type = GeometricSearchData::ALL) override;
  virtual void updateMortarMesh();

  void createMortarInterface(
      const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
      const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
      bool on_displaced,
      bool periodic);

  const AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
                     const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
                     bool on_displaced) const;

  const std::unordered_map<std::pair<BoundaryID, BoundaryID>, AutomaticMortarGeneration> &
  getMortarInterfaces(bool on_displaced) const;

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
  const MaterialPropertyStorage & getNeighborMaterialPropertyStorage()
  {
    return _neighbor_material_props;
  }
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

  /**
   * @returns Whether or not the mesh was changed
   */
  virtual bool adaptMesh();

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
  bool haveXFEM() { return _xfem != nullptr; }

  /// Update the mesh due to changing XFEM cuts
  virtual bool updateMeshXFEM();

  /**
   * Update data after a mesh change.
   */
  virtual void meshChanged() override;

  /**
   * Register an object that derives from MeshChangedInterface
   * to be notified when the mesh changes.
   */
  void notifyWhenMeshChanges(MeshChangedInterface * mci);

  /**
   * Method called to perform a series of sanity checks before a simulation is run. This method
   * doesn't return when errors are found, instead it generally calls mooseError() directly.
   */
  virtual void checkProblemIntegrity();

  void serializeSolution();

  void registerRandomInterface(RandomInterface & random_interface, const std::string & name);

  /**
   * Set flag that Jacobian is constant (for optimization purposes)
   * @param state True if the Jacobian is constant, false otherwise
   */
  void setConstJacobian(bool state) { _const_jacobian = state; }

  /**
   * Set flag to indicate whether kernel coverage checks should be performed. This check makes
   * sure that at least one kernel is active on all subdomains in the domain (default: true).
   */
  void setKernelCoverageCheck(bool flag) { _kernel_coverage_check = flag; }

  /**
   * Set flag to indicate whether material coverage checks should be performed. This check makes
   * sure that at least one material is active on all subdomains in the domain if any material is
   * supplied. If no materials are supplied anywhere, a simulation is still considered OK as long as
   * no properties are being requested anywhere.
   */
  void setMaterialCoverageCheck(bool flag) { _material_coverage_check = flag; }

  /**
   * Toggle parallel barrier messaging (defaults to on).
   */
  void setParallelBarrierMessaging(bool flag) { _parallel_barrier_messaging = flag; }

  /**
   * Calls parentOutputPositionChanged() on all sub apps.
   */
  void parentOutputPositionChanged();

  ///@{
  /**
   * These methods are used to determine whether stateful material properties need to be stored on
   * internal sides.  There are five situations where this may be the case: 1) DGKernels
   * 2) IntegratedBCs 3)InternalSideUserObjects 4)ElementalAuxBCs 5)InterfaceUserObjects
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
  bool needBoundaryMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid);
  bool needInterfaceMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid);
  bool needSubdomainMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid);
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
  const MaterialWarehouse & getMaterialWarehouse() const { return _all_materials; }

  /*
   * Return a reference to the material warehouse of Material objects to be computed.
   */
  const MaterialWarehouse & getRegularMaterialsWarehouse() const { return _materials; }
  const MaterialWarehouse & getDiscreteMaterialWarehouse() const { return _discrete_materials; }
  const MaterialWarehouse & getInterfaceMaterialsWarehouse() const { return _interface_materials; }

  /**
   * Return a pointer to a MaterialBase object.  If no_warn is true, suppress
   * warning about retrieving a material reference potentially during the
   * material's calculation.
   *
   * This will return enabled or disabled objects, the main purpose is for iterative materials.
   */
  std::shared_ptr<MaterialBase> getMaterial(std::string name,
                                            Moose::MaterialDataType type,
                                            THREAD_ID tid = 0,
                                            bool no_warn = false);

  std::shared_ptr<MaterialBase> getInterfaceMaterial(std::string name,
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
  bool errorOnJacobianNonzeroReallocation() const
  {
    return _error_on_jacobian_nonzero_reallocation;
  }

  void setErrorOnJacobianNonzeroReallocation(bool state)
  {
    _error_on_jacobian_nonzero_reallocation = state;
  }

  bool ignoreZerosInJacobian() const { return _ignore_zeros_in_jacobian; }

  void setIgnoreZerosInJacobian(bool state) { _ignore_zeros_in_jacobian = state; }

  /// Returns whether or not this Problem has a TimeIntegrator
  bool hasTimeIntegrator() const { return _has_time_integrator; }

  ///@{
  /**
   * Return/set the current execution flag.
   *
   * Returns EXEC_NONE when not being executed.
   * @see FEProblemBase::execute
   */
  const ExecFlagType & getCurrentExecuteOnFlag() const;
  void setCurrentExecuteOnFlag(const ExecFlagType &);
  ///@}

  /**
   * Convenience function for performing execution of MOOSE systems.
   */
  virtual void execute(const ExecFlagType & exec_type);

  /**
   * Call compute methods on UserObjects.
   */
  virtual void computeUserObjects(const ExecFlagType & type, const Moose::AuxGroup & group);

  /**
   * Compute an user object with the given name
   */
  virtual void computeUserObjectByName(const ExecFlagType & type, const std::string & name);

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
  bool needsPreviousNewtonIteration() const;

  /**
   * Whether or not to skip loading the additional data when restarting
   */
  bool skipAdditionalRestartData() const { return _skip_additional_restart_data; }

  ///@{
  /**
   * Convenience zeros
   */
  std::vector<Real> _real_zero;
  std::vector<VariableValue> _scalar_zero;
  std::vector<VariableValue> _zero;
  std::vector<VariablePhiValue> _phi_zero;
  std::vector<MooseArray<ADReal>> _ad_zero;
  std::vector<VariableGradient> _grad_zero;
  std::vector<MooseArray<ADRealVectorValue>> _ad_grad_zero;
  std::vector<VariablePhiGradient> _grad_phi_zero;
  std::vector<VariableSecond> _second_zero;
  std::vector<MooseArray<ADRealTensorValue>> _ad_second_zero;
  std::vector<VariablePhiSecond> _second_phi_zero;
  std::vector<Point> _point_zero;
  std::vector<VectorVariableValue> _vector_zero;
  std::vector<VectorVariableCurl> _vector_curl_zero;
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
  virtual void updateActiveObjects();

  /**
   * Register a MOOSE object dependency so we can either order
   * operations properly or report when we cannot.
   * a -> b (a depends on b)
   */
  void reportMooseObjectDependency(MooseObject * a, MooseObject * b);

  ExecuteMooseObjectWarehouse<MultiApp> & getMultiAppWarehouse() { return _multi_apps; }

  const VectorPostprocessorData & getVectorPostprocessorData() const;

  /**
   * Returns _has_jacobian
   */
  bool hasJacobian() const;

  /**
   * Returns _const_jacobian (whether a MOOSE object has specified that
   * the Jacobian is the same as the previous time it was computed)
   */
  bool constJacobian() const;

  /**
   * Adds an Output object.
   */
  void addOutput(const std::string &, const std::string &, InputParameters &);

  inline TheWarehouse & theWarehouse() const { return _app.theWarehouse(); }

  /**
   * If or not to reuse the base vector for matrix-free calculation
   */
  void setSNESMFReuseBase(bool reuse, bool set_by_user)
  {
    _snesmf_reuse_base = reuse, _snesmf_reuse_base_set_by_user = set_by_user;
  }

  /**
   * Return a flag that indicates if we are reusing the vector base
   */
  bool useSNESMFReuseBase() { return _snesmf_reuse_base; }

  /**
   * Set a flag that indicates if we want to skip exception and stop solve
   */
  void skipExceptionCheck(bool skip_exception_check)
  {
    _skip_exception_check = skip_exception_check;
  }

  /**
   * Return a flag to indicate if _snesmf_reuse_base is set by users
   */
  bool isSNESMFReuseBaseSetbyUser() { return _snesmf_reuse_base_set_by_user; }

  /**
   * If petsc options are already inserted
   */
  bool & petscOptionsInserted() { return _is_petsc_options_inserted; }

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  PetscOptions & petscOptionsDatabase() { return _petsc_option_data_base; }
#endif

  /**
   * Set the global automatic differentiaion (AD) flag which indicates whether any consumer has
   * requested an AD material property or whether any suppier has declared an AD material property
   */
  void usingADMatProps(bool using_ad_mat_props)
  {
    _using_ad_mat_props = using_ad_mat_props;
    if (_using_ad_mat_props)
      haveADObjects(true);
  }

  /**
   * Whether any object has requested/supplied an AD material property
   */
  bool usingADMatProps() const { return _using_ad_mat_props; }

  /// Set boolean flag to true to store solution time derivative
  virtual void setUDotRequested(const bool u_dot_requested) { _u_dot_requested = u_dot_requested; };

  /// Set boolean flag to true to store solution second time derivative
  virtual void setUDotDotRequested(const bool u_dotdot_requested)
  {
    _u_dotdot_requested = u_dotdot_requested;
  };

  /// Set boolean flag to true to store old solution time derivative
  virtual void setUDotOldRequested(const bool u_dot_old_requested)
  {
    _u_dot_old_requested = u_dot_old_requested;
  };

  /// Set boolean flag to true to store old solution second time derivative
  virtual void setUDotDotOldRequested(const bool u_dotdot_old_requested)
  {
    _u_dotdot_old_requested = u_dotdot_old_requested;
  };

  /// Get boolean flag to check whether solution time derivative needs to be stored
  virtual bool uDotRequested() { return _u_dot_requested; };

  /// Get boolean flag to check whether solution second time derivative needs to be stored
  virtual bool uDotDotRequested() { return _u_dotdot_requested; };

  /// Get boolean flag to check whether old solution time derivative needs to be stored
  virtual bool uDotOldRequested()
  {
    if (_u_dot_old_requested && !_u_dot_requested)
      mooseError("FEProblemBase: When requesting old time derivative of solution, current time "
                 "derivative of solution should also be stored. Please set `u_dot_requested` to "
                 "true using setUDotRequested.");

    return _u_dot_old_requested;
  };

  /// Get boolean flag to check whether old solution second time derivative needs to be stored
  virtual bool uDotDotOldRequested()
  {
    if (_u_dotdot_old_requested && !_u_dotdot_requested)
      mooseError("FEProblemBase: When requesting old second time derivative of solution, current "
                 "second time derivation of solution should also be stored. Please set "
                 "`u_dotdot_requested` to true using setUDotDotRequested.");
    return _u_dotdot_old_requested;
  };

  using SubProblem::haveADObjects;
  void haveADObjects(bool have_ad_objects) override;

  // Whether or not we should solve this system
  bool shouldSolve() const { return _solve; }

  /**
   * Returns the mortar data object
   */
  const MortarData & mortarData() const { return _mortar_data; }

  /**
   * Whether the simulation has neighbor coupling
   */
  virtual bool hasNeighborCoupling() const { return _has_internal_edge_residual_objects; }

  /**
   * Whether the simulation has mortar coupling
   */
  virtual bool hasMortarCoupling() const { return _has_mortar; }

  using SubProblem::computingNonlinearResid;
  void computingNonlinearResid(bool computing_nonlinear_residual) override;

  /**
   * Set the number of steps in a grid sequences
   */
  void numGridSteps(unsigned int num_grid_steps) { _num_grid_steps = num_grid_steps; }

  /**
   * uniformly refine the problem mesh(es). This will also prolong the the solution, and in order
   * for that to be safe, we can only perform one refinement at a time
   */
  void uniformRefine();

  using SubProblem::automaticScaling;
  void automaticScaling(bool automatic_scaling) override;

  ///@{
  /**
   * Helpers for calling the necessary setup/execute functions for the supplied objects
   */
  template <typename T>
  static void objectSetupHelper(const std::vector<T *> & objects, const ExecFlagType & exec_flag);
  template <typename T>
  static void objectExecuteHelper(const std::vector<T *> & objects);
  ///@}

  /**
   * reinitialize FE objects on a given element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitElemFaceRef(const Elem * elem,
                                 unsigned int side,
                                 BoundaryID bnd_id,
                                 Real tolerance,
                                 const std::vector<Point> * const pts,
                                 const std::vector<Real> * const weights = nullptr,
                                 THREAD_ID tid = 0) override;

  /**
   * reinitialize FE objects on a given neighbor element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitNeighborFaceRef(const Elem * neighbor_elem,
                                     unsigned int neighbor_side,
                                     BoundaryID bnd_id,
                                     Real tolerance,
                                     const std::vector<Point> * const pts,
                                     const std::vector<Real> * const weights = nullptr,
                                     THREAD_ID tid = 0) override;

protected:
  /// Create extra tagged vectors and matrices
  void createTagVectors();

  MooseMesh & _mesh;
  EquationSystems _eq;
  bool _initialized;

  std::set<TagID> _fe_vector_tags;

  std::set<TagID> _fe_matrix_tags;

  /// Whether or not to actually solve the nonlinear system
  const bool & _solve;

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

  std::vector<std::unique_ptr<Assembly>> _assembly;

  /// functions
  MooseObjectWarehouse<Function> _functions;

  /// nonlocal kernels
  MooseObjectWarehouse<KernelBase> _nonlocal_kernels;

  /// nonlocal integrated_bcs
  MooseObjectWarehouse<IntegratedBCBase> _nonlocal_integrated_bcs;

  ///@{
  /// Initial condition storage
  InitialConditionWarehouse _ics;
  ScalarInitialConditionWarehouse _scalar_ics; // use base b/c of setup methods
  ///@}

  // material properties
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;
  MaterialPropertyStorage & _neighbor_material_props;

  std::vector<std::shared_ptr<MaterialData>> _material_data;
  std::vector<std::shared_ptr<MaterialData>> _bnd_material_data;
  std::vector<std::shared_ptr<MaterialData>> _neighbor_material_data;

  ///@{
  // Material Warehouses
  MaterialWarehouse _materials;           // regular materials
  MaterialWarehouse _interface_materials; // interface materials
  MaterialWarehouse _discrete_materials;  // Materials that the user must compute
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

  // TODO: delete this after apps have been updated to not call getUserObjects
  ExecuteMooseObjectWarehouse<UserObject> _all_user_objects;

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

  /// Cache for calculating materials on interface
  std::vector<std::unordered_map<BoundaryID, bool>> _interface_mat_side_cache;

  /// Objects to be notified when the mesh changes
  std::vector<MeshChangedInterface *> _notify_when_mesh_changes;

  /**
   * Helper method to update some or all data after a mesh change.
   *
   * Iff intermediate_change is true, only perform updates as
   * necessary to prepare for another mesh change
   * immediately-subsequent.
   */
  void meshChangedHelper(bool intermediate_change = false);

  /// Helper to check for duplicate variable names across systems or within a single system
  bool duplicateVariableCheck(const std::string & var_name, const FEType & type, bool is_aux);

  void computeUserObjectsInternal(const ExecFlagType & type,
                                  const Moose::AuxGroup & group,
                                  TheWarehouse::Query & query);

  /// Verify that SECOND order mesh uses SECOND order displacements.
  void checkDisplacementOrders();

  void checkUserObjects();

  /**
   * Helper method for checking Material object dependency.
   *
   * @see checkProblemIntegrity
   */
  void checkDependMaterialsHelper(
      const std::map<SubdomainID, std::vector<std::shared_ptr<MaterialBase>>> & materials_map);

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
  MortarData _mortar_data;

  bool _reinit_displaced_elem;
  bool _reinit_displaced_face;

  /// whether input file has been written
  bool _input_file_saved;

  /// Whether or not this system has any Dampers associated with it.
  bool _has_dampers;

  /// Whether or not this system has any Constraints.
  bool _has_constraints;

  /// If or not to resuse the base vector for matrix-free calculation
  bool _snesmf_reuse_base;

  /// If or not skip 'exception and stop solve'
  bool _skip_exception_check;

  /// If or not _snesmf_reuse_base is set by user
  bool _snesmf_reuse_base_set_by_user;

  /// Whether nor not stateful materials have been initialized
  bool _has_initialized_stateful;

  /// Object responsible for restart (read/write)
  std::unique_ptr<RestartableDataIO> _restart_io;

  /// true if the Jacobian is constant
  bool _const_jacobian;

  /// Indicates if the Jacobian was computed
  bool _has_jacobian;

  /// Indicates that we need to compute variable values for previous Newton iteration
  bool _needs_old_newton_iter;

  /// Indicates if nonlocal coupling is required/exists
  bool _has_nonlocal_coupling;
  bool _calculate_jacobian_in_uo;

  std::vector<std::vector<const MooseVariableFEBase *>> _uo_jacobian_moose_vars;

  SolverParams _solver_params;

  /// Determines whether a check to verify an active kernel on every subdomain
  bool _kernel_coverage_check;

  /// Determines whether a check to verify an active material on every subdomain
  bool _material_coverage_check;

  /// Determines whether a check to verify material dependencies on every subdomain
  const bool _material_dependency_check;

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

  /// Whether or not information about how many transfers have completed is printed
  bool _parallel_barrier_messaging;

  /// The error message to go with an exception
  std::string _exception_message;

  /// Current execute_on flag
  ExecFlagType _current_execute_on_flag;

  /// The control logic warehouse
  ExecuteMooseObjectWarehouse<Control> _control_warehouse;

#ifdef LIBMESH_HAVE_PETSC
  /// PETSc option storage
  Moose::PetscSupport::PetscOptions _petsc_options;
#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  PetscOptions _petsc_option_data_base;
#endif
#endif // LIBMESH_HAVE_PETSC
  /// If or not petsc options have been added to database
  bool _is_petsc_options_inserted;

  std::shared_ptr<LineSearch> _line_search;

  std::unique_ptr<ConstElemRange> _evaluable_local_elem_range;

  /// Automatic differentiaion (AD) flag which indicates whether any consumer has
  /// requested an AD material property or whether any suppier has declared an AD material property
  bool _using_ad_mat_props;

private:
  void updateMaxQps();

  void joinAndFinalize(TheWarehouse::Query query, bool isgen = false);

  bool _error_on_jacobian_nonzero_reallocation;
  bool _ignore_zeros_in_jacobian;
  const bool _force_restart;
  const bool _skip_additional_restart_data;
  const bool _skip_nl_system_check;
  bool _fail_next_linear_convergence_check;

  /// At or beyond initialSteup stage
  bool _started_initial_setup;

  /// Whether the problem has dgkernels or interface kernels
  bool _has_internal_edge_residual_objects;

  /// Timers
  const PerfID _initial_setup_timer;
  const PerfID _project_solution_timer;
  const PerfID _compute_indicators_timer;
  const PerfID _compute_markers_timer;
  const PerfID _compute_user_objects_timer;
  const PerfID _execute_controls_timer;
  const PerfID _execute_samplers_timer;
  const PerfID _update_active_objects_timer;
  const PerfID _reinit_because_of_ghosting_or_new_geom_objects_timer;
  const PerfID _exec_multi_app_transfers_timer;
  const PerfID _init_timer;
  const PerfID _eq_init_timer;
  const PerfID _solve_timer;
  const PerfID _check_exception_and_stop_solve_timer;
  const PerfID _advance_state_timer;
  const PerfID _restore_solutions_timer;
  const PerfID _save_old_solutions_timer;
  const PerfID _restore_old_solutions_timer;
  const PerfID _output_step_timer;
  const PerfID _on_timestep_begin_timer;
  const PerfID _compute_residual_l2_norm_timer;
  const PerfID _compute_residual_sys_timer;
  const PerfID _compute_residual_internal_timer;
  const PerfID _compute_residual_type_timer;
  const PerfID _compute_transient_implicit_residual_timer;
  const PerfID _compute_residual_tags_timer;
  const PerfID _compute_jacobian_internal_timer;
  const PerfID _compute_jacobian_tags_timer;
  const PerfID _compute_jacobian_blocks_timer;
  const PerfID _compute_bounds_timer;
  const PerfID _compute_post_check_timer;
  const PerfID _compute_damping_timer;
  const PerfID _possibly_rebuild_geom_search_patches_timer;
  const PerfID _initial_adapt_mesh_timer;
  const PerfID _adapt_mesh_timer;
  const PerfID _update_mesh_xfem_timer;
  const PerfID _mesh_changed_timer;
  const PerfID _mesh_changed_helper_timer;
  const PerfID _check_problem_integrity_timer;
  const PerfID _serialize_solution_timer;
  const PerfID _check_nonlinear_convergence_timer;
  const PerfID _check_linear_convergence_timer;
  const PerfID _update_geometric_search_timer;
  const PerfID _exec_multi_apps_timer;
  const PerfID _backup_multi_apps_timer;

  /// Whether solution time derivative needs to be stored
  bool _u_dot_requested;

  /// Whether solution second time derivative needs to be stored
  bool _u_dotdot_requested;

  /// Whether old solution time derivative needs to be stored
  bool _u_dot_old_requested;

  /// Whether old solution second time derivative needs to be stored
  bool _u_dotdot_old_requested;

  friend class AuxiliarySystem;
  friend class NonlinearSystemBase;
  friend class MooseEigenSystem;
  friend class Resurrector;
  friend class RestartableDataIO;
  friend class Restartable;
  friend class DisplacedProblem;

  /// Whether the simulation requires mortar coupling
  bool _has_mortar;

  /// Number of steps in a grid sequence
  unsigned int _num_grid_steps;

  /// MooseEnum describing how to obtain reference points for displaced mesh dgkernels and/or
  /// interface kernels. Options are invert_elem_phys, use_undisplaced_ref, and the default unset.
  MooseEnum _displaced_neighbor_ref_pts;
};

template <typename T>
void
FEProblemBase::allowOutput(bool state)
{
  _app.getOutputWarehouse().allowOutput<T>(state);
}

template <typename T>
void
FEProblemBase::objectSetupHelper(const std::vector<T *> & objects, const ExecFlagType & exec_flag)
{
  if (exec_flag == EXEC_INITIAL)
  {
    for (T * obj_ptr : objects)
      obj_ptr->initialSetup();
  }

  else if (exec_flag == EXEC_TIMESTEP_BEGIN)
  {
    for (const auto obj_ptr : objects)
      obj_ptr->timestepSetup();
  }
  else if (exec_flag == EXEC_SUBDOMAIN)
  {
    for (const auto obj_ptr : objects)
      obj_ptr->subdomainSetup();
  }

  else if (exec_flag == EXEC_NONLINEAR)
  {
    for (const auto obj_ptr : objects)
      obj_ptr->jacobianSetup();
  }

  else if (exec_flag == EXEC_LINEAR)
  {
    for (const auto obj_ptr : objects)
      obj_ptr->residualSetup();
  }
}

template <typename T>
void
FEProblemBase::objectExecuteHelper(const std::vector<T *> & objects)
{
  for (T * obj_ptr : objects)
    obj_ptr->execute();
}
