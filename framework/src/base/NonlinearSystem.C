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

#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "PetscSupport.h"
#include "Factory.h"
#include "ParallelUniqueId.h"
#include "ThreadedElementLoop.h"
#include "MaterialData.h"
#include "ComputeResidualThread.h"
#include "ComputeJacobianThread.h"
#include "ComputeFullJacobianThread.h"
#include "ComputeJacobianBlocksThread.h"
#include "ComputeDiracThread.h"
#include "ComputeElemDampingThread.h"
#include "ComputeNodalKernelsThread.h"
#include "ComputeNodalKernelBcsThread.h"
#include "ComputeNodalKernelJacobiansThread.h"
#include "ComputeNodalKernelBCJacobiansThread.h"
#include "TimeKernel.h"
#include "BoundaryCondition.h"
#include "PresetNodalBC.h"
#include "NodalBC.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
#include "InterfaceKernel.h"
#include "ElementDamper.h"
#include "GeneralDamper.h"
#include "DisplacedProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "FaceFaceConstraint.h"
#include "ElemElemConstraint.h"
#include "ScalarKernel.h"
#include "Parser.h"
#include "Split.h"
#include "SplitBasedPreconditioner.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "NodalKernel.h"
#include "DiracKernel.h"
#include "NodalKernel.h"
#include "TimeIntegrator.h"
#include "Predictor.h"
#include "Assembly.h"
#include "ElementPairLocator.h"

// libMesh
#include "libmesh/nonlinear_solver.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dense_vector.h"
#include "libmesh/boundary_info.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/dense_submatrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"

// PETSc
#ifdef LIBMESH_HAVE_PETSC
#include "petscsnes.h"
#if !PETSC_VERSION_LESS_THAN(3,3,0)
#include <PetscDMMoose.h>
EXTERN_C_BEGIN
extern PetscErrorCode DMCreate_Moose(DM);
EXTERN_C_END
#endif
#endif


namespace Moose {
  void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, NonlinearImplicitSystem& sys)
  {
    FEProblem * p = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
    p->computeJacobian(sys, soln, jacobian);
  }

  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual, NonlinearImplicitSystem& sys)
  {
    FEProblem * p = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
    p->computeResidual(sys, soln, residual);
  }

  void compute_bounds (NumericVector<Number>& lower, NumericVector<Number>& upper, NonlinearImplicitSystem& sys)
  {
    FEProblem * p = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
    p->computeBounds(sys, lower, upper);
  }

  void compute_nullspace (std::vector<NumericVector<Number>*>& sp, NonlinearImplicitSystem& sys)
  {
    FEProblem * p = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
    p->computeNullSpace(sys, sp);
  }

  void compute_nearnullspace (std::vector<NumericVector<Number>*>& sp, NonlinearImplicitSystem& sys)
  {
    FEProblem * p = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
    p->computeNearNullSpace(sys, sp);
  }

  void compute_postcheck (const NumericVector<Number> & old_soln,
                          NumericVector<Number> & search_direction,
                          NumericVector<Number> & new_soln,
                          bool & changed_search_direction,
                          bool & changed_new_soln,
                          NonlinearImplicitSystem & sys)
  {
    FEProblem * p = sys.get_equation_systems().parameters.get<FEProblem *>("_fe_problem");
    p->computePostCheck(sys,
                        old_soln,
                        search_direction,
                        new_soln,
                        changed_search_direction,
                        changed_new_soln);
  }
} // namespace Moose


NonlinearSystem::NonlinearSystem(FEProblem & fe_problem, const std::string & name) :
    SystemTempl<TransientNonlinearImplicitSystem>(fe_problem, name, Moose::VAR_NONLINEAR),
    ConsoleStreamInterface(fe_problem.getMooseApp()),
    _fe_problem(fe_problem),
    _last_rnorm(0.),
    _last_nl_rnorm(0.),
    _l_abs_step_tol(1e-10),
    _initial_residual_before_preset_bcs(0.),
    _initial_residual_after_preset_bcs(0.),
    _current_nl_its(0),
    _compute_initial_residual_before_preset_bcs(true),
    _current_solution(NULL),
    _residual_ghosted(addVector("residual_ghosted", false, GHOSTED)),
    _serialized_solution(*NumericVector<Number>::build(_communicator).release()),
    _residual_copy(*NumericVector<Number>::build(_communicator).release()),
    _u_dot(addVector("u_dot", true, GHOSTED)),
    _Re_time(addVector("Re_time", false, GHOSTED)),
    _Re_non_time(addVector("Re_non_time", false, GHOSTED)),
    _scalar_kernels(/*threaded=*/false),
    _nodal_bcs(/*threaded=*/false),
    _preset_nodal_bcs(/*threaded=*/false),
    _splits(/*threaded=*/false),
    _increment_vec(NULL),
    _pc_side(Moose::PCS_RIGHT),
    _use_finite_differenced_preconditioner(false),
    _have_decomposition(false),
    _use_split_based_preconditioner(false),
    _add_implicit_geometric_coupling_entries_to_jacobian(false),
    _assemble_constraints_separately(false),
    _need_serialized_solution(false),
    _need_residual_copy(false),
    _need_residual_ghosted(false),
    _debugging_residuals(false),
    _doing_dg(false),
    _n_iters(0),
    _n_linear_iters(0),
    _n_residual_evaluations(0),
    _final_residual(0.),
    _computing_initial_residual(false),
    _print_all_var_norms(false),
    _has_save_in(false),
    _has_diag_save_in(false),
    _has_nodalbc_save_in(false),
    _has_nodalbc_diag_save_in(false)
{
  _sys.nonlinear_solver->residual      = Moose::compute_residual;
  _sys.nonlinear_solver->jacobian      = Moose::compute_jacobian;
  _sys.nonlinear_solver->bounds        = Moose::compute_bounds;
  _sys.nonlinear_solver->nullspace     = Moose::compute_nullspace;
  _sys.nonlinear_solver->nearnullspace = Moose::compute_nearnullspace;

#ifdef LIBMESH_HAVE_PETSC
  PetscNonlinearSolver<Real> * petsc_solver = static_cast<PetscNonlinearSolver<Real> *>(_sys.nonlinear_solver.get());
  if (petsc_solver)
  {
    petsc_solver->set_residual_zero_out(false);
    petsc_solver->set_jacobian_zero_out(false);
    petsc_solver->use_default_monitor(false);
  }
#endif
}

NonlinearSystem::~NonlinearSystem()
{
  delete &_serialized_solution;
  delete &_residual_copy;
}

void
NonlinearSystem::init()
{
  Moose::setup_perf_log.push("NonlinerSystem::init()", "Setup");

  setupDampers();

  _current_solution = _sys.current_local_solution.get();

  if (_need_serialized_solution)
    _serialized_solution.init(_sys.n_dofs(), false, SERIAL);

  if (_need_residual_copy)
    _residual_copy.init(_sys.n_dofs(), false, SERIAL);

  Moose::setup_perf_log.pop("NonlinerSystem::init()", "Setup");
}

void
NonlinearSystem::solve()
{
  // Only attach the postcheck function to the solver if we actually
  // have dampers or if the FEProblem needs to update the solution,
  // which is also done during the linesearch postcheck.  It doesn't
  // hurt to do this multiple times, it is just setting a pointer.
  if (_fe_problem.hasDampers() || _fe_problem.shouldUpdateSolution())
    _sys.nonlinear_solver->postcheck = Moose::compute_postcheck;

  if (_fe_problem.solverParams()._type != Moose::ST_LINEAR)
  {
    // Calculate the initial residual for use in the convergence criterion.
    _computing_initial_residual = true;
    _fe_problem.computeResidual(_sys, *_current_solution, *_sys.rhs);
    _computing_initial_residual = false;
    _sys.rhs->close();
    _initial_residual_before_preset_bcs = _sys.rhs->l2_norm();
    if (_compute_initial_residual_before_preset_bcs)
      _console << "Initial residual before setting preset BCs: "
               << _initial_residual_before_preset_bcs << '\n';
  }

  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;

  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  if (_use_finite_differenced_preconditioner)
    setupFiniteDifferencedPreconditioner();

  if (_use_split_based_preconditioner)
    setupSplitBasedPreconditioner();

  _time_integrator->solve();
  _time_integrator->postSolve();

  // store info about the solve
  _n_iters = _sys.n_nonlinear_iterations();
  _final_residual = _sys.final_nonlinear_residual();

#ifdef LIBMESH_HAVE_PETSC
  _n_linear_iters = static_cast<PetscNonlinearSolver<Real> &>(*_sys.nonlinear_solver).get_total_linear_iterations();
#endif

#ifdef LIBMESH_HAVE_PETSC
  if (_use_finite_differenced_preconditioner)
#if PETSC_VERSION_LESS_THAN(3,2,0)
    MatFDColoringDestroy(_fdcoloring);
#else
    MatFDColoringDestroy(&_fdcoloring);
#endif
#endif
}

void
NonlinearSystem::restoreSolutions()
{
  // call parent
  SystemTempl<TransientNonlinearImplicitSystem>::restoreSolutions();
  // and update _current_solution
  _current_solution = _sys.current_local_solution.get();
}


void
NonlinearSystem::stopSolve()
{
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,0,0)
#else
  PetscNonlinearSolver<Real> & solver =
    static_cast<PetscNonlinearSolver<Real> &>(*sys().nonlinear_solver);
  SNESSetFunctionDomainError(solver.snes());
#endif
#endif

  // Insert a NaN into the residual vector.  As of PETSc-3.6, this
  // should make PETSc return DIVERGED_NANORINF the next time it does
  // a reduction.  We'll write to the first local dof on every
  // processor I guess?
  _sys.rhs->set(_sys.rhs->first_local_index(), std::numeric_limits<Real>::quiet_NaN());
  _sys.rhs->close();

  // Clean up by getting other vectors into a valid state for a
  // (possible) subsequent solve.  There may be more than just
  // these...
  residualVector(Moose::KT_TIME).close();
  residualVector(Moose::KT_NONTIME).close();
}


