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
#include "MeshDivision.h"
#include "MortarData.h"
#include "ReporterData.h"
#include "Adaptivity.h"
#include "InitialConditionWarehouse.h"
#include "FVInitialConditionWarehouse.h"
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
#include "MooseObjectWarehouse.h"
#include "MaterialPropertyRegistry.h"
#include "RestartableEquationSystems.h"
#include "SolutionInvalidity.h"
#include "PetscSupport.h"

#include "libmesh/enum_quadrature_type.h"
#include "libmesh/equation_systems.h"

#include <unordered_map>
#include <memory>

// Forward declarations
class AuxiliarySystem;
class DisplacedProblem;
class MooseMesh;
class NonlinearSystemBase;
class LinearSystem;
class SolverSystem;
class NonlinearSystem;
class RandomInterface;
class RandomData;
class MeshChangedInterface;
class MultiMooseEnum;
class MaterialPropertyStorage;
class MaterialData;
class MooseEnum;
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
class Positions;
class Function;
class Distribution;
class Sampler;
class KernelBase;
class IntegratedBCBase;
class LineSearch;
class UserObject;
class AutomaticMortarGeneration;
class VectorPostprocessor;
class Convergence;
class MooseAppCoordTransform;
class MortarUserObject;
class SolutionInvalidity;