void
NonlinearSystem::initialSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.initialSetup(tid);
    _nodal_kernels.initialSetup(tid);
    _dirac_kernels.initialSetup(tid);
    if (_doing_dg)
      _dg_kernels.initialSetup(tid);
    _interface_kernels.initialSetup(tid);
    _element_dampers.initialSetup(tid);
    _integrated_bcs.initialSetup(tid);
  }
  _scalar_kernels.initialSetup();
  _constraints.initialSetup();
  _general_dampers.initialSetup();
  _nodal_bcs.initialSetup();
}

void
NonlinearSystem::timestepSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.timestepSetup(tid);
    _nodal_kernels.timestepSetup(tid);
    _dirac_kernels.timestepSetup(tid);
    if (_doing_dg)
      _dg_kernels.timestepSetup(tid);
    _interface_kernels.timestepSetup(tid);
    _element_dampers.timestepSetup(tid);
    _integrated_bcs.timestepSetup(tid);
  }
  _scalar_kernels.initialSetup();
  _constraints.timestepSetup();
  _general_dampers.timestepSetup();
  _nodal_bcs.timestepSetup();
}


void
NonlinearSystem::setupFiniteDifferencedPreconditioner()
{
#ifdef LIBMESH_HAVE_PETSC
  // Make sure that libMesh isn't going to override our preconditioner
  _sys.nonlinear_solver->jacobian = NULL;

  PetscNonlinearSolver<Number> & petsc_nonlinear_solver =
    dynamic_cast<PetscNonlinearSolver<Number>&>(*_sys.nonlinear_solver);

  // Pointer to underlying PetscMatrix type
  PetscMatrix<Number>* petsc_mat =
    dynamic_cast<PetscMatrix<Number>*>(_sys.matrix);

#if PETSC_VERSION_LESS_THAN(3,2,0)
  // This variable is only needed for PETSC < 3.2.0
  PetscVector<Number>* petsc_vec =
    dynamic_cast<PetscVector<Number>*>(_sys.solution.get());
#endif

  Moose::compute_jacobian(*_sys.current_local_solution,
                          *petsc_mat,
                          _sys);

  if (!petsc_mat)
    mooseError("Could not convert to Petsc matrix.");

  petsc_mat->close();

  PetscErrorCode ierr=0;
  ISColoring iscoloring;

#if PETSC_VERSION_LESS_THAN(3,2,0)
  // PETSc 3.2.x
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORING_LF, &iscoloring);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
#elif PETSC_VERSION_LESS_THAN(3,5,0)
  // PETSc 3.3.x, 3.4.x
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORINGLF, &iscoloring);
  CHKERRABORT(_communicator.get(),ierr);
#else
  // PETSc 3.5.x
  MatColoring matcoloring;
  ierr = MatColoringCreate(petsc_mat->mat(),&matcoloring);
  CHKERRABORT(_communicator.get(),ierr);
  ierr = MatColoringSetType(matcoloring,MATCOLORINGLF);
  CHKERRABORT(_communicator.get(),ierr);
  ierr = MatColoringSetFromOptions(matcoloring);
  CHKERRABORT(_communicator.get(),ierr);
  ierr = MatColoringApply(matcoloring,&iscoloring);
  CHKERRABORT(_communicator.get(),ierr);
  ierr = MatColoringDestroy(&matcoloring);
  CHKERRABORT(_communicator.get(),ierr);
#endif


  MatFDColoringCreate(petsc_mat->mat(),iscoloring, &_fdcoloring);
  MatFDColoringSetFromOptions(_fdcoloring);
  MatFDColoringSetFunction(_fdcoloring,
                           (PetscErrorCode (*)(void))&libMesh::__libmesh_petsc_snes_residual,
                           &petsc_nonlinear_solver);
#if !PETSC_RELEASE_LESS_THAN(3,5,0)
  MatFDColoringSetUp(petsc_mat->mat(),iscoloring,_fdcoloring);
#endif
#if PETSC_VERSION_LESS_THAN(3,4,0)
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESDefaultComputeJacobianColor,
                  _fdcoloring);
#else
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESComputeJacobianDefaultColor,
                  _fdcoloring);
#endif
#if PETSC_VERSION_LESS_THAN(3,2,0)
  Mat my_mat = petsc_mat->mat();
  MatStructure my_struct;

  SNESComputeJacobian(petsc_nonlinear_solver.snes(),
                      petsc_vec->vec(),
                      &my_mat,
                      &my_mat,
                      &my_struct);
#endif

#if PETSC_VERSION_LESS_THAN(3,2,0)
  ISColoringDestroy(iscoloring);
#else
  // PETSc 3.3.0
  ISColoringDestroy(&iscoloring);
#endif

#endif
}

void
NonlinearSystem::setDecomposition(const std::vector<std::string>& splits)
{
  /// Although a single top-level split is allowed in Problem, treat it as a list of splits for conformity with the Split input syntax.
 if (splits.size() && splits.size() != 1)
  {
    std::ostringstream err;
    err << "Only a single top-level split is allowed in a Problem's decomposition.";
    mooseError(err.str());
  }
  if (splits.size())
  {
    _decomposition_split = splits[0];
    _have_decomposition = true;
  }  else {
    _have_decomposition = false;
  }
}


void
NonlinearSystem::setupDecomposition()
{
  if (!_have_decomposition) return;
  MooseSharedPointer<Split> top_split = getSplit(_decomposition_split);
  top_split->setup();
}

void
NonlinearSystem::setupSplitBasedPreconditioner()
{
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
  SplitBasedPreconditioner* sbp = dynamic_cast<SplitBasedPreconditioner*>(_preconditioner.get());
  sbp->setup();

#if defined(LIBMESH_ENABLE_BLOCKED_STORAGE)
  // HACK: Use DMCreateMatrix, which we control through DMMoose
  // It would be A LOT better if libMesh would use some kind of 'CreateMatrix' callback that we could hook up to DMCreateMatrix.
  // TODO: override the Jacobian computation routine to enable DM-based submatrix computation
  NonlinearSolver<Number> *nonlinear_solver = sys().nonlinear_solver.get();
  PetscNonlinearSolver<Number> *petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nonlinear_solver);
  SNES snes = petsc_solver->snes();
  PetscErrorCode ierr;
  DM dm;
  ierr = SNESGetDM(snes,&dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  Mat J;
  ierr = DMCreateMatrix(dm,MATAIJ,&J);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  PetscMatrix<Number> petscJ(J);
  PetscMatrix<Number>* petsc_matrix = dynamic_cast<PetscMatrix<Number>*>(sys().matrix);
  petsc_matrix->swap(petscJ);
#endif
#endif
}

bool
NonlinearSystem::converged()
{
  if (_fe_problem.hasException())
    return false;

  return _sys.nonlinear_solver->converged;
}

void
NonlinearSystem::addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<SystemBase *>("_sys") = this;

  MooseSharedPointer<TimeIntegrator> ti = _factory.create<TimeIntegrator>(type, name, parameters);
  _time_integrator = ti;
}

void
NonlinearSystem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to warehouse
    MooseSharedPointer<KernelBase> kernel = _factory.create<KernelBase>(kernel_name, name, parameters, tid);
    _kernels.addObject(kernel, tid);

    // Store time/non-time kernels separately
    MooseSharedPointer<TimeKernel> t_kernel = MooseSharedNamespace::dynamic_pointer_cast<TimeKernel>(kernel);
    if (t_kernel)
      _time_kernels.addObject(kernel, tid);
    else
      _non_time_kernels.addObject(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystem::addNodalKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to the warehouse
    MooseSharedPointer<NodalKernel> kernel = _factory.create<NodalKernel>(kernel_name, name, parameters, tid);
    _nodal_kernels.addObject(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystem::addScalarKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  MooseSharedPointer<ScalarKernel> kernel = _factory.create<ScalarKernel>(kernel_name, name, parameters);
  _scalar_kernels.addObject(kernel);
}

void
NonlinearSystem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  // ThreadID
  THREAD_ID tid = 0;

  // Create the object
  MooseSharedPointer<BoundaryCondition> bc = _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  _vars[tid].addBoundaryVar(boundary_ids, &bc->variable());

  // Cast to the various types of BCs
  MooseSharedPointer<NodalBC> nbc = MooseSharedNamespace::dynamic_pointer_cast<NodalBC>(bc);
  MooseSharedPointer<IntegratedBC> ibc = MooseSharedNamespace::dynamic_pointer_cast<IntegratedBC>(bc);

  // NodalBC
  if (nbc)
  {
    _nodal_bcs.addObject(nbc);
    _vars[tid].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
      _has_nodalbc_save_in = true;
    if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
      _has_nodalbc_diag_save_in = true;

    // PresetNodalBC
    MooseSharedPointer<PresetNodalBC> pnbc = MooseSharedNamespace::dynamic_pointer_cast<PresetNodalBC>(bc);
    if (pnbc)
      _preset_nodal_bcs.addObject(pnbc);
  }

  // IntegratedBC
  else if (ibc)
  {
    _integrated_bcs.addObject(ibc, tid);
    _vars[tid].addBoundaryVars(boundary_ids, ibc->getCoupledVars());

    if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
      _has_save_in = true;
    if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
      _has_diag_save_in = true;

    for (tid = 1; tid < libMesh::n_threads(); tid++)
    {
      // Create the object
      bc = _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);

      // Active BoundaryIDs for the object
      const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
      _vars[tid].addBoundaryVar(boundary_ids, &bc->variable());

      ibc = MooseSharedNamespace::static_pointer_cast<IntegratedBC>(bc);

      _integrated_bcs.addObject(ibc, tid);
      _vars[tid].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
    }
  }

  else
    mooseError("Unknown BoundaryCondition type for object named " << bc->name());
}