// libMesh forward declarations
namespace libMesh
{
class CouplingMatrix;
class NonlinearImplicitSystem;
class LinearImplicitSystem;
} // namespace libMesh

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

  enum class CoverageCheckMode
  {
    FALSE,
    TRUE,
    OFF,
    ON,
    SKIP_LIST,
    ONLY_LIST,
  };

  virtual libMesh::EquationSystems & es() override { return _req.set().es(); }
  virtual MooseMesh & mesh() override { return _mesh; }
  virtual const MooseMesh & mesh() const override { return _mesh; }
  const MooseMesh & mesh(bool use_displaced) const override;

  void setCoordSystem(const std::vector<SubdomainName> & blocks, const MultiMooseEnum & coord_sys);
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
   * @param nl_sys_num which nonlinear system we are setting the coupling matrix for
   */
  void setCouplingMatrix(std::unique_ptr<libMesh::CouplingMatrix> cm,
                         const unsigned int nl_sys_num);

  // DEPRECATED METHOD
  void setCouplingMatrix(libMesh::CouplingMatrix * cm, const unsigned int nl_sys_num);

  const libMesh::CouplingMatrix * couplingMatrix(const unsigned int nl_sys_num) const override;

  /// Set custom coupling matrix for variables requiring nonlocal contribution
  void setNonlocalCouplingMatrix();

  bool
  areCoupled(const unsigned int ivar, const unsigned int jvar, const unsigned int nl_sys_num) const;

  /**
   * Whether or not MOOSE will perform a user object/auxiliary kernel state check
   */
  bool hasUOAuxStateCheck() const { return _uo_aux_state_check; }

  /**
   * Return a flag to indicate whether we are executing user objects and auxliary kernels for state
   * check
   * Note: This function can return true only when hasUOAuxStateCheck() returns true, i.e. the check
   *       has been activated by users through Problem/check_uo_aux_state input parameter.
   */
  bool checkingUOAuxState() const { return _checking_uo_aux_state; }

  /**
   * Whether to trust the user coupling matrix even if we want to do things like be paranoid and
   * create a full coupling matrix. See https://github.com/idaholab/moose/issues/16395 for detailed
   * background
   */
  void trustUserCouplingMatrix();

  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> &
  couplingEntries(const THREAD_ID tid, const unsigned int nl_sys_num);
  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> &
  nonlocalCouplingEntries(const THREAD_ID tid, const unsigned int nl_sys_num);

  virtual bool hasVariable(const std::string & var_name) const override;
  using SubProblem::getVariable;
  virtual const MooseVariableFieldBase &
  getVariable(const THREAD_ID tid,
              const std::string & var_name,
              Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
              Moose::VarFieldType expected_var_field_type =
                  Moose::VarFieldType::VAR_FIELD_ANY) const override;
  MooseVariableFieldBase & getActualFieldVariable(const THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual MooseVariable & getStandardVariable(const THREAD_ID tid,
                                              const std::string & var_name) override;
  virtual VectorMooseVariable & getVectorVariable(const THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual ArrayMooseVariable & getArrayVariable(const THREAD_ID tid,
                                                const std::string & var_name) override;

  virtual bool hasScalarVariable(const std::string & var_name) const override;
  virtual MooseVariableScalar & getScalarVariable(const THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual libMesh::System & getSystem(const std::string & var_name) override;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void setActiveElementalMooseVariables(const std::set<MooseVariableFEBase *> & moose_vars,
                                                const THREAD_ID tid) override;

  /**
   * Clear the active elemental MooseVariableFEBase.  If there are no active variables then they
   * will all be reinited. Call this after finishing the computation that was using a restricted set
   * of MooseVariableFEBases
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(const THREAD_ID tid) override;

  virtual void clearActiveFEVariableCoupleableMatrixTags(const THREAD_ID tid) override;

  virtual void clearActiveFEVariableCoupleableVectorTags(const THREAD_ID tid) override;

  virtual void setActiveFEVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                       const THREAD_ID tid) override;

  virtual void setActiveFEVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                       const THREAD_ID tid) override;

  virtual void clearActiveScalarVariableCoupleableMatrixTags(const THREAD_ID tid) override;

  virtual void clearActiveScalarVariableCoupleableVectorTags(const THREAD_ID tid) override;

  virtual void setActiveScalarVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                           const THREAD_ID tid) override;

  virtual void setActiveScalarVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                           const THREAD_ID tid) override;

  virtual void createQRules(libMesh::QuadratureType type,
                            libMesh::Order order,
                            libMesh::Order volume_order = libMesh::INVALID_ORDER,
                            libMesh::Order face_order = libMesh::INVALID_ORDER,
                            SubdomainID block = Moose::ANY_BLOCK_ID,
                            bool allow_negative_qweights = true);

  /**
   * Increases the element/volume quadrature order for the specified mesh
   * block if and only if the current volume quadrature order is lower.  This
   * can only cause the quadrature level to increase.  If volume_order is
   * lower than or equal to the current volume/elem quadrature rule order,
   * then nothing is done (i.e. this function is idempotent).
   */
  void bumpVolumeQRuleOrder(libMesh::Order order, SubdomainID block);

  void bumpAllQRuleOrder(libMesh::Order order, SubdomainID block);

  /**
   * @return The maximum number of quadrature points in use on any element in this problem.
   */
  unsigned int getMaxQps() const;

  /**
   * @return The maximum order for all scalar variables in this problem's systems.
   */
  libMesh::Order getMaxScalarOrder() const;

  /**
   * @return Flag indicating nonlocal coupling exists or not.
   */
  void checkNonlocalCoupling();
  void checkUserObjectJacobianRequirement(THREAD_ID tid);
  void setVariableAllDoFMap(const std::vector<const MooseVariableFEBase *> & moose_vars);

  const std::vector<const MooseVariableFEBase *> &
  getUserObjectJacobianVariables(const THREAD_ID tid) const
  {
    return _uo_jacobian_moose_vars[tid];
  }

  virtual Assembly & assembly(const THREAD_ID tid, const unsigned int sys_num) override;
  virtual const Assembly & assembly(const THREAD_ID tid, const unsigned int sys_num) const override;

  /**
   * Returns a list of all the variables in the problem (both from the NL and Aux systems.
   */
  virtual std::vector<VariableName> getVariableNames();

  void initialSetup() override;
  void checkDuplicatePostprocessorVariableNames();
  void timestepSetup() override;
  void customSetup(const ExecFlagType & exec_type) override;
  void residualSetup() override;
  void jacobianSetup() override;

  virtual void prepare(const Elem * elem, const THREAD_ID tid) override;
  virtual void prepareFace(const Elem * elem, const THREAD_ID tid) override;
  virtual void prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       const THREAD_ID tid) override;

  virtual void setCurrentSubdomainID(const Elem * elem, const THREAD_ID tid) override;
  virtual void
  setNeighborSubdomainID(const Elem * elem, unsigned int side, const THREAD_ID tid) override;
  virtual void setNeighborSubdomainID(const Elem * elem, const THREAD_ID tid);
  virtual void prepareAssembly(const THREAD_ID tid) override;

  virtual void addGhostedElem(dof_id_type elem_id) override;
  virtual void addGhostedBoundary(BoundaryID boundary_id) override;
  virtual void ghostGhostedBoundaries() override;

  virtual void sizeZeroes(unsigned int size, const THREAD_ID tid);
  virtual bool reinitDirac(const Elem * elem, const THREAD_ID tid) override;

  virtual void reinitElem(const Elem * elem, const THREAD_ID tid) override;
  virtual void reinitElemPhys(const Elem * elem,
                              const std::vector<Point> & phys_points_in_elem,
                              const THREAD_ID tid) override;
  void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID, const THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, const THREAD_ID tid) override;
  virtual void reinitLowerDElem(const Elem * lower_d_elem,
                                const THREAD_ID tid,
                                const std::vector<Point> * const pts = nullptr,
                                const std::vector<Real> * const weights = nullptr) override;
  virtual void reinitNode(const Node * node, const THREAD_ID tid) override;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, const THREAD_ID tid) override;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, const THREAD_ID tid) override;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes,
                                   const THREAD_ID tid) override;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, const THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  const THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  const THREAD_ID tid) override;
  virtual void
  reinitElemNeighborAndLowerD(const Elem * elem, unsigned int side, const THREAD_ID tid) override;
  virtual void reinitScalars(const THREAD_ID tid,
                             bool reinit_for_derivative_reordering = false) override;
  virtual void reinitOffDiagScalars(const THREAD_ID tid) override;

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems) override;
  virtual void clearDiracInfo() override;

  virtual void subdomainSetup(SubdomainID subdomain, const THREAD_ID tid);
  virtual void neighborSubdomainSetup(SubdomainID subdomain, const THREAD_ID tid);

  virtual void newAssemblyArray(std::vector<std::shared_ptr<SolverSystem>> & solver_systems);
  virtual void initNullSpaceVectors(const InputParameters & parameters,
                                    std::vector<std::shared_ptr<NonlinearSystemBase>> & nl);

  virtual void init() override;
  virtual void solve(const unsigned int nl_sys_num);

  /**
   * Build and solve a linear system
   * @param linear_sys_num The number of the linear system (1,..,num. of lin. systems)
   * @param po The petsc options for the solve, if not supplied, the defaults are used
   */
  virtual void solveLinearSystem(const unsigned int linear_sys_num,
                                 const Moose::PetscSupport::PetscOptions * po = nullptr);

  ///@{
  /**
   * In general, {evaluable elements} >= {local elements} U {algebraic ghosting elements}. That is,
   * the number of evaluable elements does NOT necessarily equal to the number of local and
   * algebraic ghosting elements. For example, if using a Lagrange basis for all variables,
   * if a non-local, non-algebraically-ghosted element is surrounded by neighbors which are
   * local or algebraically ghosted, then all the nodal (Lagrange) degrees of freedom associated
   * with the non-local, non-algebraically-ghosted element will be evaluable, and hence that
   * element will be considered evaluable.
   *
   * getNonlinearEvaluableElementRange() returns the evaluable element range based on the nonlinear
   * system dofmap;
   * getAuxliaryEvaluableElementRange() returns the evaluable element range based on the auxiliary
   * system dofmap;
   * getEvaluableElementRange() returns the element range that is evaluable based on both the
   * nonlinear dofmap and the auxliary dofmap.
   */
  const libMesh::ConstElemRange & getEvaluableElementRange();
  const libMesh::ConstElemRange & getNonlinearEvaluableElementRange();
  ///@}

  ///@{
  /**
   * These are the element and nodes that contribute to the jacobian and
   * residual for this local processor.
   *
   * getCurrentAlgebraicElementRange() returns the element range that contributes to the
   * system
   * getCurrentAlgebraicNodeRange() returns the node range that contributes to the
   * system
   * getCurrentAlgebraicBndNodeRange returns the boundary node ranges that contributes
   * to the system
   */
  const libMesh::ConstElemRange & getCurrentAlgebraicElementRange();
  const libMesh::ConstNodeRange & getCurrentAlgebraicNodeRange();
  const ConstBndNodeRange & getCurrentAlgebraicBndNodeRange();
  ///@}

  ///@{
  /**
   * These functions allow setting custom ranges for the algebraic elements, nodes,
   * and boundary nodes that contribute to the jacobian and residual for this local
   * processor.
   *
   * setCurrentAlgebraicElementRange() sets the element range that contributes to the
   * system. A nullptr will reset the range to use the mesh's range.
   *
   * setCurrentAlgebraicNodeRange() sets the node range that contributes to the
   * system. A nullptr will reset the range to use the mesh's range.
   *
   * setCurrentAlgebraicBndNodeRange() sets the boundary node range that contributes
   * to the system. A nullptr will reset the range to use the mesh's range.
   *
   * @param range A pointer to the const range object representing the algebraic
   *              elements, nodes, or boundary nodes.
   */
  void setCurrentAlgebraicElementRange(libMesh::ConstElemRange * range);
  void setCurrentAlgebraicNodeRange(libMesh::ConstNodeRange * range);
  void setCurrentAlgebraicBndNodeRange(ConstBndNodeRange * range);
  ///@}

  /**
   * Set an exception, which is stored at this point by toggling a member variable in
   * this class, and which must be followed up with by a call to
   * checkExceptionAndStopSolve().
   *
   * @param message The error message describing the exception, which will get printed
   *                when checkExceptionAndStopSolve() is called
   */
  virtual void setException(const std::string & message);

  /**
   * Whether or not an exception has occurred.
   */
  virtual bool hasException() { return _has_exception; }

  /**
   * Check to see if an exception has occurred on any processor and, if possible,
   * force the solve to fail, which will result in the time step being cut.
   *
   * Notes:
   *  * The exception have be registered by calling setException() prior to calling this.
   *  * This is collective on MPI, and must be called simultaneously by all processors!
   *  * If called when the solve can be interruped, it will do so and also throw a
   *    MooseException, which must be handled.
   *  * If called at a stage in the execution when the solve cannot be interupted (i.e.,
   *    there is no solve active), it will generate an error and terminate the application.
   *  * DO NOT CALL THIS IN A THREADED REGION! This is meant to be called just after a
   *    threaded section.
   *
   * @param print_message whether to print a message with exception information
   */
  virtual void checkExceptionAndStopSolve(bool print_message = true);

  virtual bool solverSystemConverged(const unsigned int solver_sys_num) override;
  virtual unsigned int nNonlinearIterations(const unsigned int nl_sys_num) const override;
  virtual unsigned int nLinearIterations(const unsigned int nl_sys_num) const override;
  virtual Real finalNonlinearResidual(const unsigned int nl_sys_num) const override;
  virtual bool computingPreSMOResidual(const unsigned int nl_sys_num) const override;

  /**
   * Return solver type as a human readable string
   */
  virtual std::string solverTypeString() { return Moose::stringify(solverParams()._type); }

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
  /**
   * Returns the time associated with the requested \p state
   */
  Real getTimeFromStateArg(const Moose::StateArg & state) const;

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
   * Declare that we need up to old (1) or older (2) solution states for a given type of iteration
   * @param oldest_needed oldest solution state needed
   * @param iteration_type the type of iteration for which old/older states are needed
   */
  void needSolutionState(unsigned int oldest_needed, Moose::SolutionIterationType iteration_type);

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
   * Reinitialize PETSc output for proper linear/nonlinear iteration display. This also may be used
   * for some PETSc-related solver settings
   */
  virtual void initPetscOutputAndSomeSolverSettings();

  /**
   * Retrieve a writable reference the PETSc options (used by PetscSupport)
   */
  Moose::PetscSupport::PetscOptions & getPetscOptions() { return _petsc_options; }

  /**
   * Output information about the object just added to the problem
   */
  void logAdd(const std::string & system,
              const std::string & name,
              const std::string & type,
              const InputParameters & params) const;

  // Function /////
  virtual void
  addFunction(const std::string & type, const std::string & name, InputParameters & parameters);
  virtual bool hasFunction(const std::string & name, const THREAD_ID tid = 0);
  virtual Function & getFunction(const std::string & name, const THREAD_ID tid = 0);

  /// Add a MeshDivision
  void
  addMeshDivision(const std::string & type, const std::string & name, InputParameters & params);
  /// Get a MeshDivision
  MeshDivision & getMeshDivision(const std::string & name, const THREAD_ID tid = 0) const;

  /// Adds a Convergence object
  virtual void
  addConvergence(const std::string & type, const std::string & name, InputParameters & parameters);
  /// Gets a Convergence object
  virtual Convergence & getConvergence(const std::string & name, const THREAD_ID tid = 0) const;
  /// Gets the Convergence objects
  virtual const std::vector<std::shared_ptr<Convergence>> &
  getConvergenceObjects(const THREAD_ID tid = 0) const;
  /// Returns true if the problem has a Convergence object of the given name
  virtual bool hasConvergence(const std::string & name, const THREAD_ID tid = 0) const;
  /// Returns true if the problem needs to add the default nonlinear convergence
  bool needToAddDefaultNonlinearConvergence() const
  {
    return _need_to_add_default_nonlinear_convergence;
  }
  /// Sets _need_to_add_default_nonlinear_convergence to true
  void setNeedToAddDefaultNonlinearConvergence()
  {
    _need_to_add_default_nonlinear_convergence = true;
  }
  /**
   * Adds the default nonlinear Convergence associated with the problem
   *
   * This is called if the user does not supply 'nonlinear_convergence'.
   *
   * @param[in] params   Parameters to apply to Convergence parameters
   */
  virtual void addDefaultNonlinearConvergence(const InputParameters & params);
  /**
   * Returns true if an error will result if the user supplies 'nonlinear_convergence'
   *
   * Some problems are strongly tied to their convergence, and it does not make
   * sense to use any convergence other than their default and additionally
   * would be error-prone.
   */
  virtual bool onlyAllowDefaultNonlinearConvergence() const { return false; }

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
  virtual Sampler & getSampler(const std::string & name, const THREAD_ID tid = 0);

  // NL /////
  NonlinearSystemBase & getNonlinearSystemBase(const unsigned int sys_num);
  const NonlinearSystemBase & getNonlinearSystemBase(const unsigned int sys_num) const;
  void setCurrentNonlinearSystem(const unsigned int nl_sys_num);
  NonlinearSystemBase & currentNonlinearSystem();
  const NonlinearSystemBase & currentNonlinearSystem() const;

  virtual const SystemBase & systemBaseNonlinear(const unsigned int sys_num) const override;
  virtual SystemBase & systemBaseNonlinear(const unsigned int sys_num) override;

  virtual const SystemBase & systemBaseAuxiliary() const override;
  virtual SystemBase & systemBaseAuxiliary() override;

  virtual NonlinearSystem & getNonlinearSystem(const unsigned int sys_num);

  /**
   * Get non-constant reference to a linear system
   * @param sys_num The number of the linear system
   */
  LinearSystem & getLinearSystem(unsigned int sys_num);

  /**
   * Get a constant reference to a linear system
   * @param sys_num The number of the linear system
   */
  const LinearSystem & getLinearSystem(unsigned int sys_num) const;

  /**
   * Get non-constant reference to a solver system
   * @param sys_num The number of the solver system
   */
  SolverSystem & getSolverSystem(unsigned int sys_num);

  /**
   * Get a constant reference to a solver system
   * @param sys_num The number of the solver system
   */
  const SolverSystem & getSolverSystem(unsigned int sys_num) const;

  /**
   * Set the current linear system pointer
   * @param sys_num The number of linear system
   */
  void setCurrentLinearSystem(unsigned int sys_num);

  /// Get a non-constant reference to the current linear system
  LinearSystem & currentLinearSystem();
  /// Get a constant reference to the current linear system
  const LinearSystem & currentLinearSystem() const;

  /**
   * Get a constant base class reference to a linear system
   * @param sys_num The number of the linear system
   */
  virtual const SystemBase & systemBaseLinear(unsigned int sys_num) const override;

  /**
   * Get a non-constant base class reference to a linear system
   * @param sys_num The number of the linear system
   */
  virtual SystemBase & systemBaseLinear(unsigned int sys_num) override;

  /**
   * Canonical method for adding a non-linear variable
   * @param var_type the type of the variable, e.g. MooseVariableScalar
   * @param var_name the variable name, e.g. 'u'
   * @param params the InputParameters from which to construct the variable
   */
  virtual void
  addVariable(const std::string & var_type, const std::string & var_name, InputParameters & params);

  virtual void addKernel(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters);
  virtual void addHDGKernel(const std::string & kernel_name,
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
  virtual void addHDGIntegratedBC(const std::string & kernel_name,
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
                              const libMesh::FEType & type,
                              const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxArrayVariable(const std::string & var_name,
                                   const libMesh::FEType & type,
                                   unsigned int components,
                                   const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxScalarVariable(const std::string & var_name,
                                    libMesh::Order order,
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
  // FV /////
  virtual void addFVKernel(const std::string & kernel_name,
                           const std::string & name,
                           InputParameters & parameters);

  virtual void addLinearFVKernel(const std::string & kernel_name,
                                 const std::string & name,
                                 InputParameters & parameters);
  virtual void
  addFVBC(const std::string & fv_bc_name, const std::string & name, InputParameters & parameters);
  virtual void addLinearFVBC(const std::string & fv_bc_name,
                             const std::string & name,
                             InputParameters & parameters);

  virtual void addFVInterfaceKernel(const std::string & fv_ik_name,
                                    const std::string & name,
                                    InputParameters & parameters);

  // Interface /////
  virtual void addInterfaceKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters);

  // IC /////
  virtual void addInitialCondition(const std::string & ic_name,
                                   const std::string & name,
                                   InputParameters & parameters);
  /**
   * Add an initial condition for a finite volume variables
   * @param ic_name The name of the boundary condition object
   * @param name The user-defined name from the input file
   * @param parameters The input parameters for construction
   */
  virtual void addFVInitialCondition(const std::string & ic_name,
                                     const std::string & name,
                                     InputParameters & parameters);

  void projectSolution();

  /**
   *  Retrieves the current initial condition state.
   * @return  current initial condition state
   */
  unsigned short getCurrentICState();

  /**
   * Project initial conditions for custom \p elem_range and \p bnd_node_range
   * This is needed when elements/boundary nodes are added to a specific subdomain
   * at an intermediate step
   */
  void projectInitialConditionOnCustomRange(libMesh::ConstElemRange & elem_range,
                                            ConstBndNodeRange & bnd_node_range);

  // Materials /////
  virtual void addMaterial(const std::string & material_name,
                           const std::string & name,
                           InputParameters & parameters);
  virtual void addMaterialHelper(std::vector<MaterialWarehouse *> warehouse,
                                 const std::string & material_name,
                                 const std::string & name,
                                 InputParameters & parameters);
  virtual void addInterfaceMaterial(const std::string & material_name,
                                    const std::string & name,
                                    InputParameters & parameters);
  virtual void addFunctorMaterial(const std::string & functor_material_name,
                                  const std::string & name,
                                  InputParameters & parameters);

  /**
   * Add the MooseVariables and the material properties that the current materials depend on to the
   * dependency list.
   * @param consumer_needed_mat_props The material properties needed by consumer objects (other than
   * the materials themselves)
   * @param blk_id The subdomain ID for which we are preparing our list of needed vars and props
   * @param tid The thread ID we are preparing the requirements for
   *
   * This MUST be done after the moose variable dependency list has been set for all the other
   * objects using the \p setActiveElementalMooseVariables API!
   */
  void prepareMaterials(const std::unordered_set<unsigned int> & consumer_needed_mat_props,
                        const SubdomainID blk_id,
                        const THREAD_ID tid);

  void reinitMaterials(SubdomainID blk_id, const THREAD_ID tid, bool swap_stateful = true);

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
  void reinitMaterialsFace(SubdomainID blk_id,
                           const THREAD_ID tid,
                           bool swap_stateful = true,
                           const std::deque<MaterialBase *> * reinit_mats = nullptr);

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
  void reinitMaterialsNeighbor(SubdomainID blk_id,
                               const THREAD_ID tid,
                               bool swap_stateful = true,
                               const std::deque<MaterialBase *> * reinit_mats = nullptr);

  /**
   * reinit materials on a boundary
   * @param boundary_id The boundary on which to reinit corresponding materials
   * @param tid The thread id
   * @param swap_stateful Whether to swap stateful material properties between \p MaterialData and
   * \p MaterialPropertyStorage
   * @param execute_stateful Whether to execute material objects that have stateful properties.
   * This should be \p false when for example executing material objects for mortar contexts in
   * which stateful properties don't make sense
   */
  void reinitMaterialsBoundary(BoundaryID boundary_id,
                               const THREAD_ID tid,
                               bool swap_stateful = true,
                               const std::deque<MaterialBase *> * reinit_mats = nullptr);

  void
  reinitMaterialsInterface(BoundaryID boundary_id, const THREAD_ID tid, bool swap_stateful = true);

  /*
   * Swap back underlying data storing stateful material properties
   */
  virtual void swapBackMaterials(const THREAD_ID tid);
  virtual void swapBackMaterialsFace(const THREAD_ID tid);
  virtual void swapBackMaterialsNeighbor(const THREAD_ID tid);

  /**
   * Record and set the material properties required by the current computing thread.
   * @param mat_prop_ids The set of material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  void setActiveMaterialProperties(const std::unordered_set<unsigned int> & mat_prop_ids,
                                   const THREAD_ID tid);

  /**
   * Method to check whether or not a list of active material roperties has been set. This method
   * is called by reinitMaterials to determine whether Material computeProperties methods need to be
   * called. If the return is False, this check prevents unnecessary material property computation
   * @param tid The thread id
   *
   * @return True if there has been a list of active material properties set, False otherwise
   */
  bool hasActiveMaterialProperties(const THREAD_ID tid) const;

  /**
   * Clear the active material properties. Should be called at the end of every computing thread
   *
   * @param tid The thread id
   */
  void clearActiveMaterialProperties(const THREAD_ID tid);

  /**
   * Method for creating and adding an object to the warehouse.
   *
   * @tparam T The base object type (registered in the Factory)
   * @param type String type of the object (registered in the Factory)
   * @param name Name for the object to be created
   * @param parameters InputParameters for the object
   * @param threaded Whether or not to create n_threads copies of the object
   * @param var_param_name The name of the parameter on the object which holds the primary variable.
   * @return A vector of shared_ptrs to the added objects
   */
  template <typename T>
  std::vector<std::shared_ptr<T>> addObject(const std::string & type,
                                            const std::string & name,
                                            InputParameters & parameters,
                                            const bool threaded = true,
                                            const std::string & var_param_name = "variable");

  // Postprocessors /////
  virtual void addPostprocessor(const std::string & pp_name,
                                const std::string & name,
                                InputParameters & parameters);

  // VectorPostprocessors /////
  virtual void addVectorPostprocessor(const std::string & pp_name,
                                      const std::string & name,
                                      InputParameters & parameters);

  /**
   * Add a Reporter object to the simulation.
   * @param type C++ object type to construct
   * @param name A uniquely identifying object name
   * @param parameters Complete parameters for the object to be created.
   *
   * For an example use, refer to AddReporterAction.C/h
   */
  virtual void
  addReporter(const std::string & type, const std::string & name, InputParameters & parameters);

  /**
   * Provides const access the ReporterData object.
   *
   * NOTE: There is a private non-const version of this function that uses a key object only
   *       constructable by the correct interfaces. This was done by design to encourage the use of
   *       the Reporter and ReporterInterface classes.
   */
  const ReporterData & getReporterData() const { return _reporter_data; }

  /**
   * Provides non-const access the ReporterData object that is used to store reporter values.
   *
   * see ReporterData.h
   */
  ReporterData & getReporterData(ReporterData::WriteKey /*key*/) { return _reporter_data; }

  // UserObjects /////
  virtual std::vector<std::shared_ptr<UserObject>> addUserObject(
      const std::string & user_object_name, const std::string & name, InputParameters & parameters);

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
    theWarehouse()
        .query()
        .condition<AttribSystem>("UserObject")
        .condition<AttribThread>(tid)
        .condition<AttribName>(name)
        .queryInto(objs);
    if (objs.empty())
      mooseError("Unable to find user object with name '" + name + "'");
    return *(objs[0]);
  }
  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @param tid The thread of the user object (defaults to 0)
   * @return Const reference to the user object
   */
  const UserObject & getUserObjectBase(const std::string & name, const THREAD_ID tid = 0) const;

  /**
   * Get the Positions object by its name
   * @param name The name of the Positions object being retrieved
   * @return Const reference to the Positions object
   */
  const Positions & getPositionsObject(const std::string & name) const;

  /**
   * Check if there if a user object of given name
   * @param name The name of the user object being checked for
   * @return true if the user object exists, false otherwise
   */
  bool hasUserObject(const std::string & name) const;

  /**
   * Whether or not a Postprocessor value exists by a given name.
   * @param name The name of the Postprocessor
   * @return True if a Postprocessor value exists
   *
   * Note: You should prioritize the use of PostprocessorInterface::hasPostprocessor
   * and PostprocessorInterface::hasPostprocessorByName over this method when possible.
   */
  bool hasPostprocessorValueByName(const PostprocessorName & name) const;

  /**
   * Get a read-only reference to the value associated with a Postprocessor that exists.
   * @param name The name of the post-processor
   * @param t_index Flag for getting current (0), old (1), or older (2) values
   * @return The reference to the value at the given time index
   *
   * Note: This method is only for retrieving values that already exist, the Postprocessor and
   *       PostprocessorInterface objects should be used rather than this method for creating
   *       and getting values within objects.
   */
  const PostprocessorValue & getPostprocessorValueByName(const PostprocessorName & name,
                                                         std::size_t t_index = 0) const;

  /**
   * Set the value of a PostprocessorValue.
   * @param name The name of the post-processor
   * @param t_index Flag for getting current (0), old (1), or older (2) values
   * @return The reference to the value at the given time index
   *
   * Note: This method is only for setting values that already exist, the Postprocessor and
   *       PostprocessorInterface objects should be used rather than this method for creating
   *       and getting values within objects.
   *
   * WARNING!
   * This method should be used with caution. It exists to allow Transfers and other
   * similar objects to modify Postprocessor values. It is not intended for general use.
   */
  void setPostprocessorValueByName(const PostprocessorName & name,
                                   const PostprocessorValue & value,
                                   std::size_t t_index = 0);

  /**
   * Deprecated. Use hasPostprocessorValueByName
   */
  bool hasPostprocessor(const std::string & name) const;

  /**
   * Get a read-only reference to the vector value associated with the VectorPostprocessor.
   * @param object_name The name of the VPP object.
   * @param vector_name The namve of the decalred vector within the object.
   * @return Referent to the vector of data.
   *
   * Note: This method is only for retrieving values that already exist, the VectorPostprocessor and
   *       VectorPostprocessorInterface objects should be used rather than this method for creating
   *       and getting values within objects.
   */
  const VectorPostprocessorValue &
  getVectorPostprocessorValueByName(const std::string & object_name,
                                    const std::string & vector_name,
                                    std::size_t t_index = 0) const;

  /**
   * Set the value of a VectorPostprocessor vector
   * @param object_name The name of the VPP object
   * @param vector_name The name of the declared vector
   * @param value The data to apply to the vector
   * @param t_index Flag for getting current (0), old (1), or older (2) values
   */
  void setVectorPostprocessorValueByName(const std::string & object_name,
                                         const std::string & vector_name,
                                         const VectorPostprocessorValue & value,
                                         std::size_t t_index = 0);

  /**
   * Return the VPP object given the name.
   * @param object_name The name of the VPP object
   * @return Desired VPP object
   *
   * This is used by various output objects as well as the scatter value handling.
   * @see CSV.C, XMLOutput.C, VectorPostprocessorInterface.C
   */
  const VectorPostprocessor & getVectorPostprocessorObjectByName(const std::string & object_name,
                                                                 const THREAD_ID tid = 0) const;

  ///@{
  /**
   * Returns whether or not the current simulation has any multiapps
   */
  bool hasMultiApps() const { return _multi_apps.hasActiveObjects(); }
  bool hasMultiApps(ExecFlagType type) const;
  bool hasMultiApp(const std::string & name) const;
  ///@}

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
   * Execute MultiAppTransfers associated with execution flag and direction.
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

  /**
   * Computes the residual of a nonlinear system using whatever is sitting in the current
   * solution vector then returns the L2 norm.
   */
  Real computeResidualL2Norm(NonlinearSystemBase & sys);

  /**
   * Computes the residual of a linear system using whatever is sitting in the current
   * solution vector then returns the L2 norm.
   */
  Real computeResidualL2Norm(LinearSystem & sys);

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
  virtual void computeResidualSys(libMesh::NonlinearImplicitSystem & sys,
                                  const NumericVector<libMesh::Number> & soln,
                                  NumericVector<libMesh::Number> & residual);
  /**
   * This function is called by Libmesh to form a residual. This is deprecated.
   * We should remove this as soon as RattleSnake is fixed.
   */
  void computeResidual(libMesh::NonlinearImplicitSystem & sys,
                       const NumericVector<libMesh::Number> & soln,
                       NumericVector<libMesh::Number> & residual);

  /**
   * Form a residual with default tags (nontime, time, residual).
   */
  virtual void computeResidual(const NumericVector<libMesh::Number> & soln,
                               NumericVector<libMesh::Number> & residual,
                               const unsigned int nl_sys_num);

  /**
   * Form a residual and Jacobian with default tags
   */
  void computeResidualAndJacobian(const NumericVector<libMesh::Number> & soln,
                                  NumericVector<libMesh::Number> & residual,
                                  libMesh::SparseMatrix<libMesh::Number> & jacobian);

  /**
   * Form a residual vector for a given tag
   */
  virtual void computeResidualTag(const NumericVector<libMesh::Number> & soln,
                                  NumericVector<libMesh::Number> & residual,
                                  TagID tag);
  /**
   * Form a residual vector for a given tag and "residual" tag
   */
  virtual void computeResidualType(const NumericVector<libMesh::Number> & soln,
                                   NumericVector<libMesh::Number> & residual,
                                   TagID tag);

  /**
   * Form a residual vector for a set of tags. It should not be called directly
   * by users.
   */
  virtual void computeResidualInternal(const NumericVector<libMesh::Number> & soln,
                                       NumericVector<libMesh::Number> & residual,
                                       const std::set<TagID> & tags);
  /**
   * Form multiple residual vectors and each is associated with one tag
   */
  virtual void computeResidualTags(const std::set<TagID> & tags);

  /**
   * Form a Jacobian matrix. It is called by Libmesh.
   */
  virtual void computeJacobianSys(libMesh::NonlinearImplicitSystem & sys,
                                  const NumericVector<libMesh::Number> & soln,
                                  libMesh::SparseMatrix<libMesh::Number> & jacobian);
  /**
   * Form a Jacobian matrix with the default tag (system).
   */
  virtual void computeJacobian(const NumericVector<libMesh::Number> & soln,
                               libMesh::SparseMatrix<libMesh::Number> & jacobian,
                               const unsigned int nl_sys_num);

  /**
   * Form a Jacobian matrix for a given tag.
   */
  virtual void computeJacobianTag(const NumericVector<libMesh::Number> & soln,
                                  libMesh::SparseMatrix<libMesh::Number> & jacobian,
                                  TagID tag);

  /**
   * Form a Jacobian matrix for multiple tags. It should not be called directly by users.
   */
  virtual void computeJacobianInternal(const NumericVector<libMesh::Number> & soln,
                                       libMesh::SparseMatrix<libMesh::Number> & jacobian,
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
  virtual void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks,
                                     const unsigned int nl_sys_num);

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
  virtual void computeJacobianBlock(libMesh::SparseMatrix<libMesh::Number> & jacobian,
                                    libMesh::System & precond_system,
                                    unsigned int ivar,
                                    unsigned int jvar);

  /**
   * Assemble both the right hand side and the system matrix of a given linear
   * system.
   * @param sys The linear system which should be assembled
   * @param system_matrix The sparse matrix which should hold the system matrix
   * @param rhs The vector which should hold the right hand side
   * @param compute_gradients A flag to disable the computation of new gradients during the
   * assembly, can be used to lag gradients
   */
  void computeLinearSystemSys(libMesh::LinearImplicitSystem & sys,
                              libMesh::SparseMatrix<libMesh::Number> & system_matrix,
                              NumericVector<libMesh::Number> & rhs,
                              const bool compute_gradients = true);

  /**
   * Assemble the current linear system given a set of vector and matrix tags.
   *
   * @param soln The solution which should be used for the system assembly
   * @param system_matrix The sparse matrix which should hold the system matrix
   * @param rhs The vector which should hold the right hand side
   * @param vector_tags The vector tags for the right hand side
   * @param matrix_tags The matrix tags for the matrix
   * @param compute_gradients A flag to disable the computation of new gradients during the
   * assembly, can be used to lag gradients
   */
  void computeLinearSystemTags(const NumericVector<libMesh::Number> & soln,
                               libMesh::SparseMatrix<libMesh::Number> & system_matrix,
                               NumericVector<libMesh::Number> & rhs,
                               const std::set<TagID> & vector_tags,
                               const std::set<TagID> & matrix_tags,
                               const bool compute_gradients = true);

  virtual Real computeDamping(const NumericVector<libMesh::Number> & soln,
                              const NumericVector<libMesh::Number> & update);

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
  virtual bool updateSolution(NumericVector<libMesh::Number> & vec_solution,
                              NumericVector<libMesh::Number> & ghosted_solution);

  /**
   * Perform cleanup tasks after application of predictor to solution vector
   * @param ghosted_solution  Ghosted solution vector
   */
  virtual void predictorCleanup(NumericVector<libMesh::Number> & ghosted_solution);

  virtual void computeBounds(libMesh::NonlinearImplicitSystem & sys,
                             NumericVector<libMesh::Number> & lower,
                             NumericVector<libMesh::Number> & upper);
  virtual void computeNearNullSpace(libMesh::NonlinearImplicitSystem & sys,
                                    std::vector<NumericVector<libMesh::Number> *> & sp);
  virtual void computeNullSpace(libMesh::NonlinearImplicitSystem & sys,
                                std::vector<NumericVector<libMesh::Number> *> & sp);
  virtual void computeTransposeNullSpace(libMesh::NonlinearImplicitSystem & sys,
                                         std::vector<NumericVector<libMesh::Number> *> & sp);
  virtual void computePostCheck(libMesh::NonlinearImplicitSystem & sys,
                                const NumericVector<libMesh::Number> & old_soln,
                                NumericVector<libMesh::Number> & search_direction,
                                NumericVector<libMesh::Number> & new_soln,
                                bool & changed_search_direction,
                                bool & changed_new_soln);

  virtual void computeIndicatorsAndMarkers();
  virtual void computeIndicators();
  virtual void computeMarkers();

  virtual void addResidual(const THREAD_ID tid) override;
  virtual void addResidualNeighbor(const THREAD_ID tid) override;
  virtual void addResidualLower(const THREAD_ID tid) override;
  virtual void addResidualScalar(const THREAD_ID tid = 0);

  virtual void cacheResidual(const THREAD_ID tid) override;
  virtual void cacheResidualNeighbor(const THREAD_ID tid) override;
  virtual void addCachedResidual(const THREAD_ID tid) override;

  /**
   * Allows for all the residual contributions that are currently cached to be added directly into
   * the vector passed in.
   *
   * @param residual The vector to add the cached contributions to.
   * @param tid The thread id.
   */
  virtual void addCachedResidualDirectly(NumericVector<libMesh::Number> & residual,
                                         const THREAD_ID tid);

  virtual void setResidual(NumericVector<libMesh::Number> & residual, const THREAD_ID tid) override;
  virtual void setResidualNeighbor(NumericVector<libMesh::Number> & residual,
                                   const THREAD_ID tid) override;

  virtual void addJacobian(const THREAD_ID tid) override;
  virtual void addJacobianNeighbor(const THREAD_ID tid) override;
  virtual void addJacobianNeighborLowerD(const THREAD_ID tid) override;
  virtual void addJacobianLowerD(const THREAD_ID tid) override;
  virtual void addJacobianBlockTags(libMesh::SparseMatrix<libMesh::Number> & jacobian,
                                    unsigned int ivar,
                                    unsigned int jvar,
                                    const DofMap & dof_map,
                                    std::vector<dof_id_type> & dof_indices,
                                    const std::set<TagID> & tags,
                                    const THREAD_ID tid);
  virtual void addJacobianNeighbor(libMesh::SparseMatrix<libMesh::Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   const std::set<TagID> & tags,
                                   const THREAD_ID tid) override;
  virtual void addJacobianScalar(const THREAD_ID tid = 0);
  virtual void addJacobianOffDiagScalar(unsigned int ivar, const THREAD_ID tid = 0);

  virtual void cacheJacobian(const THREAD_ID tid) override;
  virtual void cacheJacobianNeighbor(const THREAD_ID tid) override;
  virtual void addCachedJacobian(const THREAD_ID tid) override;

  virtual void prepareShapes(unsigned int var, const THREAD_ID tid) override;
  virtual void prepareFaceShapes(unsigned int var, const THREAD_ID tid) override;
  virtual void prepareNeighborShapes(unsigned int var, const THREAD_ID tid) override;

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
      bool periodic,
      const bool debug,
      const bool correct_edge_dropping,
      const Real minimum_projection_angle);

  /**
   * Return the undisplaced or displaced mortar generation object associated with the provided
   * boundaries and subdomains
   */
  ///@{
  const AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
                     const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
                     bool on_displaced) const;

  AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & primary_secondary_boundary_pair,
                     const std::pair<SubdomainID, SubdomainID> & primary_secondary_subdomain_pair,
                     bool on_displaced);
  ///@}

  const std::unordered_map<std::pair<BoundaryID, BoundaryID>, AutomaticMortarGeneration> &
  getMortarInterfaces(bool on_displaced) const;

  virtual void possiblyRebuildGeomSearchPatches();

  virtual GeometricSearchData & geomSearchData() override { return _geometric_search_data; }

  /**
   * Communicate to the Resurector the name of the restart filer
   * @param file_name The file name for restarting from
   */
  void setRestartFile(const std::string & file_name);

  /**
   * @return A reference to the material property registry
   */
  const MaterialPropertyRegistry & getMaterialPropertyRegistry() const
  {
    return _material_prop_registry;
  }

  /**
   * Return a reference to the material property storage
   * @return A const reference to the material property storage
   */
  ///@{
  const MaterialPropertyStorage & getMaterialPropertyStorage() { return _material_props; }
  const MaterialPropertyStorage & getBndMaterialPropertyStorage() { return _bnd_material_props; }
  const MaterialPropertyStorage & getNeighborMaterialPropertyStorage()
  {
    return _neighbor_material_props;
  }
  ///@}

  /**
   * Return indicator/marker storage.
   */
  ///@{
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
   * Return FVInitialCondition storage
   */
  const FVInitialConditionWarehouse & getFVInitialConditionWarehouse() const { return _fv_ics; }

  /**
   * Get the solver parameters
   */
  SolverParams & solverParams();

  /**
   * const version
   */
  const SolverParams & solverParams() const;

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
   * Initialize stateful properties for elements in a specific \p elem_range
   * This is needed when elements/boundary nodes are added to a specific subdomain
   * at an intermediate step
   */
  void initElementStatefulProps(const libMesh::ConstElemRange & elem_range, const bool threaded);

  /**
   * Method called to perform a series of sanity checks before a simulation is run. This method
   * doesn't return when errors are found, instead it generally calls mooseError() directly.
   */
  virtual void checkProblemIntegrity();

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
  void setKernelCoverageCheck(CoverageCheckMode mode) { _kernel_coverage_check = mode; }

  /**
   * Set flag to indicate whether kernel coverage checks should be performed. This check makes
   * sure that at least one kernel is active on all subdomains in the domain (default: true).
   */
  void setKernelCoverageCheck(bool flag)
  {
    _kernel_coverage_check = flag ? CoverageCheckMode::TRUE : CoverageCheckMode::FALSE;
  }

  /**
   * Set flag to indicate whether material coverage checks should be performed. This check makes
   * sure that at least one material is active on all subdomains in the domain if any material is
   * supplied. If no materials are supplied anywhere, a simulation is still considered OK as long as
   * no properties are being requested anywhere.
   */
  void setMaterialCoverageCheck(CoverageCheckMode mode) { _material_coverage_check = mode; }

  /**
   * Set flag to indicate whether material coverage checks should be performed. This check makes
   * sure that at least one material is active on all subdomains in the domain if any material is
   * supplied. If no materials are supplied anywhere, a simulation is still considered OK as long as
   * no properties are being requested anywhere.
   */
  void setMaterialCoverageCheck(bool flag)
  {
    _material_coverage_check = flag ? CoverageCheckMode::TRUE : CoverageCheckMode::FALSE;
  }

  /**
   * Toggle parallel barrier messaging (defaults to on).
   */
  void setParallelBarrierMessaging(bool flag) { _parallel_barrier_messaging = flag; }

  /// Make the problem be verbose
  void setVerboseProblem(bool verbose);

  /**
   * Whether or not to use verbose printing for MultiApps.
   */
  bool verboseMultiApps() const { return _verbose_multiapps; }

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
  bool needBoundaryMaterialOnSide(BoundaryID bnd_id, const THREAD_ID tid);
  bool needInterfaceMaterialOnSide(BoundaryID bnd_id, const THREAD_ID tid);
  bool needSubdomainMaterialOnSide(SubdomainID subdomain_id, const THREAD_ID tid);
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
                                            const THREAD_ID tid = 0,
                                            bool no_warn = false);

  /*
   * @return The MaterialData for the type \p type for thread \p tid
   */
  MaterialData & getMaterialData(Moose::MaterialDataType type, const THREAD_ID tid = 0);

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

  /**
   * Will return True if the executioner in use requires preserving the sparsity pattern of the
   * matrices being formed during the solve. This is usually the Jacobian.
   */
  bool preserveMatrixSparsityPattern() const { return _preserve_matrix_sparsity_pattern; };

  /// Set whether the sparsity pattern of the matrices being formed during the solve (usually the Jacobian)
  /// should be preserved. This global setting can be retrieved by kernels, notably those using AD, to decide
  /// whether to take additional care to preserve the sparsity pattern
  void setPreserveMatrixSparsityPattern(bool preserve);

  /**
   * Will return true if zeros in the Jacobian are to be dropped from the sparsity pattern.
   * Note that this can make preserving the matrix sparsity pattern impossible.
   */
  bool ignoreZerosInJacobian() const { return _ignore_zeros_in_jacobian; }

  /// Set whether the zeros in the Jacobian should be dropped from the sparsity pattern
  void setIgnoreZerosInJacobian(bool state) { _ignore_zeros_in_jacobian = state; }

  /**
   * Whether or not to accept the solution based on its invalidity.
   *
   * If this returns false, it means that an invalid solution was encountered
   * (an error) that was not allowed.
   */
  bool acceptInvalidSolution() const;
  /**
   * Whether to accept / allow an invalid solution
   */
  bool allowInvalidSolution() const { return _allow_invalid_solution; }

  /**
   * Whether or not to print out the invalid solutions summary table in console
   */
  bool showInvalidSolutionConsole() const { return _show_invalid_solution_console; }

  /**
   * Whether or not the solution invalid warnings are printed out immediately
   */
  bool immediatelyPrintInvalidSolution() const { return _immediately_print_invalid_solution; }

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
  virtual void executeAllObjects(const ExecFlagType & exec_type);

  virtual Executor & getExecutor(const std::string & name) { return _app.getExecutor(name); }

  /**
   * Call compute methods on UserObjects.
   */
  virtual void computeUserObjects(const ExecFlagType & type, const Moose::AuxGroup & group);

  /**
   * Compute an user object with the given name
   */
  virtual void computeUserObjectByName(const ExecFlagType & type,
                                       const Moose::AuxGroup & group,
                                       const std::string & name);

  /**
   * Set a flag that indicated that user required values for the previous Newton iterate
   */
  void needsPreviousNewtonIteration(bool state);

  /**
   * Check to see whether we need to compute the variable values of the previous Newton iterate
   * @return true if the user required values of the previous Newton iterate
   */
  bool needsPreviousNewtonIteration() const;

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
   * If PETSc options are already inserted
   */
  bool & petscOptionsInserted() { return _is_petsc_options_inserted; }