void
NonlinearSystem::addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters)
{
  MooseSharedPointer<Constraint> constraint = _factory.create<Constraint>(c_name, name, parameters);
  _constraints.addObject(constraint);

  if (constraint && constraint->addCouplingEntriesToJacobian())
    addImplicitGeometricCouplingEntriesToJacobian(true);
}

void
NonlinearSystem::addDiracKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    MooseSharedPointer<DiracKernel> kernel = _factory.create<DiracKernel>(kernel_name, name, parameters, tid);
    _dirac_kernels.addObject(kernel, tid);
  }
}

void
NonlinearSystem::addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    MooseSharedPointer<DGKernel> dg_kernel = _factory.create<DGKernel>(dg_kernel_name, name, parameters, tid);
    _dg_kernels.addObject(dg_kernel, tid);
  }

  _doing_dg = true;
}

void
NonlinearSystem::addInterfaceKernel(std::string interface_kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    MooseSharedPointer<InterfaceKernel> interface_kernel = _factory.create<InterfaceKernel>(interface_kernel_name, name, parameters, tid);

    const std::set<BoundaryID> & boundary_ids = interface_kernel->boundaryIDs();
    _vars[tid].addBoundaryVar(boundary_ids, &interface_kernel->variable());

    _interface_kernels.addObject(interface_kernel, tid);
    _vars[tid].addBoundaryVars(boundary_ids, interface_kernel->getCoupledVars());
  }

  _doing_dg = true;
}

void
NonlinearSystem::addDamper(const std::string & damper_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    MooseSharedPointer<Damper> damper = _factory.create<Damper>(damper_name, name, parameters, tid);

    // Attempt to cast to the damper types
    MooseSharedPointer<ElementDamper> ed = MooseSharedNamespace::dynamic_pointer_cast<ElementDamper>(damper);
    MooseSharedPointer<GeneralDamper> gd = MooseSharedNamespace::dynamic_pointer_cast<GeneralDamper>(damper);

    if (gd)
    {
      _general_dampers.addObject(gd);
      break; // not threaded
    }
    else if (ed)
      _element_dampers.addObject(ed,tid);
    else
      mooseError("Invalid damper type");
  }
}

void
NonlinearSystem::addSplit(const  std::string & split_name, const std::string & name, InputParameters parameters)
{
  MooseSharedPointer<Split> split = _factory.create<Split>(split_name, name, parameters);
  _splits.addObject(split);
}

MooseSharedPointer<Split>
NonlinearSystem::getSplit(const std::string & name)
{
  return _splits.getActiveObject(name);
}

NumericVector<Number> &
NonlinearSystem::addVector(const std::string & vector_name, const bool project, const ParallelType type, bool zero_for_residual)
{
  if (hasVector(vector_name))
    return getVector(vector_name);

  NumericVector<Number> * vec = &_sys.add_vector(vector_name, project, type);

  if (zero_for_residual)
    _vecs_to_zero_for_residual.push_back(vec);
  return *vec;
}

void
NonlinearSystem::computeResidual(NumericVector<Number> & residual, Moose::KernelType type)
{
  Moose::perf_log.push("compute_residual()", "Execution");

  _n_residual_evaluations++;

  Moose::enableFPE();

  for (std::vector<NumericVector<Number> *>::iterator it = _vecs_to_zero_for_residual.begin();
      it != _vecs_to_zero_for_residual.end();
      ++it)
  {
    (*it)->close();
    (*it)->zero();
  }

  try
  {
    residual.zero();
    residualVector(Moose::KT_TIME).zero();
    residualVector(Moose::KT_NONTIME).zero();
    computeResidualInternal(type);
    residualVector(Moose::KT_TIME).close();
    residualVector(Moose::KT_NONTIME).close();
    _time_integrator->postStep(residual);
    residual.close();

    computeNodalBCs(residual);

    // If we are debugging residuals we need one more assignment to have the ghosted copy up to date
    if (_need_residual_ghosted && _debugging_residuals)
    {
      _residual_ghosted = residual;
      _residual_ghosted.close();
    }

    // Need to close and update the aux system in case residuals were saved to it.
    if (_has_nodalbc_save_in)
      _fe_problem.getAuxiliarySystem().solution().close();
    if (hasSaveIn())
      _fe_problem.getAuxiliarySystem().update();
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_residual()", "Execution");
}


void
NonlinearSystem::onTimestepBegin()
{
  _time_integrator->preSolve();
  if (_predictor.get())
    _predictor->timestepSetup();
}

void
NonlinearSystem::setInitialSolution()
{
  NumericVector<Number> & initial_solution(solution());
  if (_predictor.get() && _predictor->shouldApply())
  {
    _predictor->apply(initial_solution);
    _fe_problem.predictorCleanup(initial_solution);
  }

  // do nodal BC
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    BoundaryID boundary_id = bnode->_bnd_id;
    Node * node = bnode->_node;

    if (node->processor_id() == processor_id())
    {
      // reinit variables in nodes
      _fe_problem.reinitNodeFace(node, boundary_id, 0);

      if (_preset_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
      {
        const std::vector<MooseSharedPointer<PresetNodalBC> > & preset = _preset_nodal_bcs.getActiveBoundaryObjects(boundary_id);
        for (std::vector<MooseSharedPointer<PresetNodalBC> >::const_iterator it = preset.begin(); it != preset.end(); ++it)
          (*it)->computeValue(initial_solution);
      }
    }
  }

  _sys.solution->close();
  update();

  // Set constraint slave values
  setConstraintSlaveValues(initial_solution, false);

  if (_fe_problem.getDisplacedProblem())
    setConstraintSlaveValues(initial_solution, true);
}

void NonlinearSystem::setPredictor(MooseSharedPointer<Predictor> predictor)
{
  _predictor = predictor;
}

void
NonlinearSystem::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  _kernels.subdomainSetup(subdomain, tid);
  _nodal_kernels.subdomainSetup(subdomain, tid);
  _element_dampers.subdomainSetup(subdomain, tid);
}

NumericVector<Number> &
NonlinearSystem::solutionUDot()
{
  return _u_dot;
}

NumericVector<Number> &
NonlinearSystem::residualVector(Moose::KernelType type)
{
  switch (type)
  {
  case Moose::KT_TIME: return _Re_time;
  case Moose::KT_NONTIME: return _Re_non_time;
  case Moose::KT_ALL: return _Re_non_time;

  default: mooseError("Trying to get residual vector that is not available");
  }
}

void
NonlinearSystem::computeTimeDerivatives()
{
  _time_integrator->preStep();
  _time_integrator->computeTimeDerivatives();
}

void
NonlinearSystem::enforceNodalConstraintsResidual(NumericVector<Number> & residual)
{
  THREAD_ID tid = 0; // constraints are going to be done single-threaded
  residual.close();
  if (_constraints.hasActiveNodalConstraints())
  {
    const std::vector<MooseSharedPointer<NodalConstraint> > & ncs = _constraints.getActiveNodalConstraints();
    for (std::vector<MooseSharedPointer<NodalConstraint> >::const_iterator it = ncs.begin(); it != ncs.end(); ++it)
    {
      MooseSharedPointer<NodalConstraint> nc = (*it);
      std::vector<dof_id_type> & slave_node_ids = nc->getSlaveNodeId();
      std::vector<dof_id_type> & master_node_ids = nc->getMasterNodeId();

      if ((slave_node_ids.size() > 0) && (master_node_ids.size() > 0))
      {
        _fe_problem.reinitNodes(master_node_ids, tid);
        _fe_problem.reinitNodesNeighbor(slave_node_ids, tid);
        nc->computeResidual(residual);
      }
    }
    _fe_problem.addCachedResidualDirectly(residual, tid);
    residual.close();
  }
}

void
NonlinearSystem::enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian)
{
  THREAD_ID tid = 0;    // constraints are going to be done single-threaded
  jacobian.close();
  if (_constraints.hasActiveNodalConstraints())
  {
    const std::vector<MooseSharedPointer<NodalConstraint> > & ncs = _constraints.getActiveNodalConstraints();
    for (std::vector<MooseSharedPointer<NodalConstraint> >::const_iterator it = ncs.begin(); it != ncs.end(); ++it)
    {
      MooseSharedPointer<NodalConstraint> nc = (*it);
      std::vector<dof_id_type> & slave_node_ids = nc->getSlaveNodeId();
      std::vector<dof_id_type> & master_node_ids = nc->getMasterNodeId();

      if ((slave_node_ids.size() > 0) && (master_node_ids.size() > 0))
      {
        _fe_problem.reinitNodes(master_node_ids, tid);
        _fe_problem.reinitNodesNeighbor(slave_node_ids, tid);
        nc->computeJacobian(jacobian);
      }
    }
    _fe_problem.addCachedJacobian(jacobian, tid);
    jacobian.close();
  }
}

void
NonlinearSystem::setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced)
{
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied = false;

  for (std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {
    PenetrationLocator & pen_loc = *it->second;

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const std::vector<MooseSharedPointer<NodeFaceConstraint> > & constraints = _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (unsigned int i=0; i<slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if (slave_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            const Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _fe_problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _fe_problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _fe_problem.reinitNeighborPhys(master_elem, master_side, points, 0);

            for (unsigned int c=0; c < constraints.size(); c++)
            {
              MooseSharedPointer<NodeFaceConstraint> nfc = constraints[c];

              if (nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeSlaveValue(solution);
              }
            }
          }
        }
      }
    }
  }

  // See if constraints were applied anywhere
  _communicator.max(constraints_applied);

  if (constraints_applied)
  {
    solution.close();
    update();
  }
}