#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  PetscOptions & petscOptionsDatabase() { return _petsc_option_data_base; }
#endif

  /// Set boolean flag to true to store solution time derivative
  virtual void setUDotRequested(const bool u_dot_requested) { _u_dot_requested = u_dot_requested; }

  /// Set boolean flag to true to store solution second time derivative
  virtual void setUDotDotRequested(const bool u_dotdot_requested)
  {
    _u_dotdot_requested = u_dotdot_requested;
  }

  /// Set boolean flag to true to store old solution time derivative
  virtual void setUDotOldRequested(const bool u_dot_old_requested)
  {
    _u_dot_old_requested = u_dot_old_requested;
  }

  /// Set boolean flag to true to store old solution second time derivative
  virtual void setUDotDotOldRequested(const bool u_dotdot_old_requested)
  {
    _u_dotdot_old_requested = u_dotdot_old_requested;
  }

  /// Get boolean flag to check whether solution time derivative needs to be stored
  virtual bool uDotRequested() { return _u_dot_requested; }

  /// Get boolean flag to check whether solution second time derivative needs to be stored
  virtual bool uDotDotRequested() { return _u_dotdot_requested; }

  /// Get boolean flag to check whether old solution time derivative needs to be stored
  virtual bool uDotOldRequested()
  {
    if (_u_dot_old_requested && !_u_dot_requested)
      mooseError("FEProblemBase: When requesting old time derivative of solution, current time "
                 "derivative of solution should also be stored. Please set `u_dot_requested` to "
                 "true using setUDotRequested.");

    return _u_dot_old_requested;
  }

  /// Get boolean flag to check whether old solution second time derivative needs to be stored
  virtual bool uDotDotOldRequested()
  {
    if (_u_dotdot_old_requested && !_u_dotdot_requested)
      mooseError("FEProblemBase: When requesting old second time derivative of solution, current "
                 "second time derivation of solution should also be stored. Please set "
                 "`u_dotdot_requested` to true using setUDotDotRequested.");
    return _u_dotdot_old_requested;
  }

  using SubProblem::haveADObjects;
  void haveADObjects(bool have_ad_objects) override;

  // Whether or not we should solve this system
  bool shouldSolve() const { return _solve; }

  /**
   * Returns the mortar data object
   */
  const MortarData & mortarData() const { return _mortar_data; }
  MortarData & mortarData() { return _mortar_data; }

  /**
   * Whether the simulation has neighbor coupling
   */
  virtual bool hasNeighborCoupling() const { return _has_internal_edge_residual_objects; }

  /**
   * Whether the simulation has mortar coupling
   */
  virtual bool hasMortarCoupling() const { return _has_mortar; }

  using SubProblem::computingNonlinearResid;
  void computingNonlinearResid(bool computing_nonlinear_residual) final;

  using SubProblem::currentlyComputingResidual;
  void setCurrentlyComputingResidual(bool currently_computing_residual) final;

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
                                 Real tolerance,
                                 const std::vector<Point> * const pts,
                                 const std::vector<Real> * const weights = nullptr,
                                 const THREAD_ID tid = 0) override;

  /**
   * reinitialize FE objects on a given neighbor element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitNeighborFaceRef(const Elem * neighbor_elem,
                                     unsigned int neighbor_side,
                                     Real tolerance,
                                     const std::vector<Point> * const pts,
                                     const std::vector<Real> * const weights = nullptr,
                                     const THREAD_ID tid = 0) override;

  /**
   * @return whether to perform a boundary condition integrity check for finite volume
   */
  bool fvBCsIntegrityCheck() const { return _fv_bcs_integrity_check; }

  /**
   * @param fv_bcs_integrity_check Whether to perform a boundary condition integrity check for
   * finite volume
   */
  void fvBCsIntegrityCheck(bool fv_bcs_integrity_check);

  /**
   * Get the materials and variables potentially needed for FV
   * @param block_id SubdomainID The subdomain id that we want to retrieve materials for
   * @param face_materials The face materials container that we will fill
   * @param neighbor_materials The neighbor materials container that we will fill
   * @param variables The variables container that we will fill that our materials depend on
   * @param tid The thread id
   */
  void getFVMatsAndDependencies(SubdomainID block_id,
                                std::vector<std::shared_ptr<MaterialBase>> & face_materials,
                                std::vector<std::shared_ptr<MaterialBase>> & neighbor_materials,
                                std::set<MooseVariableFieldBase *> & variables,
                                const THREAD_ID tid);

  /**
   * Resize material data
   * @param data_type The type of material data to resize
   * @param nqp The number of quadrature points to resize for
   * @param tid The thread ID
   */
  void resizeMaterialData(Moose::MaterialDataType data_type, unsigned int nqp, const THREAD_ID tid);

  bool haveDisplaced() const override final { return _displaced_problem.get(); }

  /**
   * Sets the nonlinear convergence object name if there is one
   */
  void setNonlinearConvergenceNames(const std::vector<ConvergenceName> & convergence_names);

  /**
   * Gets the nonlinear convergence object name(s).
   */
  std::vector<ConvergenceName> getNonlinearConvergenceNames() const;

  /**
   * Setter for whether we're computing the scaling jacobian
   */
  void computingScalingJacobian(bool computing_scaling_jacobian)
  {
    _computing_scaling_jacobian = computing_scaling_jacobian;
  }

  bool computingScalingJacobian() const override final { return _computing_scaling_jacobian; }

  /**
   * Setter for whether we're computing the scaling residual
   */
  void computingScalingResidual(bool computing_scaling_residual)
  {
    _computing_scaling_residual = computing_scaling_residual;
  }

  /**
   * @return whether we are currently computing a residual for automatic scaling purposes
   */
  bool computingScalingResidual() const override final { return _computing_scaling_residual; }

  /**
   * @return the coordinate transformation object that describes how to transform this problem's
   * coordinate system into the canonical/reference coordinate system
   */
  MooseAppCoordTransform & coordTransform();

  virtual std::size_t numNonlinearSystems() const override { return _num_nl_sys; }

  virtual std::size_t numLinearSystems() const override { return _num_linear_sys; }

  virtual std::size_t numSolverSystems() const override { return _num_nl_sys + _num_linear_sys; }

  /// Check if the solver system is nonlinear
  bool isSolverSystemNonlinear(const unsigned int sys_num) { return sys_num < _num_nl_sys; }

  virtual unsigned int currentNlSysNum() const override;

  virtual unsigned int currentLinearSysNum() const override;

  /**
   * @return the nonlinear system number corresponding to the provided \p nl_sys_name
   */
  virtual unsigned int nlSysNum(const NonlinearSystemName & nl_sys_name) const override;

  /**
   * @return the linear system number corresponding to the provided \p linear_sys_name
   */
  unsigned int linearSysNum(const LinearSystemName & linear_sys_name) const override;

  /**
   * @return the solver system number corresponding to the provided \p solver_sys_name
   */
  unsigned int solverSysNum(const SolverSystemName & solver_sys_name) const override;

  /**
   * @return the system number for the provided \p variable_name
   * Can be nonlinear or auxiliary
   */
  unsigned int systemNumForVariable(const VariableName & variable_name) const;

  /**
   * Whether it will skip further residual evaluations and fail the next nonlinear convergence check
   */
  bool getFailNextNonlinearConvergenceCheck() const
  {
    return _fail_next_nonlinear_convergence_check;
  }

  /**
   * Skip further residual evaluations and fail the next nonlinear convergence check
   */
  void setFailNextNonlinearConvergenceCheck() { _fail_next_nonlinear_convergence_check = true; }

  /**
   * Do not skip further residual evaluations and fail the next nonlinear convergence check
   */
  void resetFailNextNonlinearConvergenceCheck() { _fail_next_nonlinear_convergence_check = false; }
  /*
   * Set the status of loop order of execution printing
   * @param print_exec set of execution flags to print on
   */
  void setExecutionPrinting(const ExecFlagEnum & print_exec) { _print_execution_on = print_exec; }

  /**
   * Check whether the problem should output execution orders at this time
   */
  bool shouldPrintExecution(const THREAD_ID tid) const;
  /**
   * Call \p reinit on mortar user objects with matching primary boundary ID, secondary boundary ID,
   * and displacement characteristics
   */
  void reinitMortarUserObjects(BoundaryID primary_boundary_id,
                               BoundaryID secondary_boundary_id,
                               bool displaced);

  virtual const std::vector<VectorTag> & currentResidualVectorTags() const override;

  /**
   * Class that is used as a parameter to set/clearCurrentResidualVectorTags that allows only
   * blessed classes to call said methods
   */
  class CurrentResidualVectorTagsKey
  {
    friend class CrankNicolson;
    friend class FEProblemBase;
    CurrentResidualVectorTagsKey() {}
    CurrentResidualVectorTagsKey(const CurrentResidualVectorTagsKey &) {}
  };

  /**
   * Set the current residual vector tag data structure based on the passed in tag IDs
   */
  void setCurrentResidualVectorTags(const std::set<TagID> & vector_tags);

  /**
   * Clear the current residual vector tag data structure
   */
  void clearCurrentResidualVectorTags();

  /**
   * Clear the current Jacobian matrix tag data structure ... if someone creates it
   */
  void clearCurrentJacobianMatrixTags() {}

  using SubProblem::doingPRefinement;
  virtual void doingPRefinement(bool doing_p_refinement,
                                const MultiMooseEnum & disable_p_refinement_for_families) override;

  virtual void needFV() override { _have_fv = true; }
  virtual bool haveFV() const override { return _have_fv; }

  virtual bool hasNonlocalCoupling() const override { return _has_nonlocal_coupling; }

  /**
   * Whether to identify variable groups in nonlinear systems. This affects dof ordering
   */
  bool identifyVariableGroupsInNL() const { return _identify_variable_groups_in_nl; }

  virtual void setCurrentLowerDElem(const Elem * const lower_d_elem, const THREAD_ID tid) override;
  virtual void setCurrentBoundaryID(BoundaryID bid, const THREAD_ID tid) override;

  /**
   * @returns the nolinear system names in the problem
   */
  const std::vector<NonlinearSystemName> & getNonlinearSystemNames() const { return _nl_sys_names; }
  /**
   * @returns the linear system names in the problem
   */
  const std::vector<LinearSystemName> & getLinearSystemNames() const { return _linear_sys_names; }

protected:
  /// Create extra tagged vectors and matrices
  void createTagVectors();

  /// Create extra tagged solution vectors
  void createTagSolutions();

  /**
   * Do generic system computations
   */
  void computeSystems(const ExecFlagType & type);

  MooseMesh & _mesh;

private:
  /// The EquationSystems object, wrapped for restart
  Restartable::ManagedValue<RestartableEquationSystems> _req;

  /**
   * Set the subproblem and system parameters for residual objects and log their addition
   * @param ro_name The type of the residual object
   * @param name The name of the residual object
   * @param parameters The residual object parameters
   * @param nl_sys_num The nonlinear system that the residual object belongs to
   * @param base_name The base type of the residual object, e.g. Kernel, BoundaryCondition, etc.
   * @param reinit_displaced A data member indicating whether a geometric concept should be reinit'd
   * for the displaced problem. Examples of valid data members to pass in are \p
   * _reinit_displaced_elem and \p _reinit_displaced_face
   */
  void setResidualObjectParamsAndLog(const std::string & ro_name,
                                     const std::string & name,
                                     InputParameters & params,
                                     const unsigned int nl_sys_num,
                                     const std::string & base_name,
                                     bool & reinit_displaced);

protected:
  bool _initialized;

  /// Nonlinear system(s) convergence name(s)
  std::vector<ConvergenceName> _nonlinear_convergence_names;

  std::set<TagID> _fe_vector_tags;

  std::set<TagID> _fe_matrix_tags;

  /// Temporary storage for filtered vector tags for linear systems
  std::set<TagID> _linear_vector_tags;

  /// Temporary storage for filtered matrix tags for linear systems
  std::set<TagID> _linear_matrix_tags;

  /// Whether or not to actually solve the nonlinear system
  const bool & _solve;

  bool _transient;
  Real & _time;
  Real & _time_old;
  int & _t_step;
  Real & _dt;
  Real & _dt_old;

  /// Flag that the nonlinear convergence name has been set
  bool _set_nonlinear_convergence_names;
  /// Flag that the problem needs to add the default nonlinear convergence
  bool _need_to_add_default_nonlinear_convergence;

  /// The linear system names
  const std::vector<LinearSystemName> _linear_sys_names;

  /// The number of linear systems
  const std::size_t _num_linear_sys;

  /// The vector of linear systems
  std::vector<std::shared_ptr<LinearSystem>> _linear_systems;

  /// Map from linear system name to number
  std::map<LinearSystemName, unsigned int> _linear_sys_name_to_num;

  /// The current linear system that we are solving
  LinearSystem * _current_linear_sys;

  /// Boolean to check if we have the default nonlinear system
  const bool _using_default_nl;

  /// The nonlinear system names
  const std::vector<NonlinearSystemName> _nl_sys_names;

  /// The number of nonlinear systems
  const std::size_t _num_nl_sys;

  /// The nonlinear systems
  std::vector<std::shared_ptr<NonlinearSystemBase>> _nl;

  /// Map from nonlinear system name to number
  std::map<NonlinearSystemName, unsigned int> _nl_sys_name_to_num;

  /// The current nonlinear system that we are solving
  NonlinearSystemBase * _current_nl_sys;

  /// The current solver system
  SolverSystem * _current_solver_sys;

  /// Combined container to base pointer of every solver system
  std::vector<std::shared_ptr<SolverSystem>> _solver_systems;

  /// Map connecting variable names with their respective solver systems
  std::map<SolverVariableName, unsigned int> _solver_var_to_sys_num;

  /// Map connecting solver system names with their respective systems
  std::map<SolverSystemName, unsigned int> _solver_sys_name_to_num;

  /// The union of nonlinear and linear system names
  std::vector<std::string> _solver_sys_names;

  /// The auxiliary system
  std::shared_ptr<AuxiliarySystem> _aux;

  Moose::CouplingType _coupling;                             ///< Type of variable coupling
  std::vector<std::unique_ptr<libMesh::CouplingMatrix>> _cm; ///< Coupling matrix for variables.

  /// Dimension of the subspace spanned by the vectors with a given prefix
  std::map<std::string, unsigned int> _subspace_dim;

  /// The Assembly objects. The first index corresponds to the thread ID and the second index
  /// corresponds to the nonlinear system number
  std::vector<std::vector<std::unique_ptr<Assembly>>> _assembly;

  /// Warehouse to store mesh divisions
  /// NOTE: this could probably be moved to the MooseMesh instead of the Problem
  /// Time (and people's uses) will tell where this fits best
  MooseObjectWarehouse<MeshDivision> _mesh_divisions;

  /// functions
  MooseObjectWarehouse<Function> _functions;

  /// convergence warehouse
  MooseObjectWarehouse<Convergence> _convergences;

  /// nonlocal kernels
  MooseObjectWarehouse<KernelBase> _nonlocal_kernels;

  /// nonlocal integrated_bcs
  MooseObjectWarehouse<IntegratedBCBase> _nonlocal_integrated_bcs;

  ///@{
  /// Initial condition storage
  InitialConditionWarehouse _ics;
  FVInitialConditionWarehouse _fv_ics;
  ScalarInitialConditionWarehouse _scalar_ics; // use base b/c of setup methods
  ///@}