void
NonlinearSystem::constraintResiduals(NumericVector<Number> & residual, bool displaced)
{
  // Make sure the residual is in a good state
  residual.close();

  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied;
  bool residual_has_inserted_values = false;
  if (!_assemble_constraints_separately) constraints_applied = false;
  for (std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {

    if (_assemble_constraints_separately)
    {
      // Reset the constraint_applied flag before each new constraint, as they need to be assembled separately
      constraints_applied = false;
    }
    PenetrationLocator & pen_loc = *it->second;

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const std::vector<MooseSharedPointer<NodeFaceConstraint> > & constraints = _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (unsigned int i=0; i<slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if (slave_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            const Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // *These next steps MUST be done in this order!*

            // This reinits the variables that exist on the slave node
            _fe_problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            // This will set aside residual and jacobian space for the variables that have dofs on the slave node
            _fe_problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _fe_problem.reinitNeighborPhys(master_elem, master_side, points, 0);

            for (unsigned int c=0; c < constraints.size(); c++)
            {
              MooseSharedPointer<NodeFaceConstraint> nfc = constraints[c];

              if (nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeResidual();

                if (nfc->overwriteSlaveResidual())
                {
                  _fe_problem.setResidual(residual, 0);
                  residual_has_inserted_values = true;
                }
                else
                  _fe_problem.cacheResidual(0);
                _fe_problem.cacheResidualNeighbor(0);
              }
            }
          }
        }
      }
    }
    if (_assemble_constraints_separately)
    {
      // Make sure that slave contribution to master are assembled, and ghosts have been exchanged,
      // as current masters might become slaves on next iteration
      // and will need to contribute their former slaves' contributions
      // to the future masters.
      // See if constraints were applied anywhere
      _communicator.max(constraints_applied);

      if (constraints_applied)
      {
        // If any of the above constraints inserted values in the residual, it needs to be assembled
        // before adding the cached residuals below.
        _communicator.max( residual_has_inserted_values );
        if ( residual_has_inserted_values )
        {
          residual.close();
          residual_has_inserted_values = false;
        }
        _fe_problem.addCachedResidualDirectly(residual, 0);
        residual.close();

        if (_need_residual_ghosted)
          _residual_ghosted = residual;
      }
    }
  }
  if (!_assemble_constraints_separately)
  {
    _communicator.max(constraints_applied);

    if (constraints_applied)
    {
      // If any of the above constraints inserted values in the residual, it needs to be assembled
      // before adding the cached residuals below.
      _communicator.max( residual_has_inserted_values );
      if ( residual_has_inserted_values )
        residual.close();

      _fe_problem.addCachedResidualDirectly(residual, 0);
      residual.close();

      if (_need_residual_ghosted)
        _residual_ghosted = residual;
    }
  }

  THREAD_ID tid = 0;
  // go over mortar interfaces
  std::vector<MooseMesh::MortarInterface *> & ifaces = _mesh.getMortarInterfaces();
  for (std::vector<MooseMesh::MortarInterface *>::iterator it = ifaces.begin(); it != ifaces.end(); ++it)
  {
    MooseMesh::MortarInterface * iface = *it;

    if (_constraints.hasActiveFaceFaceConstraints(iface->_name))
    {
      const std::vector<MooseSharedPointer<FaceFaceConstraint> > & face_constraints = _constraints.getActiveFaceFaceConstraints(iface->_name);

      // go over elements on that interface
      const std::vector<Elem *> & elems = iface->_elems;
      for (std::vector<Elem *>::const_iterator el = elems.begin(); el != elems.end(); ++el)
      {
        const Elem * elem = *el;
        // for each element process constraints on the
        _fe_problem.prepare(elem, tid);
        _fe_problem.reinitElem(elem, tid);

        for (std::vector<MooseSharedPointer<FaceFaceConstraint> >::const_iterator fc_it = face_constraints.begin(); fc_it != face_constraints.end(); ++fc_it)
        {
          MooseSharedPointer<FaceFaceConstraint> ffc = *fc_it;
          ffc->reinit();
          ffc->computeResidual();
        }
        _fe_problem.cacheResidual(tid);

        // evaluate residuals that go into master and slave side
        for (std::vector<MooseSharedPointer<FaceFaceConstraint> >::const_iterator fc_it = face_constraints.begin(); fc_it != face_constraints.end(); ++fc_it)
        {
          MooseSharedPointer<FaceFaceConstraint> ffc = *fc_it;

          ffc->reinitSide(Moose::Master);
          ffc->computeResidualSide(Moose::Master);
          _fe_problem.cacheResidual(tid);

          ffc->reinitSide(Moose::Slave);
          ffc->computeResidualSide(Moose::Slave);
          _fe_problem.cacheResidual(tid);
        }
      }
      _fe_problem.addCachedResidual(tid);
    }
  }

  // go over element-element constraint interface
  std::map<unsigned int, ElementPairLocator *> * element_pair_locators = NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    element_pair_locators = &geom_search_data._element_pair_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _fe_problem.getDisplacedProblem()->geomSearchData();
    element_pair_locators = &displaced_geom_search_data._element_pair_locators;
  }

  for (std::map<unsigned int, ElementPairLocator *>::iterator it = element_pair_locators->begin();
       it != element_pair_locators->end();
       ++it)
  {
    ElementPairLocator & elem_pair_loc = *it->second;

    if (_constraints.hasActiveElemElemConstraints(it->first))
    {
      // ElemElemConstraint objects
      const std::vector<MooseSharedPointer<ElemElemConstraint> > & _element_constraints = _constraints.getActiveElemElemConstraints(it->first);

      // go over pair elements
      for (unsigned int ie = 0; ie < elem_pair_loc._elem_pairs.size(); ++ie)
      {
        const Elem * elem = elem_pair_loc._elem_pairs[ie].first;
        const Elem * pair_elem = elem_pair_loc._elem_pairs[ie].second;

        if (elem->processor_id() != processor_id())
          continue;

        ElementPairInfo & info = *elem_pair_loc._element_pair_info[elem];
        
        // for each element process constraints on the
        for (std::vector<MooseSharedPointer<ElemElemConstraint> >::const_iterator ec_it = _element_constraints.begin(); ec_it != _element_constraints.end(); ++ec_it)
        {
          MooseSharedPointer<ElemElemConstraint> ec = *ec_it;

          _fe_problem.reinitElemPhys(elem, info._q_point, tid);
          _fe_problem.reinitNeighborPhys(pair_elem, 0, info._q_point, tid);

          ec->subProblem().prepareShapes(ec->variable().number(), tid);
          ec->subProblem().prepareNeighborShapes(ec->variable().number(), tid);

          ec->reinit(info);
          ec->computeResidual();
          _fe_problem.cacheResidual(tid);
          _fe_problem.cacheResidualNeighbor(tid);
        }
        _fe_problem.addCachedResidual(tid);
      }
    }
  }
}


void
NonlinearSystem::computeResidualInternal(Moose::KernelType type)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.residualSetup(tid);
    _nodal_kernels.residualSetup(tid);
    _dirac_kernels.residualSetup(tid);
    if (_doing_dg)
      _dg_kernels.residualSetup(tid);
    _interface_kernels.residualSetup(tid);
    _element_dampers.residualSetup(tid);
    _integrated_bcs.residualSetup(tid);
  }
  _scalar_kernels.residualSetup();
  _constraints.residualSetup();
  _general_dampers.residualSetup();
  _nodal_bcs.residualSetup();

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  // residual contributions from the domain
  PARALLEL_TRY {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeResidualThread cr(_fe_problem, *this, type);

    Threads::parallel_reduce(elem_range, cr);

    unsigned int n_threads = libMesh::n_threads();
    for (unsigned int i=0; i<n_threads; i++) // Add any cached residuals that might be hanging around
      _fe_problem.addCachedResidual(i);
  }
  PARALLEL_CATCH;

  // residual contributions from the scalar kernels
  PARALLEL_TRY {
    // do scalar kernels (not sure how to thread this)
    if (_scalar_kernels.hasActiveObjects())
    {
      const std::vector<MooseSharedPointer<ScalarKernel> > & scalars = _scalar_kernels.getActiveObjects();
      for (std::vector<MooseSharedPointer<ScalarKernel> >::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
      {
        (*it)->reinit();
        (*it)->computeResidual();
      }
      _fe_problem.addResidualScalar();
    }
  }
  PARALLEL_CATCH;

  // residual contributions from Block NodalKernels
  PARALLEL_TRY
  {
    if (_nodal_kernels.hasActiveBlockObjects())
    {
      ComputeNodalKernelsThread cnk(_fe_problem, _fe_problem.getAuxiliarySystem(), _nodal_kernels);

      ConstNodeRange & range = *_mesh.getLocalNodeRange();

      _fe_problem.reinitNode(*range.begin(), 0);

      Threads::parallel_reduce(range, cnk);

      unsigned int n_threads = libMesh::n_threads();
      for (unsigned int i = 0; i < n_threads; i++) // Add any cached residuals that might be hanging around
        _fe_problem.addCachedResidual(i);
    }
  }
  PARALLEL_CATCH;

  // residual contributions from boundary NodalKernels
  PARALLEL_TRY
  {
    if (_nodal_kernels.hasActiveBoundaryObjects())
    {
      ComputeNodalKernelBcsThread cnk(_fe_problem, _fe_problem.getAuxiliarySystem(), _nodal_kernels);

      ConstBndNodeRange & bnd_node_range = *_mesh.getBoundaryNodeRange();

      Threads::parallel_reduce(bnd_node_range, cnk);

      unsigned int n_threads = libMesh::n_threads();
      for (unsigned int i = 0; i < n_threads; i++) // Add any cached residuals that might be hanging around
        _fe_problem.addCachedResidual(i);
    }
  }
  PARALLEL_CATCH;

  if (_need_residual_copy)
  {
    residualVector(Moose::KT_NONTIME).close();
    residualVector(Moose::KT_NONTIME).localize(_residual_copy);
  }

  if (_need_residual_ghosted)
  {
    residualVector(Moose::KT_NONTIME).close();
    _residual_ghosted = residualVector(Moose::KT_NONTIME);
    _residual_ghosted.close();
  }

  PARALLEL_TRY {
    computeDiracContributions();
  }
  PARALLEL_CATCH;

  if (_fe_problem._has_constraints)
  {
    PARALLEL_TRY {
      enforceNodalConstraintsResidual(residualVector(Moose::KT_NONTIME));
    }
    PARALLEL_CATCH;
    residualVector(Moose::KT_NONTIME).close();
  }

  // Add in Residual contributions from Constraints
  if (_fe_problem._has_constraints)
  {
    PARALLEL_TRY {
      // Undisplaced Constraints
      constraintResiduals(residualVector(Moose::KT_NONTIME), false);

      // Displaced Constraints
      if (_fe_problem.getDisplacedProblem())
        constraintResiduals(residualVector(Moose::KT_NONTIME), true);
    }
    PARALLEL_CATCH;
    residualVector(Moose::KT_NONTIME).close();
  }
}