protected:
  // material properties
  MaterialPropertyRegistry _material_prop_registry;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;
  MaterialPropertyStorage & _neighbor_material_props;

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

  // Helper class to access Reporter object values
  ReporterData _reporter_data;

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

  /// Transfers executed just before MultiApps to transfer data between them
  ExecuteMooseObjectWarehouse<Transfer> _between_multi_app_transfers;

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
  bool duplicateVariableCheck(const std::string & var_name,
                              const libMesh::FEType & type,
                              bool is_aux,
                              const std::set<SubdomainID> * const active_subdomains);

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
   * @param mortar_changed Whether an update of mortar data has been requested since the last
   * EquationSystems (re)initialization
   */
  void reinitBecauseOfGhostingOrNewGeomObjects(bool mortar_changed = false);

  /**
   * Helper for setting the "_subproblem" and "_sys" parameters in addObject() and
   * in addUserObject().
   *
   * This is needed due to header includes/forward declaration issues
   */
  void addObjectParamsHelper(InputParameters & params,
                             const std::string & object_name,
                             const std::string & var_param_name = "variable");

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

  /// Whether to call DisplacedProblem::reinitElem when this->reinitElem is called
  bool _reinit_displaced_elem;
  /// Whether to call DisplacedProblem::reinitElemFace when this->reinitElemFace is called
  bool _reinit_displaced_face;
  /// Whether to call DisplacedProblem::reinitNeighbor when this->reinitNeighbor is called
  bool _reinit_displaced_neighbor;

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

  /// true if the Jacobian is constant
  bool _const_jacobian;

  /// Indicates if the Jacobian was computed
  bool _has_jacobian;

  /// Indicates that we need to compute variable values for previous Newton iteration
  bool _needs_old_newton_iter;

  /// Indicates we need to save the previous NL iteration variable values
  bool _previous_nl_solution_required;

  /// Indicates if nonlocal coupling is required/exists
  bool _has_nonlocal_coupling;
  bool _calculate_jacobian_in_uo;

  std::vector<std::vector<const MooseVariableFEBase *>> _uo_jacobian_moose_vars;

  /// Whether there are active material properties on each thread
  std::vector<unsigned char> _has_active_material_properties;

  SolverParams _solver_params;

  /// Determines whether and which subdomains are to be checked to ensure that they have an active kernel
  CoverageCheckMode _kernel_coverage_check;
  std::vector<SubdomainName> _kernel_coverage_blocks;

  /// whether to perform checking of boundary restricted nodal object variable dependencies,
  /// e.g. whether the variable dependencies are defined on the selected boundaries
  const bool _boundary_restricted_node_integrity_check;

  /// whether to perform checking of boundary restricted elemental object variable dependencies,
  /// e.g. whether the variable dependencies are defined on the selected boundaries
  const bool _boundary_restricted_elem_integrity_check;

  /// Determines whether and which subdomains are to be checked to ensure that they have an active material
  CoverageCheckMode _material_coverage_check;
  std::vector<SubdomainName> _material_coverage_blocks;

  /// Whether to check overlapping Dirichlet and Flux BCs and/or multiple DirichletBCs per sideset
  bool _fv_bcs_integrity_check;

  /// Determines whether a check to verify material dependencies on every subdomain
  const bool _material_dependency_check;

  /// Whether or not checking the state of uo/aux evaluation
  const bool _uo_aux_state_check;

  /// Maximum number of quadrature points used in the problem
  unsigned int _max_qps;

  /// Maximum scalar variable order
  libMesh::Order _max_scalar_order;

  /// Indicates whether or not this executioner has a time integrator (during setup)
  bool _has_time_integrator;

  /// Whether or not an exception has occurred
  bool _has_exception;

  /// Whether or not information about how many transfers have completed is printed
  bool _parallel_barrier_messaging;

  /// Whether or not to be verbose during setup
  MooseEnum _verbose_setup;

  /// Whether or not to be verbose with multiapps
  bool _verbose_multiapps;

  /// The error message to go with an exception
  std::string _exception_message;

  /// Current execute_on flag
  ExecFlagType _current_execute_on_flag;

  /// The control logic warehouse
  ExecuteMooseObjectWarehouse<Control> _control_warehouse;

  /// PETSc option storage
  Moose::PetscSupport::PetscOptions _petsc_options;