void
NonlinearSystem::computeNodalBCs(NumericVector<Number> & residual)
{
  // We need to close the diag_save_in variables on the aux system before NodalBCs clear the dofs on boundary nodes
  if (_has_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  PARALLEL_TRY {
    // last thing to do are nodal BCs
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if (node->processor_id() == processor_id())
      {
        // reinit variables in nodes
        _fe_problem.reinitNodeFace(node, boundary_id, 0);

        if (_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
        {
          const std::vector<MooseSharedPointer<NodalBC> > & bcs = _nodal_bcs.getActiveBoundaryObjects(boundary_id);
          for (std::vector<MooseSharedPointer<NodalBC> >::const_iterator it = bcs.begin(); it != bcs.end(); ++it)
            if ((*it)->shouldApply())
              (*it)->computeResidual(residual);
        }
      }
    }
  }
  PARALLEL_CATCH;

  residual.close();
  residualVector(Moose::KT_TIME).close();
  residualVector(Moose::KT_NONTIME).close();
}

void
NonlinearSystem::getNodeDofs(unsigned int node_id, std::vector<dof_id_type> & dofs)
{
  const Node & node = _mesh.node(node_id);
  unsigned int s = number();
  if (node.has_dofs(s))
  {
    for (unsigned int v = 0; v < nVariables(); v++)
      for (unsigned int c = 0; c < node.n_comp(s, v); c++)
        dofs.push_back(node.dof_number(s, v, c));
  }
}

void
NonlinearSystem::findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data, std::map<dof_id_type, std::vector<dof_id_type> > & graph)
{
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> & nearest_node_locators = geom_search_data._nearest_node_locators;

  for (std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator it = nearest_node_locators.begin();
      it != nearest_node_locators.end();
      ++it)
  {
    std::vector<dof_id_type> & slave_nodes = it->second->_slave_nodes;

    for (unsigned int i=0; i<slave_nodes.size(); i++)
    {
      std::set<dof_id_type> unique_slave_indices;
      std::set<dof_id_type> unique_master_indices;

      dof_id_type slave_node = slave_nodes[i];

      {
        std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[slave_node];

        // Get the dof indices from each elem connected to the node
        for (unsigned int el=0; el < elems.size(); ++el)
        {
          dof_id_type cur_elem = elems[el];

          std::vector<dof_id_type> dof_indices;
          dofMap().dof_indices(_mesh.elem(cur_elem), dof_indices);

          for (unsigned int di=0; di < dof_indices.size(); di++)
            unique_slave_indices.insert(dof_indices[di]);
        }
      }

      std::vector<dof_id_type> master_nodes = it->second->_neighbor_nodes[slave_node];

      for (unsigned int k=0; k<master_nodes.size(); k++)
      {
        dof_id_type master_node = master_nodes[k];

        {
          std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[master_node];

          // Get the dof indices from each elem connected to the node
          for (unsigned int el=0; el < elems.size(); ++el)
          {
            dof_id_type cur_elem = elems[el];

            std::vector<dof_id_type> dof_indices;
            dofMap().dof_indices(_mesh.elem(cur_elem), dof_indices);

            for (unsigned int di=0; di < dof_indices.size(); di++)
              unique_master_indices.insert(dof_indices[di]);
          }
        }
      }

      for (std::set<dof_id_type>::iterator sit=unique_slave_indices.begin(); sit != unique_slave_indices.end(); ++sit)
      {
        dof_id_type slave_id = *sit;

        for (std::set<dof_id_type>::iterator mit=unique_master_indices.begin(); mit != unique_master_indices.end(); ++mit)
        {
          dof_id_type master_id = *mit;

          graph[slave_id].push_back(master_id);
          graph[master_id].push_back(slave_id);
        }
      }
    }
  }

  // handle node-to-node constraints
  const std::vector<MooseSharedPointer<NodalConstraint> > & ncs = _constraints.getActiveNodalConstraints();
  for (std::vector<MooseSharedPointer<NodalConstraint> >::const_iterator it = ncs.begin(); it != ncs.end(); ++it)
  {
    MooseSharedPointer<NodalConstraint> nc = (*it);

    std::vector<dof_id_type> master_dofs;
    std::vector<dof_id_type> &  master_node_ids = nc->getMasterNodeId();
    for (std::vector<dof_id_type>::iterator mi = master_node_ids.begin(); mi != master_node_ids.end(); mi++)
      getNodeDofs(*mi, master_dofs);

    std::vector<dof_id_type> slave_dofs;
    std::vector<dof_id_type> & slave_node_ids = nc->getSlaveNodeId();
    for (std::vector<dof_id_type>::iterator si = slave_node_ids.begin(); si != slave_node_ids.end(); si++)
      getNodeDofs(*si, slave_dofs);

    for (std::vector<dof_id_type>::iterator mi = master_dofs.begin(); mi != master_dofs.end(); mi++)
    {
      dof_id_type master_id = *mi;
      for (std::vector<dof_id_type>::iterator si = slave_dofs.begin(); si != slave_dofs.end(); si++)
      {
        dof_id_type slave_id = *si;
        graph[master_id].push_back(slave_id);
        graph[slave_id].push_back(master_id);
      }
    }
  }

  // Make every entry sorted and unique
  for (std::map<dof_id_type, std::vector<dof_id_type> >::iterator git=graph.begin(); git != graph.end(); ++git)
  {
    std::vector<dof_id_type> & row = git->second;

    std::sort(row.begin(), row.end());
    std::vector<dof_id_type>::iterator uit = std::unique(row.begin(), row.end());
    row.resize(uit - row.begin());
  }
}



void
NonlinearSystem::addImplicitGeometricCouplingEntries(SparseMatrix<Number> & jacobian, GeometricSearchData & geom_search_data)
{
  std::map<dof_id_type, std::vector<dof_id_type> > graph;

  findImplicitGeometricCouplingEntries(geom_search_data, graph);

  for (std::map<dof_id_type, std::vector<dof_id_type> >::iterator git=graph.begin(); git != graph.end(); ++git)
  {
    dof_id_type dof = git->first;
    std::vector<dof_id_type> & row = git->second;

    for (unsigned int i=0; i<row.size(); i++)
    {
      dof_id_type coupled_dof = row[i];
      jacobian.add(dof, coupled_dof, 0);
    }
  }
}

void
NonlinearSystem::constraintJacobians(SparseMatrix<Number> & jacobian, bool displaced)
{
#if PETSC_VERSION_LESS_THAN(3,3,0)
#else
  if (!_fe_problem.errorOnJacobianNonzeroReallocation())
    MatSetOption(static_cast<PetscMatrix<Number> &>(*sys().matrix).mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
#endif

  std::vector<numeric_index_type> zero_rows;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied;
  if (!_assemble_constraints_separately) constraints_applied = false;
  for (std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {
    if (_assemble_constraints_separately)
    {
      // Reset the constraint_applied flag before each new constraint, as they need to be assembled separately
      constraints_applied = false;
    }
    PenetrationLocator & pen_loc = *it->second;

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    zero_rows.clear();
    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const std::vector<MooseSharedPointer<NodeFaceConstraint> > & constraints = _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (unsigned int i=0; i<slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if (slave_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            const Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _fe_problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _fe_problem.prepareAssembly(0);
            _fe_problem.reinitOffDiagScalars(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _fe_problem.reinitNeighborPhys(master_elem, master_side, points, 0);
            for (unsigned int c=0; c < constraints.size(); c++)
            {
              MooseSharedPointer<NodeFaceConstraint> nfc = constraints[c];

              nfc->_jacobian = &jacobian;

              if (nfc->shouldApply())
              {
                constraints_applied = true;

                nfc->subProblem().prepareShapes(nfc->variable().number(), 0);
                nfc->subProblem().prepareNeighborShapes(nfc->variable().number(), 0);

                nfc->computeJacobian();

                if (nfc->overwriteSlaveJacobian())
                {
                  // Add this variable's dof's row to be zeroed
                  zero_rows.push_back(nfc->variable().nodalDofIndex());
                }

                std::vector<dof_id_type> slave_dofs(1,nfc->variable().nodalDofIndex());

                // Cache the jacobian block for the slave side
                _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kee, slave_dofs, nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                // Cache the jacobian block for the master side
                if (nfc->addCouplingEntriesToJacobian())
                  _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kne, nfc->masterVariable().dofIndicesNeighbor(), nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                _fe_problem.cacheJacobian(0);
                if (nfc->addCouplingEntriesToJacobian())
                  _fe_problem.cacheJacobianNeighbor(0);

                // Do the off-diagonals next
                const std::vector<MooseVariable *> coupled_vars = nfc->getCoupledMooseVars();
                for (std::vector<MooseVariable *>::const_iterator jt = coupled_vars.begin(); jt != coupled_vars.end(); jt++)
                {
                  MooseVariable & jvar = *(*jt);

                  // Only compute jacobians for nonlinear variables
                  if (jvar.kind() != Moose::VAR_NONLINEAR)
                    continue;

                  // Only compute Jacobian entries if this coupling is being used by the preconditioner
                  if (nfc->variable().number() == jvar.number() ||
                      !_fe_problem.areCoupled(nfc->variable().number(), jvar.number()))
                    continue;

                  // Need to zero out the matrices first
                  _fe_problem.prepareAssembly(0);

                  nfc->subProblem().prepareShapes(nfc->variable().number(), 0);
                  nfc->subProblem().prepareNeighborShapes(jvar.number(), 0);

                  nfc->computeOffDiagJacobian(jvar.number());

                  // Cache the jacobian block for the slave side
                  _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kee, slave_dofs, nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                  // Cache the jacobian block for the master side
                  if (nfc->addCouplingEntriesToJacobian())
                    _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kne, nfc->variable().dofIndicesNeighbor(), nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                  _fe_problem.cacheJacobian(0);
                  if (nfc->addCouplingEntriesToJacobian())
                    _fe_problem.cacheJacobianNeighbor(0);
                }
              }
            }
          }
        }
      }
    }
    if (_assemble_constraints_separately)
    {
      // See if constraints were applied anywhere
      _communicator.max(constraints_applied);

      if (constraints_applied)
      {
#ifdef LIBMESH_HAVE_PETSC
        //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
        MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3,1,0)
        // In Petsc 3.0.0, MatSetOption has three args...the third arg
        // determines whether the option is set (true) or unset (false)
        MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                     MAT_KEEP_ZEROED_ROWS,
                     PETSC_TRUE);
#else
        MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                     MAT_KEEP_NONZERO_PATTERN,  // This is changed in 3.1
                     PETSC_TRUE);
#endif
#endif

        jacobian.close();
        jacobian.zero_rows(zero_rows, 0.0);
        jacobian.close();
        _fe_problem.addCachedJacobian(jacobian, 0);
        jacobian.close();
      }
    }
  }
  if (!_assemble_constraints_separately)
  {
    // See if constraints were applied anywhere
    _communicator.max(constraints_applied);

    if (constraints_applied)
    {
#ifdef LIBMESH_HAVE_PETSC
      //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3,1,0)
      // In Petsc 3.0.0, MatSetOption has three args...the third arg
      // determines whether the option is set (true) or unset (false)
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                   MAT_KEEP_ZEROED_ROWS,
                   PETSC_TRUE);
#else
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                   MAT_KEEP_NONZERO_PATTERN,  // This is changed in 3.1
                   PETSC_TRUE);
#endif
#endif

      jacobian.close();
      jacobian.zero_rows(zero_rows, 0.0);
      jacobian.close();
      _fe_problem.addCachedJacobian(jacobian, 0);
      jacobian.close();
    }
  }

  THREAD_ID tid = 0;
  // go over mortar interfaces
  std::vector<MooseMesh::MortarInterface *> & ifaces = _mesh.getMortarInterfaces();
  for (std::vector<MooseMesh::MortarInterface *>::iterator it = ifaces.begin(); it != ifaces.end(); ++it)
  {
    MooseMesh::MortarInterface * iface = *it;

    if (_constraints.hasActiveFaceFaceConstraints(iface->_name))
    {
      // FaceFaceConstraint objects
      const std::vector<MooseSharedPointer<FaceFaceConstraint> > & face_constraints = _constraints.getActiveFaceFaceConstraints(iface->_name);

      // go over elements on that interface
      const std::vector<Elem *> & elems = iface->_elems;
      for (std::vector<Elem *>::const_iterator el = elems.begin(); el != elems.end(); ++el)
      {
        const Elem * elem = *el;

        // for each element process constraints on the
        for (std::vector<MooseSharedPointer<FaceFaceConstraint> >::const_iterator fc_it = face_constraints.begin(); fc_it != face_constraints.end(); ++fc_it)
        {
          MooseSharedPointer<FaceFaceConstraint> ffc = *fc_it;

          _fe_problem.prepare(elem, tid);
          _fe_problem.reinitElem(elem, tid);
          ffc->reinit();
          ffc->subProblem().prepareShapes(ffc->variable().number(), tid);
          ffc->computeJacobian();
          _fe_problem.cacheJacobian(tid);

          ffc->reinitSide(Moose::Master);
          ffc->computeJacobianSide(Moose::Master);
          _fe_problem.cacheJacobian(tid);

          ffc->reinitSide(Moose::Slave);
          ffc->computeJacobianSide(Moose::Slave);
          _fe_problem.cacheJacobian(tid);
        }

        _fe_problem.addCachedJacobian(jacobian, tid);
      }
    }
  }

  // go over element-element constraint interface
  std::map<unsigned int, ElementPairLocator *> * element_pair_locators = NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    element_pair_locators = &geom_search_data._element_pair_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _fe_problem.getDisplacedProblem()->geomSearchData();
    element_pair_locators = &displaced_geom_search_data._element_pair_locators;
  }

  for (std::map<unsigned int, ElementPairLocator *>::iterator it = element_pair_locators->begin();
       it != element_pair_locators->end();
       ++it)
  {
    ElementPairLocator & elem_pair_loc = *it->second;

    if (_constraints.hasActiveElemElemConstraints(it->first))
    {
      // ElemElemConstraint objects
      const std::vector<MooseSharedPointer<ElemElemConstraint> > & _element_constraints = _constraints.getActiveElemElemConstraints(it->first);

      // go over pair elements
      for (unsigned int ie = 0; ie < elem_pair_loc._elem_pairs.size(); ++ie)
      {
        const Elem * elem = elem_pair_loc._elem_pairs[ie].first;
        const Elem * pair_elem = elem_pair_loc._elem_pairs[ie].second;

        if (elem->processor_id() != processor_id())
          continue;

        ElementPairInfo & info = *elem_pair_loc._element_pair_info[elem];
        
        // for each element process constraints on the
        for (std::vector<MooseSharedPointer<ElemElemConstraint> >::const_iterator ec_it = _element_constraints.begin(); ec_it != _element_constraints.end(); ++ec_it)
        {
          MooseSharedPointer<ElemElemConstraint> ec = *ec_it;

          _fe_problem.reinitElemPhys(elem, info._q_point, tid);
          _fe_problem.reinitNeighborPhys(pair_elem, 0, info._q_point, tid);

          ec->subProblem().prepareShapes(ec->variable().number(), tid);
          ec->subProblem().prepareNeighborShapes(ec->variable().number(), tid);

          ec->reinit(info);
          ec->computeJacobian();
          _fe_problem.cacheJacobian(tid);
          _fe_problem.cacheJacobianNeighbor(tid);
        }
        _fe_problem.addCachedJacobian(jacobian, tid);
      }
    }
  }
}

void
NonlinearSystem::computeScalarKernelsJacobians(SparseMatrix<Number> & jacobian)
{
  // Compute the diagonal block for scalar variables
  if (_scalar_kernels.hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<ScalarKernel> > & scalars = _scalar_kernels.getActiveObjects();

    _fe_problem.reinitScalars(/*tid=*/0);
    for (std::vector<MooseSharedPointer<ScalarKernel> >::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
    {
      MooseSharedPointer<ScalarKernel> kernel = *it;

      kernel->reinit();
      kernel->computeJacobian();
      _fe_problem.addJacobianOffDiagScalar(jacobian, kernel->variable().number());
    }
    _fe_problem.addJacobianScalar(jacobian);
  }
}

void
NonlinearSystem::computeJacobianInternal(SparseMatrix<Number> &  jacobian)
{
#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3,1,0)
  // In Petsc 3.0.0, MatSetOption has three args...the third arg
  // determines whether the option is set (true) or unset (false)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
    MAT_KEEP_ZEROED_ROWS,
    PETSC_TRUE);
#else
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
    MAT_KEEP_NONZERO_PATTERN,  // This is changed in 3.1
    PETSC_TRUE);