#if !PETSC_RELEASE_LESS_THAN(3, 12, 0)
  PetscOptions _petsc_option_data_base;
#endif

  /// If or not PETSc options have been added to database
  bool _is_petsc_options_inserted;

  std::shared_ptr<LineSearch> _line_search;

  std::unique_ptr<libMesh::ConstElemRange> _evaluable_local_elem_range;
  std::unique_ptr<libMesh::ConstElemRange> _nl_evaluable_local_elem_range;
  std::unique_ptr<libMesh::ConstElemRange> _aux_evaluable_local_elem_range;

  std::unique_ptr<libMesh::ConstElemRange> _current_algebraic_elem_range;
  std::unique_ptr<libMesh::ConstNodeRange> _current_algebraic_node_range;
  std::unique_ptr<ConstBndNodeRange> _current_algebraic_bnd_node_range;

  /// Automatic differentiaion (AD) flag which indicates whether any consumer has
  /// requested an AD material property or whether any suppier has declared an AD material property
  bool _using_ad_mat_props;

  // loop state during projection of initial conditions
  unsigned short _current_ic_state;

private:
  /**
   * Handle exceptions. Note that the result of this call will be a thrown MooseException. The
   * caller of this method must determine how to handle the thrown exception
   */
  void handleException(const std::string & calling_method);

  /**
   * Helper for getting mortar objects corresponding to primary boundary ID, secondary boundary ID,
   * and displaced parameters, given some initial set
   */
  std::vector<MortarUserObject *>
  getMortarUserObjects(BoundaryID primary_boundary_id,
                       BoundaryID secondary_boundary_id,
                       bool displaced,
                       const std::vector<MortarUserObject *> & mortar_uo_superset);

  /**
   * Helper for getting mortar objects corresponding to primary boundary ID, secondary boundary ID,
   * and displaced parameters from the entire active mortar user object set
   */
  std::vector<MortarUserObject *> getMortarUserObjects(BoundaryID primary_boundary_id,
                                                       BoundaryID secondary_boundary_id,
                                                       bool displaced);

  /**
   * Determine what solver system the provided variable name lies in
   * @param var_name The name of the variable we are doing solver system lookups for
   * @param error_if_not_found Whether to error if the variable name isn't found in any of the
   * solver systems
   * @return A pair in which the first member indicates whether the variable was found in the
   * solver systems and the second member indicates the solver system number in which the
   * variable was found (or an invalid unsigned integer if not found)
   */
  virtual std::pair<bool, unsigned int>
  determineSolverSystem(const std::string & var_name,
                        bool error_if_not_found = false) const override;

  /**
   * Checks if the variable of the initial condition is getting restarted and errors for specific
   * cases
   * @param ic_name The name of the initial condition
   * @param var_name The name of the variable
   */
  void checkICRestartError(const std::string & ic_name,
                           const std::string & name,
                           const VariableName & var_name);

  /*
   * Test if stateful property redistribution is expected to be
   * necessary, and set it up if so.
   */
  void addAnyRedistributers();

  void updateMaxQps();

  void joinAndFinalize(TheWarehouse::Query query, bool isgen = false);

  /**
   * Reset state of this object in preparation for the next evaluation.
   */
  virtual void resetState();

  // Parameters handling Jacobian sparsity pattern behavior
  /// Whether to error when the Jacobian is re-allocated, usually because the sparsity pattern changed
  bool _error_on_jacobian_nonzero_reallocation;
  /// Whether to ignore zeros in the Jacobian, thereby leading to a reduced sparsity pattern
  bool _ignore_zeros_in_jacobian;
  /// Whether to preserve the system matrix / Jacobian sparsity pattern, using 0-valued entries usually
  bool _preserve_matrix_sparsity_pattern;

  const bool _force_restart;
  const bool _allow_ics_during_restart;
  const bool _skip_nl_system_check;
  bool _fail_next_nonlinear_convergence_check;
  const bool _allow_invalid_solution;
  const bool _show_invalid_solution_console;
  const bool & _immediately_print_invalid_solution;

  /// At or beyond initialSteup stage
  bool _started_initial_setup;

  /// Whether the problem has dgkernels or interface kernels
  bool _has_internal_edge_residual_objects;

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
  friend class Restartable;
  friend class DisplacedProblem;

  /// Whether the simulation requires mortar coupling
  bool _has_mortar;

  /// Number of steps in a grid sequence
  unsigned int _num_grid_steps;

  /// Whether to trust the user coupling matrix no matter what. See
  /// https://github.com/idaholab/moose/issues/16395 for detailed background
  bool _trust_user_coupling_matrix = false;

  /// Flag used to indicate whether we are computing the scaling Jacobian
  bool _computing_scaling_jacobian = false;

  /// Flag used to indicate whether we are computing the scaling Residual
  bool _computing_scaling_residual = false;

  /// Flag used to indicate whether we are doing the uo/aux state check in execute
  bool _checking_uo_aux_state = false;

  /// When to print the execution of loops
  ExecFlagEnum _print_execution_on;

  /// Whether to identify variable groups in nonlinear systems. This affects dof ordering
  const bool _identify_variable_groups_in_nl;

  /// A data member to store the residual vector tag(s) passed into \p computeResidualTag(s). This
  /// data member will be used when APIs like \p cacheResidual, \p addCachedResiduals, etc. are
  /// called
  std::vector<VectorTag> _current_residual_vector_tags;

  /// Whether we are performing some calculations with finite volume discretizations
  bool _have_fv = false;

  /// If we catch an exception during residual/Jacobian evaluaton for which we don't have specific
  /// handling, immediately error instead of allowing the time step to be cut
  const bool _regard_general_exceptions_as_errors;

  friend void Moose::PetscSupport::setSinglePetscOption(const std::string & name,
                                                        const std::string & value,
                                                        FEProblemBase * const problem);
};