#endif
#if PETSC_VERSION_LESS_THAN(3,3,0)
#else
  if (!_fe_problem.errorOnJacobianNonzeroReallocation())
    MatSetOption(static_cast<PetscMatrix<Number> &>(*sys().matrix).mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
#endif

#endif

  // jacobianSetup /////
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.jacobianSetup(tid);
    _nodal_kernels.jacobianSetup(tid);
    _dirac_kernels.jacobianSetup(tid);
    if (_doing_dg)
      _dg_kernels.jacobianSetup(tid);
    _interface_kernels.jacobianSetup(tid);
    _element_dampers.jacobianSetup(tid);
    _integrated_bcs.jacobianSetup(tid);
  }
  _scalar_kernels.jacobianSetup();
  _constraints.jacobianSetup();
  _general_dampers.jacobianSetup();
  _nodal_bcs.jacobianSetup();

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    switch (_fe_problem.coupling())
    {
    case Moose::COUPLING_DIAG:
      {
        ComputeJacobianThread cj(_fe_problem, *this, jacobian);
        Threads::parallel_reduce(elem_range, cj);

        unsigned int n_threads = libMesh::n_threads();
        for (unsigned int i=0; i<n_threads; i++) // Add any Jacobian contributions still hanging around
          _fe_problem.addCachedJacobian(jacobian, i);

        // Block restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBlockObjects())
        {
          ComputeNodalKernelJacobiansThread cnkjt(_fe_problem, _fe_problem.getAuxiliarySystem(), _nodal_kernels, jacobian);
          ConstNodeRange & range = *_mesh.getLocalNodeRange();
          Threads::parallel_reduce(range, cnkjt);

          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads; i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }

        // Boundary restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBoundaryObjects())
        {
          ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, _fe_problem.getAuxiliarySystem(), _nodal_kernels, jacobian);
          ConstBndNodeRange & bnd_range = *_mesh.getBoundaryNodeRange();

          Threads::parallel_reduce(bnd_range, cnkjt);
          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads; i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }
      }
      break;

    default:
    case Moose::COUPLING_CUSTOM:
      {
        ComputeFullJacobianThread cj(_fe_problem, *this, jacobian);
        Threads::parallel_reduce(elem_range, cj);
        unsigned int n_threads = libMesh::n_threads();

        for (unsigned int i=0; i<n_threads; i++)
          _fe_problem.addCachedJacobian(jacobian, i);

        // Block restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBlockObjects())
        {
          ComputeNodalKernelJacobiansThread cnkjt(_fe_problem, _fe_problem.getAuxiliarySystem(), _nodal_kernels, jacobian);
          ConstNodeRange & range = *_mesh.getLocalNodeRange();
          Threads::parallel_reduce(range, cnkjt);

          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads; i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }

        // Boundary restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBoundaryObjects())
        {
          ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, _fe_problem.getAuxiliarySystem(), _nodal_kernels, jacobian);
          ConstBndNodeRange & bnd_range = *_mesh.getBoundaryNodeRange();

          Threads::parallel_reduce(bnd_range, cnkjt);

          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads; i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }
      }
      break;
    }

    computeDiracContributions(&jacobian);
    computeScalarKernelsJacobians(jacobian);

    static bool first = true;

    // This adds zeroes into geometric coupling entries to ensure they stay in the matrix
    if (first && (_add_implicit_geometric_coupling_entries_to_jacobian))
    {
      first = false;
      addImplicitGeometricCouplingEntries(jacobian, _fe_problem.geomSearchData());

      if (_fe_problem.getDisplacedProblem())
        addImplicitGeometricCouplingEntries(jacobian, _fe_problem.getDisplacedProblem()->geomSearchData());
    }
  }
  PARALLEL_CATCH;
  jacobian.close();

  PARALLEL_TRY {
    // Add in Jacobian contributions from Constraints
    if (_fe_problem._has_constraints)
    {
      // Nodal Constraints
      enforceNodalConstraintsJacobian(jacobian);

      // Undisplaced Constraints
      constraintJacobians(jacobian, false);

      // Displaced Constraints
      if (_fe_problem.getDisplacedProblem())
        constraintJacobians(jacobian, true);
    }
  }
  PARALLEL_CATCH;
  jacobian.close();

  // We need to close the save_in variables on the aux system before NodalBCs clear the dofs on boundary nodes
  if (_has_diag_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  PARALLEL_TRY {
    // Cache the information about which BCs are coupled to which
    // variables, so we don't have to figure it out for each node.
    std::map<std::string, std::set<unsigned int> > bc_involved_vars;
    const std::set<BoundaryID> & all_boundary_ids = _mesh.getBoundaryIDs();
    for (std::set<BoundaryID>::const_iterator it=all_boundary_ids.begin(); it != all_boundary_ids.end(); ++it)
    {
      // Get reference to all the NodalBCs for this ID.  This is only
      // safe if there are NodalBCs there to be gotten...
      if (_nodal_bcs.hasActiveBoundaryObjects(*it))
      {
        const std::vector<MooseSharedPointer<NodalBC> > & bcs = _nodal_bcs.getActiveBoundaryObjects(*it);
        for (std::vector<MooseSharedPointer<NodalBC> >::const_iterator bc_it = bcs.begin(); bc_it != bcs.end(); ++bc_it)
        {
          MooseSharedPointer<NodalBC> bc = *bc_it;

          const std::vector<MooseVariable *> & coupled_moose_vars = bc->getCoupledMooseVars();

          // Create the set of "involved" MOOSE nonlinear vars, which includes all coupled vars and the BC's own variable
          std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];
          for (unsigned int var = 0; var < coupled_moose_vars.size(); ++var)
          {
            if (coupled_moose_vars[var]->kind() == Moose::VAR_NONLINEAR)
              var_set.insert(coupled_moose_vars[var]->number());
          }

          var_set.insert(bc->variable().number());
        }
      }
    }

    // Get variable coupling list.  We do all the NodalBC stuff on
    // thread 0...  The couplingEntries() data structure determines
    // which variables are "coupled" as far as the preconditioner is
    // concerned, not what variables a boundary condition specifically
    // depends on.
    std::vector<std::pair<MooseVariable *, MooseVariable *> > & coupling_entries = _fe_problem.couplingEntries(/*_tid=*/0);

    // Compute Jacobians for NodalBCs
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin(); nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if (_nodal_bcs.hasActiveBoundaryObjects(boundary_id) && node->processor_id() == processor_id())
      {
        _fe_problem.reinitNodeFace(node, boundary_id, 0);

        const std::vector<MooseSharedPointer<NodalBC> > & bcs = _nodal_bcs.getActiveBoundaryObjects(boundary_id);
        for (std::vector<MooseSharedPointer<NodalBC> >::const_iterator bc_it = bcs.begin(); bc_it != bcs.end(); ++bc_it)
        {
          MooseSharedPointer<NodalBC> bc = *bc_it;

          // Get the set of involved MOOSE vars for this BC
          std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];

          // Loop over all the variables whose Jacobian blocks are
          // actually being computed, call computeOffDiagJacobian()
          // for each one which is actually coupled (otherwise the
          // value is zero.)
          for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = coupling_entries.begin();
               it != coupling_entries.end(); ++it)
          {
            unsigned int
              ivar = it->first->number(),
              jvar = it->second->number();

            // We are only going to call computeOffDiagJacobian() if:
            // 1.) the BC's variable is ivar
            // 2.) jvar is "involved" with the BC (including jvar==ivar), and
            // 3.) the BC should apply.
            if ((bc->variable().number() == ivar) && var_set.count(jvar) && bc->shouldApply())
              bc->computeOffDiagJacobian(jvar);
          }
        }
      }
    } // end loop over boundary nodes

    // Set the cached NodalBC values in the Jacobian matrix
    _fe_problem.assembly(0).setCachedJacobianContributions(jacobian);
  }
  PARALLEL_CATCH;
  jacobian.close();

  // We need to close the save_in variables on the aux system before NodalBCs clear the dofs on boundary nodes
  if (_has_nodalbc_diag_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  if (hasDiagSaveIn())
    _fe_problem.getAuxiliarySystem().update();
}

void
NonlinearSystem::computeJacobian(SparseMatrix<Number> & jacobian)
{
  Moose::perf_log.push("compute_jacobian()", "Execution");

  Moose::enableFPE();

  try {
    jacobian.zero();
    computeJacobianInternal(jacobian);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_jacobian()", "Execution");
}

void
NonlinearSystem::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  Moose::perf_log.push("compute_jacobian_block()", "Execution");

  Moose::enableFPE();

  for (unsigned int i=0; i<blocks.size(); i++)
  {
    SparseMatrix<Number> & jacobian = blocks[i]->_jacobian;

#ifdef LIBMESH_HAVE_PETSC
    //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3,1,0)
    // In Petsc 3.0.0, MatSetOption has three args...the third arg
    // determines whether the option is set (true) or unset (false)
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                 MAT_KEEP_ZEROED_ROWS,
                 PETSC_TRUE);
#else
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                 MAT_KEEP_NONZERO_PATTERN,  // This is changed in 3.1
                 PETSC_TRUE);
#endif
#if PETSC_VERSION_LESS_THAN(3,3,0)
#else
  if (!_fe_problem.errorOnJacobianNonzeroReallocation())
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
#endif

#endif

    jacobian.zero();
  }

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeJacobianBlocksThread cjb(_fe_problem, blocks);
    Threads::parallel_reduce(elem_range, cjb);
  }
  PARALLEL_CATCH;

  for (unsigned int i=0; i<blocks.size(); i++)
    blocks[i]->_jacobian.close();


  for (unsigned int i=0; i<blocks.size(); i++)
  {
    libMesh::System & precond_system = blocks[i]->_precond_system;
    SparseMatrix<Number> & jacobian = blocks[i]->_jacobian;

    unsigned int ivar = blocks[i]->_ivar;
    unsigned int jvar = blocks[i]->_jvar;

    //Dirichlet BCs
    std::vector<numeric_index_type> zero_rows;
    PARALLEL_TRY {
      ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
      for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
      {
        const BndNode * bnode = *nd;
        BoundaryID boundary_id = bnode->_bnd_id;
        Node * node = bnode->_node;

        if (_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
        {
          const std::vector<MooseSharedPointer<NodalBC> > & bcs = _nodal_bcs.getActiveBoundaryObjects(boundary_id);
          {
            if (node->processor_id() == processor_id())
            {
              _fe_problem.reinitNodeFace(node, boundary_id, 0);

              for (std::vector<MooseSharedPointer<NodalBC> >::const_iterator it = bcs.begin(); it != bcs.end(); ++it)
              {
                MooseSharedPointer<NodalBC> bc = *it;
                if (bc->variable().number() == ivar && bc->shouldApply())
                {
                  //The first zero is for the variable number... there is only one variable in each mini-system
                  //The second zero only works with Lagrange elements!
                  zero_rows.push_back(node->dof_number(precond_system.number(), 0, 0));
                }
              }
            }
          }
        }
      }
    }
    PARALLEL_CATCH;

    jacobian.close();

    //This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
    if (ivar == jvar)
      jacobian.zero_rows(zero_rows, 1.0);
    else
      jacobian.zero_rows(zero_rows, 0.0);

    jacobian.close();
  }

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_jacobian_block()", "Execution");
}

void
NonlinearSystem::updateActive(THREAD_ID tid)
{
  _element_dampers.updateActive(tid);
  _integrated_bcs.updateActive(tid);
  _dg_kernels.updateActive(tid);
  _interface_kernels.updateActive(tid);
  _dirac_kernels.updateActive(tid);
  _kernels.updateActive(tid);
  _nodal_kernels.updateActive(tid);
  if (tid == 0)
  {
    _general_dampers.updateActive();
    _nodal_bcs.updateActive();
    _preset_nodal_bcs.updateActive();
    _constraints.updateActive();
    _scalar_kernels.updateActive();
  }
}