using FVProblemBase = FEProblemBase;

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

template <typename T>
std::vector<std::shared_ptr<T>>
FEProblemBase::addObject(const std::string & type,
                         const std::string & name,
                         InputParameters & parameters,
                         const bool threaded,
                         const std::string & var_param_name)
{
  parallel_object_only();

  logAdd(MooseUtils::prettyCppType<T>(), name, type, parameters);
  // Add the _subproblem and _sys parameters depending on use_displaced_mesh
  addObjectParamsHelper(parameters, name, var_param_name);

  const auto n_threads = threaded ? libMesh::n_threads() : 1;
  std::vector<std::shared_ptr<T>> objects(n_threads);
  for (THREAD_ID tid = 0; tid < n_threads; ++tid)
  {
    std::shared_ptr<T> obj = _factory.create<T>(type, name, parameters, tid);
    theWarehouse().add(obj);
    objects[tid] = std::move(obj);
  }

  return objects;
}

inline NonlinearSystemBase &
FEProblemBase::getNonlinearSystemBase(const unsigned int sys_num)
{
  mooseAssert(sys_num < _nl.size(), "System number greater than the number of nonlinear systems");
  return *_nl[sys_num];
}

inline const NonlinearSystemBase &
FEProblemBase::getNonlinearSystemBase(const unsigned int sys_num) const
{
  mooseAssert(sys_num < _nl.size(), "System number greater than the number of nonlinear systems");
  return *_nl[sys_num];
}

inline SolverSystem &
FEProblemBase::getSolverSystem(const unsigned int sys_num)
{
  mooseAssert(sys_num < _solver_systems.size(),
              "System number greater than the number of solver systems");
  return *_solver_systems[sys_num];
}

inline const SolverSystem &
FEProblemBase::getSolverSystem(const unsigned int sys_num) const
{
  mooseAssert(sys_num < _solver_systems.size(),
              "System number greater than the number of solver systems");
  return *_solver_systems[sys_num];
}

inline NonlinearSystemBase &
FEProblemBase::currentNonlinearSystem()
{
  mooseAssert(_current_nl_sys, "The nonlinear system is not currently set");
  return *_current_nl_sys;
}

inline const NonlinearSystemBase &
FEProblemBase::currentNonlinearSystem() const
{
  mooseAssert(_current_nl_sys, "The nonlinear system is not currently set");
  return *_current_nl_sys;
}

inline LinearSystem &
FEProblemBase::getLinearSystem(const unsigned int sys_num)
{
  mooseAssert(sys_num < _linear_systems.size(),
              "System number greater than the number of linear systems");
  return *_linear_systems[sys_num];
}

inline const LinearSystem &
FEProblemBase::getLinearSystem(const unsigned int sys_num) const
{
  mooseAssert(sys_num < _linear_systems.size(),
              "System number greater than the number of linear systems");
  return *_linear_systems[sys_num];
}

inline LinearSystem &
FEProblemBase::currentLinearSystem()
{
  mooseAssert(_current_linear_sys, "The linear system is not currently set");
  return *_current_linear_sys;
}

inline const LinearSystem &
FEProblemBase::currentLinearSystem() const
{
  mooseAssert(_current_linear_sys, "The linear system is not currently set");
  return *_current_linear_sys;
}

inline Assembly &
FEProblemBase::assembly(const THREAD_ID tid, const unsigned int sys_num)
{
  mooseAssert(tid < _assembly.size(), "Assembly objects not initialized");
  mooseAssert(sys_num < _assembly[tid].size(),
              "System number larger than the assembly container size");
  return *_assembly[tid][sys_num];
}

inline const Assembly &
FEProblemBase::assembly(const THREAD_ID tid, const unsigned int sys_num) const
{
  mooseAssert(tid < _assembly.size(), "Assembly objects not initialized");
  mooseAssert(sys_num < _assembly[tid].size(),
              "System number larger than the assembly container size");
  return *_assembly[tid][sys_num];
}

inline const libMesh::CouplingMatrix *
FEProblemBase::couplingMatrix(const unsigned int i) const
{
  return _cm[i].get();
}

inline void
FEProblemBase::fvBCsIntegrityCheck(const bool fv_bcs_integrity_check)
{
  if (!_fv_bcs_integrity_check)
    // the user has requested that we don't check integrity so we will honor that
    return;

  _fv_bcs_integrity_check = fv_bcs_integrity_check;
}

inline const std::vector<VectorTag> &
FEProblemBase::currentResidualVectorTags() const
{
  return _current_residual_vector_tags;
}

inline void
FEProblemBase::setCurrentResidualVectorTags(const std::set<TagID> & vector_tags)
{
  _current_residual_vector_tags = getVectorTags(vector_tags);
}

inline void
FEProblemBase::clearCurrentResidualVectorTags()
{
  _current_residual_vector_tags.clear();
}