Real
NonlinearSystem::computeDamping(const NumericVector<Number> & solution,
                                const NumericVector<Number> & update)
{
  Moose::perf_log.push("compute_dampers()", "Execution");

  // Default to no damping
  Real damping = 1.0;

  if (_element_dampers.hasActiveObjects())
  {
    *_increment_vec = update;
    ComputeElemDampingThread cid(_fe_problem, *this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cid);
    damping = cid.damping();
  }

  if (_general_dampers.hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<GeneralDamper> > & gdampers = _general_dampers.getActiveObjects();
    for (std::vector<MooseSharedPointer<GeneralDamper> >::const_iterator it = gdampers.begin(); it != gdampers.end(); ++it)
    {
      Real gd_damping = (*it)->computeDamping(solution, update);
      if (gd_damping < damping)
        damping = gd_damping;
    }
  }

  _communicator.min(damping);

  Moose::perf_log.pop("compute_dampers()", "Execution");

  return damping;
}

void
NonlinearSystem::computeDiracContributions(SparseMatrix<Number> * jacobian)
{
  Moose::perf_log.push("computeDiracContributions()", "Execution");

  _fe_problem.clearDiracInfo();

  std::set<const Elem *> dirac_elements;

  if (_dirac_kernels.hasActiveObjects())
  {

    // TODO: Need a threading fix... but it's complicated!
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      const std::vector<MooseSharedPointer<DiracKernel> > & dkernels = _dirac_kernels.getActiveObjects(tid);
      for (std::vector<MooseSharedPointer<DiracKernel> >::const_iterator it = dkernels.begin(); it != dkernels.end(); ++it)
      {
        (*it)->clearPoints();
        (*it)->addPoints();
      }
    }

    ComputeDiracThread cd(_fe_problem, *this, jacobian);

    _fe_problem.getDiracElements(dirac_elements);

    DistElemRange range(dirac_elements.begin(),
                        dirac_elements.end(),
                        1);
    // TODO: Make Dirac work thread!
    //Threads::parallel_reduce(range, cd);

    cd(range);
  }

  if (jacobian == NULL)
    residualVector(Moose::KT_NONTIME).close();

  Moose::perf_log.pop("computeDiracContributions()", "Execution");
}

NumericVector<Number> &
NonlinearSystem::residualCopy()
{
  _need_residual_copy = true;
  return _residual_copy;
}

NumericVector<Number> &
NonlinearSystem::residualGhosted()
{
  _need_residual_ghosted = true;
  return _residual_ghosted;
}

void
NonlinearSystem::augmentSparsity(SparsityPattern::Graph & sparsity,
                                 std::vector<dof_id_type> & n_nz,
                                 std::vector<dof_id_type> & n_oz)
{

  if (_add_implicit_geometric_coupling_entries_to_jacobian)
  {
    _fe_problem.updateGeomSearch();

    std::map<dof_id_type, std::vector<dof_id_type> > graph;

    findImplicitGeometricCouplingEntries(_fe_problem.geomSearchData(), graph);

    if (_fe_problem.getDisplacedProblem())
      findImplicitGeometricCouplingEntries(_fe_problem.getDisplacedProblem()->geomSearchData(), graph);

    const dof_id_type first_dof_on_proc = dofMap().first_dof(processor_id());
    const dof_id_type end_dof_on_proc   = dofMap().end_dof(processor_id());

    // If we're on the last processor then we have all of the scalar dofs
    const dof_id_type n_scalar_dofs_on_proc = processor_id() == n_processors()-1 ? dofMap().n_SCALAR_dofs() : 0;

    // If we're _not_ on the last processor then all of the scalar dofs are off processor (n_oz)
    const dof_id_type n_scalar_dofs_not_on_proc = processor_id() == n_processors()-1 ? 0 : dofMap().n_SCALAR_dofs();

    // The total number of dofs on and off processor (including scalar dofs)
    const dof_id_type n_dofs_on_proc = dofMap().n_local_dofs() + n_scalar_dofs_on_proc;
    const dof_id_type n_dofs_not_on_proc = (dofMap().n_dofs() - dofMap().n_local_dofs()) + n_scalar_dofs_not_on_proc;

    unsigned int max_nz = sparsity.size();

    for (std::map<dof_id_type, std::vector<dof_id_type> >::iterator git=graph.begin(); git != graph.end(); ++git)
    {
      dof_id_type dof = git->first;
      dof_id_type local_dof = dof - first_dof_on_proc;

      if (dof < first_dof_on_proc || dof >= end_dof_on_proc)
        continue;

      std::vector<dof_id_type> & row = git->second;

      SparsityPattern::Row & sparsity_row = sparsity[local_dof];

      unsigned int original_row_length = sparsity_row.size();

      sparsity_row.insert(sparsity_row.end(), row.begin(), row.end());

      SparsityPattern::sort_row(sparsity_row.begin(), sparsity_row.begin()+original_row_length, sparsity_row.end());

      // Fix up nonzero arrays
      for (unsigned int i=0; i<row.size(); i++)
      {
        dof_id_type coupled_dof = row[i];

        if (coupled_dof < first_dof_on_proc || coupled_dof >= end_dof_on_proc)
        {
          if (n_oz[local_dof] < max_nz)
            n_oz[local_dof]++;
        }
        else
        {
          if (n_nz[local_dof] < max_nz)
            n_nz[local_dof]++;
        }
      }

      // Safety
      n_nz[local_dof] = std::min(n_nz[local_dof], n_dofs_on_proc);
      n_oz[local_dof] = std::min(n_oz[local_dof], n_dofs_not_on_proc);
    }
  }
}

void
NonlinearSystem::serializeSolution()
{
  if (_need_serialized_solution)
  {
    if (!_serialized_solution.initialized() || _serialized_solution.size() != _sys.n_dofs())
    {
      _serialized_solution.clear();
      _serialized_solution.init(_sys.n_dofs(), false, SERIAL);
    }

    _current_solution->localize(_serialized_solution);
  }
 }

void
NonlinearSystem::setSolution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  if (_need_serialized_solution)
    serializeSolution();
}

void
NonlinearSystem::setSolutionUDot(const NumericVector<Number> & udot)
{
  _u_dot = udot;
}

NumericVector<Number> &
NonlinearSystem::serializedSolution()
{
  if (!_serialized_solution.initialized())
    _serialized_solution.init(_sys.n_dofs(), false, SERIAL);

  _need_serialized_solution = true;
  return _serialized_solution;
}

void
NonlinearSystem::setPreconditioner(MooseSharedPointer<MoosePreconditioner> pc)
{
  if (_preconditioner.get() != NULL)
    mooseError("More than one active Preconditioner detected");

  _preconditioner = pc;
}

void
NonlinearSystem::setupDampers()
{
  _increment_vec = &_sys.add_vector("u_increment", true, GHOSTED);
}

void
NonlinearSystem::reinitIncrementForDampers(THREAD_ID tid)
{
  // Find the variables that are being operated on by active element dampers
  std::set<MooseVariable *> damped_vars;

  const std::vector<MooseSharedPointer<ElementDamper> > & edampers = _element_dampers.getActiveObjects(tid);
  for (std::vector<MooseSharedPointer<ElementDamper> >::const_iterator it = edampers.begin(); it != edampers.end(); ++it)
    damped_vars.insert( (*it)->getVariable() );

  for (std::set<MooseVariable *>::const_iterator it = damped_vars.begin();
       it != damped_vars.end(); ++it)
  {
    MooseVariable * var = *it;
    var->computeIncrement(*_increment_vec);
  }
}

void
NonlinearSystem::checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains) const
{
  // Check kernel coverage of subdomains (blocks) in your mesh
  std::set<SubdomainID> input_subdomains;
  std::set<std::string> kernel_variables;

  bool global_kernels_exist = _kernels.hasActiveBlockObjects(Moose::ANY_BLOCK_ID);
  global_kernels_exist |= _scalar_kernels.hasActiveObjects();
  global_kernels_exist |= _nodal_kernels.hasActiveObjects();

  _kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _nodal_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _scalar_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _constraints.subdomainsCovered(input_subdomains, kernel_variables);

  if (!global_kernels_exist)
  {
    std::set<SubdomainID> difference;
    std::set_difference (mesh_subdomains.begin(), mesh_subdomains.end(),
                         input_subdomains.begin(), input_subdomains.end(),
                         std::inserter(difference, difference.end()));

    if (!difference.empty())
    {
      std::stringstream missing_block_ids;
      std::copy (difference.begin(), difference.end(), std::ostream_iterator<unsigned int>( missing_block_ids, " "));
      mooseError("Each subdomain must contain at least one Kernel.\nThe following block(s) lack an active kernel: "
                 + missing_block_ids.str());
    }
  }

  std::set<VariableName> variables(getVariableNames().begin(), getVariableNames().end());

  std::set<VariableName> difference;
  std::set_difference (variables.begin(), variables.end(),
                       kernel_variables.begin(), kernel_variables.end(),
                       std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::stringstream missing_kernel_vars;
    std::copy (difference.begin(), difference.end(), std::ostream_iterator<std::string>( missing_kernel_vars, " "));
    mooseError("Each variable must be referenced by at least one active Kernel.\nThe following variable(s) lack an active kernel: "
               + missing_kernel_vars.str());
  }
}

bool
NonlinearSystem::containsTimeKernel()
{
  return _time_kernels.hasActiveObjects();
}

void
NonlinearSystem::setPCSide(MooseEnum pcs)
{
  if (pcs == "left")
    _pc_side = Moose::PCS_LEFT;
  else if (pcs == "right")
    _pc_side = Moose::PCS_RIGHT;
  else if (pcs == "symmetric")
    _pc_side = Moose::PCS_SYMMETRIC;
  else
    mooseError("Unknown PC side specified.");
}

bool
NonlinearSystem::needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const
{
  return _integrated_bcs.hasActiveBoundaryObjects(bnd_id, tid);
}

bool
NonlinearSystem::needMaterialOnSide(SubdomainID /*subdomain_id*/, THREAD_ID /*tid*/) const
{
  return _doing_dg;
}

bool
NonlinearSystem::doingDG() const
{
  return _doing_dg;
}
