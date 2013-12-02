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
#include "ComputeJacobianBlockThread.h"
#include "ComputeDiracThread.h"
#include "ComputeDampingThread.h"
#include "TimeKernel.h"
#include "BoundaryCondition.h"
#include "PresetNodalBC.h"
#include "NodalBC.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
#include "Damper.h"
#include "DisplacedProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "ScalarKernel.h"
#include "Parser.h"
#include "Split.h"
#include "SplitBasedPreconditioner.h"

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
} // namespace Moose


NonlinearSystem::NonlinearSystem(FEProblem & fe_problem, const std::string & name) :
    SystemTempl<TransientNonlinearImplicitSystem>(fe_problem, name, Moose::VAR_NONLINEAR),
    _fe_problem(fe_problem),
    _last_rnorm(0.),
    _last_nl_rnorm(0.),
    _l_abs_step_tol(1e-10),
    _initial_residual(0.),
    _current_nl_its(0),
    _current_solution(NULL),
    _residual_ghosted(addVector("residual_ghosted", false, GHOSTED)),
    _serialized_solution(*NumericVector<Number>::build().release()),
    _residual_copy(*NumericVector<Number>::build().release()),
    _time_integrator(NULL),
    _u_dot(addVector("u_dot", true, GHOSTED)),
    _du_dot_du(addVector("du_dot_du", true, GHOSTED)),
    _Re_time(addVector("Re_time", false, GHOSTED)),
    _Re_non_time(addVector("Re_non_time", false, GHOSTED)),
    _increment_vec(NULL),
    _preconditioner(NULL),
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
    _predictor(NULL),
    _computing_initial_residual(false),
    _print_all_var_norms(false)
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

  unsigned int n_threads = libMesh::n_threads();
  _kernels.resize(n_threads);
  _bcs.resize(n_threads);
  _dirac_kernels.resize(n_threads);
  _dg_kernels.resize(n_threads);
  _dampers.resize(n_threads);
  _constraints.resize(n_threads);
}

NonlinearSystem::~NonlinearSystem()
{
  delete _time_integrator;
  delete _preconditioner;
  delete _predictor;
  delete &_serialized_solution;
  delete &_residual_copy;
}

void
NonlinearSystem::init()
{
  setupDampers();

  _current_solution = _sys.current_local_solution.get();

  if(_need_serialized_solution)
  {
    Moose::setup_perf_log.push("Init serialized_solution","Setup");
    _serialized_solution.init(_sys.n_dofs(), false, SERIAL);
    Moose::setup_perf_log.pop("Init serialized_solution","Setup");
  }

  if(_need_residual_copy)
  {
    Moose::setup_perf_log.push("Init residual_copy","Setup");
    _residual_copy.init(_sys.n_dofs(), false, SERIAL);
    Moose::setup_perf_log.pop("Init residual_copy","Setup");
  }
}

void
NonlinearSystem::solve()
{
  try
  {
    if (_fe_problem.solverParams()._type != Moose::ST_LINEAR)
    {
      //Calculate the initial residual for use in the convergence criterion.  The initial
      //residual
      _computing_initial_residual = true;
      _fe_problem.computeResidual(_sys, *_current_solution, *_sys.rhs);
      _computing_initial_residual = false;
      _sys.rhs->close();
      _initial_residual = _sys.rhs->l2_norm();
      Moose::out << std::scientific << std::setprecision(6);
      Moose::out << " Initial |R| = " << _initial_residual << std::endl;
    }
  }
  catch (MooseException & e)
  {
    // re-throw the exception, we can do this since we did not enter the solve() call yet
    throw;
  }

  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;

  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  if(_use_finite_differenced_preconditioner)
    setupFiniteDifferencedPreconditioner();

  if(_have_decomposition)
    setupDecomposition();

  if(_use_split_based_preconditioner)
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

  // we are back from the libMesh solve, so re-throw the exception if we got one;
  if (_exception > 0)
    throw _exception;
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
NonlinearSystem::initialSetup()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
    _constraints[i].initialSetup();
}

void
NonlinearSystem::initialSetupBCs()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
    _bcs[i].initialSetup();
}

void
NonlinearSystem::initialSetupKernels()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _kernels[i].initialSetup();
    _dirac_kernels[i].initialSetup();
    if (_doing_dg) _dg_kernels[i].initialSetup();
  }
}

void
NonlinearSystem::timestepSetup()
{
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _kernels[i].timestepSetup();
    _bcs[i].timestepSetup();
    _dirac_kernels[i].timestepSetup();
    _constraints[i].timestepSetup();
    if (_doing_dg) _dg_kernels[i].timestepSetup();
  }
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
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORING_LF, &iscoloring);
#else
  // PETSc 3.3.0
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORINGLF, &iscoloring);
#endif
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  MatFDColoringCreate(petsc_mat->mat(),iscoloring, &_fdcoloring);
  MatFDColoringSetFromOptions(_fdcoloring);
  MatFDColoringSetFunction(_fdcoloring,
                           (PetscErrorCode (*)(void))&libMesh::__libmesh_petsc_snes_residual,
                           &petsc_nonlinear_solver);
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

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
#undef __FUNCT__
#define __FUNCT__ "SNESUpdateMoose"
static PetscErrorCode  SNESUpdateMoose(SNES snes, PetscInt iteration)
{
  PetscErrorCode ierr;
  DM dm;
  KSP ksp;
  const char* prefix;
  MPI_Comm comm;
  PC pc;

  PetscFunctionBegin;
  if (iteration) {
    /* TODO: limit this only to situations when displaced (un)contact splits are present, as is DisplacedProblem(). */
    ierr = SNESGetDM(snes,&dm);CHKERRQ(ierr);
    ierr = DMMooseReset(dm);CHKERRQ(ierr);
    ierr = DMSetUp(dm);CHKERRQ(ierr);
    ierr = SNESGetKSP(snes,&ksp);CHKERRQ(ierr);
    /* Should we rebuild the whole KSP? */
    ierr = PetscObjectGetOptionsPrefix((PetscObject)ksp,&prefix);CHKERRQ(ierr);
    ierr = PetscObjectGetComm((PetscObject)ksp,&comm);CHKERRQ(ierr);
    ierr = PCCreate(comm,&pc);CHKERRQ(ierr);
    ierr = PCSetDM(pc,dm);CHKERRQ(ierr);
    ierr = PCSetOptionsPrefix(pc,prefix);CHKERRQ(ierr);
    ierr = PCSetFromOptions(pc);CHKERRQ(ierr);
    ierr = KSPSetPC(ksp,pc);CHKERRQ(ierr);
    ierr = PCDestroy(&pc);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}
#endif

void
NonlinearSystem::setupDecomposition()
{
  Split* top_split = getSplit(_decomposition_split);
  top_split->setup();

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
  static bool     DMMooseRegistered = false;
  PetscErrorCode  ierr;

  // Create and set up the DM that will consume the split options set above.
  if (!DMMooseRegistered) {
#if PETSC_VERSION_LESS_THAN(3,4,0)
    ierr = DMRegister(DMMOOSE, PETSC_NULL, "DMCreate_Moose", DMCreate_Moose);
    CHKERRABORT(libMesh::COMM_WORLD, ierr);
#else
    ierr = DMRegister(DMMOOSE, DMCreate_Moose);
    CHKERRABORT(libMesh::COMM_WORLD, ierr);
#endif
    DMMooseRegistered = true;
  }

  PetscNonlinearSolver<Number> *petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();
  /* FIXME: reset the DM, do not recreate it anew every time? */
  DM dm = PETSC_NULL;
  ierr = DMCreateMoose(libMesh::COMM_WORLD, *this, &dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMSetFromOptions(dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMSetUp(dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = SNESSetDM(snes,dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMDestroy(&dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = SNESSetUpdate(snes,SNESUpdateMoose);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
#endif
}

void
NonlinearSystem::setupSplitBasedPreconditioner()
{
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
   SplitBasedPreconditioner* sbp = dynamic_cast<SplitBasedPreconditioner*>(_preconditioner);
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
  return _sys.nonlinear_solver->converged;
}

void
NonlinearSystem::addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters)
{
  TimeIntegrator * ti = static_cast<TimeIntegrator *>(_factory.create(type, name, parameters));
  if (ti == NULL)
    mooseError("Not an time integrator object.");
  _time_integrator = ti;
}

void
NonlinearSystem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Set the parameters for thread ID and material data
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _fe_problem._material_data[tid];

    // Create the kernel object via the factory
    Kernel *kernel = static_cast<Kernel *>(_factory.create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Kernel object");

    // Extract the SubdomainIDs from the object (via BlockRestrictable class)
    std::set<SubdomainID> blk_ids = kernel->blockIDs();

    // Add the kernel to the warehouse
    _kernels[tid].addKernel(kernel, blk_ids);
    _fe_problem._objects_by_name[tid][name].push_back(kernel);
  }
}

void
NonlinearSystem::addScalarKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    ScalarKernel *kernel = static_cast<ScalarKernel *>(_factory.create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Kernel object");

    _kernels[tid].addScalarKernel(kernel);
    _fe_problem._objects_by_name[tid][name].push_back(kernel);
  }
}

void
NonlinearSystem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  std::vector<BoundaryName> boundaries = parameters.get<std::vector<BoundaryName> >("boundary");

  /**
   * Since MOOSE supports named boundary conditions we need to see if the vector contains
   * names (strings) or ids (unsigned ints)
   */
  for (unsigned int i=0; i<boundaries.size(); ++i)
  {
    BoundaryID boundary_id = _mesh.getBoundaryID(boundaries[i]);

    parameters.set<BoundaryID>("_boundary_id") = boundary_id;
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      parameters.set<THREAD_ID>("_tid") = tid;
      parameters.set<MaterialData *>("_material_data") = _fe_problem._bnd_material_data[tid];

      BoundaryCondition * bc = static_cast<BoundaryCondition *>(_factory.create(bc_name, name, parameters));
      mooseAssert(bc != NULL, "Not a BoundaryCondition object");
      _fe_problem._objects_by_name[tid][name].push_back(bc);

      if (dynamic_cast<PresetNodalBC*>(bc) != NULL)
      {
        PresetNodalBC * pnbc = dynamic_cast<PresetNodalBC*>(bc);
        _bcs[tid].addPresetNodalBC(boundary_id, pnbc);
      }

      if (dynamic_cast<NodalBC *>(bc) != NULL)
      {
        NodalBC * nbc = dynamic_cast<NodalBC *>(bc);
        _bcs[tid].addNodalBC(boundary_id, nbc);
        _vars[tid].addBoundaryVars(boundary_id, nbc->getCoupledVars());
      }
      else if (dynamic_cast<IntegratedBC *>(bc) != NULL)
      {
        IntegratedBC * ibc = dynamic_cast<IntegratedBC *>(bc);
        _bcs[tid].addBC(boundary_id, ibc);
        _vars[tid].addBoundaryVars(boundary_id, ibc->getCoupledVars());
      }
      else
        mooseError("Unknown type of BoudaryCondition object");

      _vars[tid].addBoundaryVar(boundary_id, &bc->variable());
    }
  }
}

void
NonlinearSystem::addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters)
{
  parameters.set<THREAD_ID>("_tid") = 0;

  MooseObject * obj = _factory.create(c_name, name, parameters);
  _fe_problem._objects_by_name[0][name].push_back(obj);

  NodalConstraint    * nc = dynamic_cast<NodalConstraint *>(obj);
  NodeFaceConstraint * nfc = dynamic_cast<NodeFaceConstraint *>(obj);
  if (nfc != NULL)
  {
    unsigned int slave = _mesh.getBoundaryID(parameters.get<BoundaryName>("slave"));
    unsigned int master = _mesh.getBoundaryID(parameters.get<BoundaryName>("master"));
    _constraints[0].addNodeFaceConstraint(slave, master, nfc);
  }
  else if (nc != NULL)
  {
    _constraints[0].addNodalConstraint(nc);
  }
  else
  {
    mooseError("Unknown type of Constraint object");
  }
  addImplicitGeometricCouplingEntriesToJacobian(true);
}

void
NonlinearSystem::addDiracKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _fe_problem._material_data[tid];

    DiracKernel *kernel = static_cast<DiracKernel *>(_factory.create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Dirac Kernel object");

    _dirac_kernels[tid].addDiracKernel(kernel);
    _fe_problem._objects_by_name[tid][name].push_back(kernel);
  }
}

void
NonlinearSystem::addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _fe_problem._bnd_material_data[tid];
    parameters.set<MaterialData *>("_neighbor_material_data") = _fe_problem._neighbor_material_data[tid];

    DGKernel *dg_kernel = static_cast<DGKernel *>(_factory.create(dg_kernel_name, name, parameters));
    mooseAssert(dg_kernel != NULL, "Not a DG Kernel object");

    _dg_kernels[tid].addDGKernel(dg_kernel);
    _fe_problem._objects_by_name[tid][name].push_back(dg_kernel);
  }

  _doing_dg = true;
}

void
NonlinearSystem::addDamper(const std::string & damper_name, const std::string & name, InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _fe_problem._material_data[tid];

    Damper * damper = static_cast<Damper *>(_factory.create(damper_name, name, parameters));
    _dampers[tid].addDamper(damper);
    _fe_problem._objects_by_name[tid][name].push_back(damper);
  }
}

void
NonlinearSystem::addSplit(const  std::string & split_name, const std::string & name, InputParameters parameters)
{
  Split *split = static_cast<Split *>(_factory.create(split_name, name, parameters));
  mooseAssert(split != NULL, "Not a Split object");
  _splits.addSplit(name, split);
}

Split*
NonlinearSystem::getSplit(const std::string & name)
{

  Split *split = _splits.getSplit(name);
  return split;
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
  Moose::perf_log.push("compute_residual()","Solve");

  _n_residual_evaluations++;

  Moose::enableFPE();

  for(std::vector<NumericVector<Number> *>::iterator it = _vecs_to_zero_for_residual.begin();
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
    _time_integrator->preStep();
    computeTimeDerivatives();
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
  }
  catch (MooseException & e)
  {
    residual.close();
    if (!_computing_initial_residual)
    {
      // tell solver to stop
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,0,0)
#else
      PetscNonlinearSolver<Real> & solver = static_cast<PetscNonlinearSolver<Real> &>(*_sys.nonlinear_solver);
      SNESSetFunctionDomainError(solver.snes());
#endif
#endif
    }
    throw;
  }

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_residual()","Solve");
}


void
NonlinearSystem::onTimestepBegin()
{
  _time_integrator->preSolve();
}

void
NonlinearSystem::setInitialSolution()
{
  NumericVector<Number> & initial_solution(solution());
  if (_predictor != NULL)
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

    if (node->processor_id() == libMesh::processor_id())
    {
      // reinit variables in nodes
      _fe_problem.reinitNodeFace(node, boundary_id, 0);

      std::vector<PresetNodalBC*> p(_bcs[0].activePresetNodal(boundary_id));
      for (std::vector<PresetNodalBC *>::iterator it = p.begin(); it != p.end(); ++it)
        (*it)->computeValue(initial_solution);
    }
  }

  _sys.solution->close();
  update();

  // Set constraint slave values
  setConstraintSlaveValues(initial_solution, false);

  if(_fe_problem.getDisplacedProblem())
    setConstraintSlaveValues(initial_solution, true);
}

void NonlinearSystem::setPredictor(Predictor * predictor)
{
  _predictor = predictor;
}

void
NonlinearSystem::subdomainSetup(unsigned int /*subdomain*/, THREAD_ID tid)
{
  //Global Kernels
  for(std::vector<Kernel *>::const_iterator kernel_it = _kernels[tid].active().begin(); kernel_it != _kernels[tid].active().end(); kernel_it++)
    (*kernel_it)->subdomainSetup();
}

NumericVector<Number> &
NonlinearSystem::solutionUDot()
{
  return _u_dot;
}

NumericVector<Number> &
NonlinearSystem::solutionDuDotDu()
{
  return _du_dot_du;
}

NumericVector<Number> &
NonlinearSystem::residualVector(Moose::KernelType type)
{
  switch (type)
  {
  case Moose::KT_TIME: return _Re_time;
  case Moose::KT_NONTIME: return _Re_non_time;
  default: mooseError("Trying to get residual vector that is not available");
  }
}

void
NonlinearSystem::computeTimeDerivatives()
{
  _time_integrator->computeTimeDerivatives();
}

void
NonlinearSystem::enforceNodalConstraintsResidual(NumericVector<Number> & residual)
{
  THREAD_ID tid = 0;                            // constraints are going to be done single-threaded
  // loop over nodes with nodal constraints
  std::vector<NodalConstraint *> & ncs = _constraints[0].getNodalConstraints();
  for (std::vector<NodalConstraint *>::iterator it = ncs.begin(); it != ncs.end(); ++it)
  {
    NodalConstraint * nc = (*it);

    Node & master_node = _mesh.node(nc->getMasterNodeId());
    // reinit variables at the master node
    _fe_problem.reinitNode(&master_node, tid);
    _fe_problem.prepareAssembly(tid);

    // go over slave nodes
    std::vector<dof_id_type> slave_nodes = nc->getSlaveNodeId();
    for (std::vector<dof_id_type>::iterator it = slave_nodes.begin(); it != slave_nodes.end(); ++it)
    {
      Node & slave_node = _mesh.node(*it);
      // reinit variables on the slave node
      _fe_problem.reinitNodeNeighbor(&slave_node, tid);
      // compute residual
      nc->computeResidual(residual);
    }
  }
}

void
NonlinearSystem::enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian)
{
  THREAD_ID tid = 0;                            // constraints are going to be done single-threaded
  // loop over nodes with nodal constraints
  std::vector<NodalConstraint *> & ncs = _constraints[0].getNodalConstraints();
  for (std::vector<NodalConstraint *>::iterator it = ncs.begin(); it != ncs.end(); ++it)
  {
    NodalConstraint * nc = (*it);

    Node & master_node = _mesh.node(nc->getMasterNodeId());
    // reinit variables at the master node
    _fe_problem.reinitNode(&master_node, tid);
    _fe_problem.prepareAssembly(tid);

    // go over slave nodes
    std::vector<dof_id_type> slave_nodes = nc->getSlaveNodeId();
    for (std::vector<dof_id_type>::iterator it = slave_nodes.begin(); it != slave_nodes.end(); ++it)
    {
      Node & slave_node = _mesh.node(*it);
      // reinit variables on the slave node
      _fe_problem.reinitNodeNeighbor(&slave_node, tid);
      // compute residual
      nc->computeJacobian(jacobian);
    }
  }
}

void
NonlinearSystem::setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced)
{
  std::map<std::pair<dof_id_type, dof_id_type>, PenetrationLocator *> * penetration_locators = NULL;

  if(!displaced)
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

  for(std::map<std::pair<dof_id_type, dof_id_type>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {
    PenetrationLocator & pen_loc = *it->second;

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    std::vector<NodeFaceConstraint *> constraints;

    if(!displaced)
      constraints = _constraints[0].getNodeFaceConstraints(slave_boundary);
    else
      constraints = _constraints[0].getDisplacedNodeFaceConstraints(slave_boundary);

    if(constraints.size())
    {
      for(dof_id_type i=0; i<slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if(slave_node.processor_id() == libMesh::processor_id())
        {
          if(pen_loc._penetration_info[slave_node_num])
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

            for(unsigned int c=0; c < constraints.size(); c++)
            {
              NodeFaceConstraint * nfc = constraints[c];

              if(nfc->shouldApply())
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
  Parallel::max(constraints_applied);

  if(constraints_applied)
  {
    solution.close();
    update();
  }
}

void
NonlinearSystem::constraintResiduals(NumericVector<Number> & residual, bool displaced)
{
  std::map<std::pair<dof_id_type, dof_id_type>, PenetrationLocator *> * penetration_locators = NULL;

  if(!displaced)
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
  for(std::map<std::pair<dof_id_type, dof_id_type>, PenetrationLocator *>::iterator it = penetration_locators->begin();
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

    std::vector<NodeFaceConstraint *> constraints;

    if(!displaced)
      constraints = _constraints[0].getNodeFaceConstraints(slave_boundary);
    else
      constraints = _constraints[0].getDisplacedNodeFaceConstraints(slave_boundary);

    if(constraints.size())
    {
      for(dof_id_type i=0; i<slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if(slave_node.processor_id() == libMesh::processor_id())
        {
          if(pen_loc._penetration_info[slave_node_num])
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

            for(unsigned int c=0; c < constraints.size(); c++)
            {
              NodeFaceConstraint * nfc = constraints[c];

              if(nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeResidual();

                if(nfc->overwriteSlaveResidual())
                  _fe_problem.setResidual(residual, 0);
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
      Parallel::max(constraints_applied);

      if(constraints_applied)
      {
	residual.close();
	_fe_problem.addCachedResidual(residual, 0);
	residual.close();
	if (_need_residual_ghosted) {
	  _residual_ghosted = residual;
	  _residual_ghosted.close();  // somewhat redundant since the assignment above should also close
	}
      }
    }
  }
  if (!_assemble_constraints_separately)
  {
    Parallel::max(constraints_applied);

    if(constraints_applied)
    {
      residual.close();
      _fe_problem.addCachedResidual(residual, 0);
      residual.close();
      if (_need_residual_ghosted)
      {
        _residual_ghosted = residual;
        _residual_ghosted.close();  // somewhat redundant since the assignment above should also close
      }
    }
  }
}


void
NonlinearSystem::computeResidualInternal(Moose::KernelType type)
{

   // residualSetup() /////
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _kernels[i].residualSetup();
    _bcs[i].residualSetup();
    _dirac_kernels[i].residualSetup();
    if (_doing_dg) _dg_kernels[i].residualSetup();
    _constraints[i].residualSetup();
  }

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  // residual contributions from the domain
  PARALLEL_TRY {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeResidualThread cr(_fe_problem, *this, type);
    Threads::parallel_reduce(elem_range, cr);
  }
  PARALLEL_CATCH;

  // residual contributions from the scalar kernels
  PARALLEL_TRY {
    // do scalar kernels (not sure how to thread this)
    const std::vector<ScalarKernel *> & scalars = _kernels[0].scalars();
    if (scalars.size() > 0)
    {
      for (std::vector<ScalarKernel *>::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
      {
        ScalarKernel * kernel = *it;

        kernel->reinit();
        kernel->computeResidual();
      }
      _fe_problem.addResidualScalar();
    }
  }
  PARALLEL_CATCH;

  if(_need_residual_copy)
  {
    Moose::perf_log.push("residual.close1()","Solve");
    residualVector(Moose::KT_NONTIME).close();
    Moose::perf_log.pop("residual.close1()","Solve");
    residualVector(Moose::KT_NONTIME).localize(_residual_copy);
  }

  if(_need_residual_ghosted)
  {
    Moose::perf_log.push("residual.close2()","Solve");
    residualVector(Moose::KT_NONTIME).close();
    Moose::perf_log.pop("residual.close2()","Solve");
    _residual_ghosted = residualVector(Moose::KT_NONTIME);
    _residual_ghosted.close();
  }

  PARALLEL_TRY {
    computeDiracContributions();
  }
  PARALLEL_CATCH;

  if(_fe_problem._has_constraints)
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
      if(_fe_problem.getDisplacedProblem())
        constraintResiduals(residualVector(Moose::KT_NONTIME), true);
    }
    PARALLEL_CATCH;
    residualVector(Moose::KT_NONTIME).close();
  }
}

void
NonlinearSystem::computeNodalBCs(NumericVector<Number> & residual)
{
  PARALLEL_TRY {
    // last thing to do are nodal BCs
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if(node->processor_id() == libMesh::processor_id())
      {
        // reinit variables in nodes
        _fe_problem.reinitNodeFace(node, boundary_id, 0);

        std::vector<NodalBC *> bcs = _bcs[0].activeNodal(boundary_id);
        for (std::vector<NodalBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
        {
          NodalBC * bc = *it;
          if (bc->shouldApply())
            bc->computeResidual(residual);
        }
      }
    }
  }
  PARALLEL_CATCH;

  Moose::perf_log.push("residual.close4()","Solve");
  residual.close();
  residualVector(Moose::KT_TIME).close();
  residualVector(Moose::KT_NONTIME).close();
  Moose::perf_log.pop("residual.close4()","Solve");
}


void
NonlinearSystem::findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data, std::map<unsigned int, std::vector<unsigned int> > & graph)
{
  std::map<std::pair<dof_id_type, dof_id_type>, NearestNodeLocator *> & nearest_node_locators = geom_search_data._nearest_node_locators;

  for(std::map<std::pair<dof_id_type, dof_id_type>, NearestNodeLocator *>::iterator it = nearest_node_locators.begin();
      it != nearest_node_locators.end();
      ++it)
  {
    std::vector<dof_id_type> & slave_nodes = it->second->_slave_nodes;

    for(dof_id_type i=0; i<slave_nodes.size(); i++)
    {
      std::set<dof_id_type> unique_slave_indices;
      std::set<dof_id_type> unique_master_indices;

      dof_id_type slave_node = slave_nodes[i];

      {
        std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[slave_node];

        // Get the dof indices from each elem connected to the node
        for(dof_id_type el=0; el < elems.size(); ++el)
        {
          dof_id_type cur_elem = elems[el];

          std::vector<dof_id_type> dof_indices;
          dofMap().dof_indices(_mesh.elem(cur_elem), dof_indices);

          for(dof_id_type di=0; di < dof_indices.size(); di++)
            unique_slave_indices.insert(dof_indices[di]);
        }
      }

      std::vector<dof_id_type> master_nodes = it->second->_neighbor_nodes[slave_node];

      for(dof_id_type k=0; k<master_nodes.size(); k++)
      {
        dof_id_type master_node = master_nodes[k];

        {
          std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[master_node];

          // Get the dof indices from each elem connected to the node
          for(dof_id_type el=0; el < elems.size(); ++el)
          {
            dof_id_type cur_elem = elems[el];

            std::vector<dof_id_type> dof_indices;
            dofMap().dof_indices(_mesh.elem(cur_elem), dof_indices);

            for(dof_id_type di=0; di < dof_indices.size(); di++)
              unique_master_indices.insert(dof_indices[di]);
          }
        }
      }

      for(std::set<dof_id_type>::iterator sit=unique_slave_indices.begin(); sit != unique_slave_indices.end(); ++sit)
      {
        dof_id_type slave_id = *sit;

        for(std::set<dof_id_type>::iterator mit=unique_master_indices.begin(); mit != unique_master_indices.end(); ++mit)
        {
          dof_id_type master_id = *mit;

          graph[slave_id].push_back(master_id);
          graph[master_id].push_back(slave_id);
        }
      }
    }
  }

  // Make every entry sorted and unique
  for(std::map<dof_id_type, std::vector<dof_id_type> >::iterator git=graph.begin(); git != graph.end(); ++git)
  {
    std::vector<unsigned int> & row = git->second;

    std::sort(row.begin(), row.end());
    std::vector<unsigned int>::iterator uit = std::unique(row.begin(), row.end());
    row.resize(uit - row.begin());
  }
}



void
NonlinearSystem::addImplicitGeometricCouplingEntries(SparseMatrix<Number> & jacobian, GeometricSearchData & geom_search_data)
{
  std::map<dof_id_type, std::vector<dof_id_type> > graph;

  findImplicitGeometricCouplingEntries(geom_search_data, graph);

  for(std::map<dof_id_type, std::vector<dof_id_type> >::iterator git=graph.begin(); git != graph.end(); ++git)
  {
    dof_id_type dof = git->first;
    std::vector<unsigned int> & row = git->second;

    for(unsigned int i=0; i<row.size(); i++)
    {
      dof_id_type coupled_dof = row[i];
//          if(slave_dof_indices[l] >= _sys.get_dof_map().first_dof() && slave_dof_indices[l] < _sys.get_dof_map().end_dof())
      jacobian.add(dof, coupled_dof, 0);
    }
  }
}

void
NonlinearSystem::constraintJacobians(SparseMatrix<Number> & jacobian, bool displaced)
{
  std::vector<numeric_index_type> zero_rows;
  std::map<std::pair<dof_id_type, dof_id_type>, PenetrationLocator *> * penetration_locators = NULL;

  if(!displaced)
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
  for(std::map<std::pair<dof_id_type, dof_id_type>, PenetrationLocator *>::iterator it = penetration_locators->begin();
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

    std::vector<NodeFaceConstraint *> constraints;

    if(!displaced)
      constraints = _constraints[0].getNodeFaceConstraints(slave_boundary);
    else
      constraints = _constraints[0].getDisplacedNodeFaceConstraints(slave_boundary);

    zero_rows.clear();
    if(constraints.size())
    {
      for(dof_id_type i=0; i<slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if(slave_node.processor_id() == libMesh::processor_id())
        {
          if(pen_loc._penetration_info[slave_node_num])
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
            for(unsigned int c=0; c < constraints.size(); c++)
            {
              NodeFaceConstraint * nfc = constraints[c];

              nfc->_jacobian = &jacobian;

              if(nfc->shouldApply())
              {
                constraints_applied = true;

                nfc->subProblem().prepareShapes(nfc->variable().index(), 0);
                nfc->subProblem().prepareNeighborShapes(nfc->variable().index(), 0);

                nfc->computeJacobian();

                if(nfc->overwriteSlaveJacobian())
                {
                  // Add this variable's dof's row to be zeroed
                  zero_rows.push_back(nfc->variable().nodalDofIndex());
                }

                std::vector<dof_id_type> slave_dofs(1,nfc->variable().nodalDofIndex());

                // Cache the jacobian block for the slave side
                _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kee, slave_dofs, nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                // Cache the jacobian block for the master side
                _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kne, nfc->variable().dofIndicesNeighbor(), nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                _fe_problem.cacheJacobian(0);
                _fe_problem.cacheJacobianNeighbor(0);
              }
            }
          }
        }
      }
    }
    if (_assemble_constraints_separately)
    {
      // See if constraints were applied anywhere
      Parallel::max(constraints_applied);

      if(constraints_applied)
      {
#ifdef LIBMESH_HAVE_PETSC
        //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
        MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
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
  if(!_assemble_constraints_separately)
  {
    // See if constraints were applied anywhere
    Parallel::max(constraints_applied);

    if (constraints_applied)
    {
#ifdef LIBMESH_HAVE_PETSC
      //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
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


void
NonlinearSystem::computeScalarKernelsJacobians(SparseMatrix<Number> & jacobian)
{
  // Compute the diagonal block for scalar variables
  THREAD_ID tid = 0;
  const std::vector<MooseVariableScalar *> & scalar_vars = getScalarVariables(tid);
  if (scalar_vars.size() > 0)
  {
    const std::vector<ScalarKernel *> & scalars = _kernels[tid].scalars();

    _fe_problem.reinitScalars(tid);
    for (std::vector<ScalarKernel *>::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
    {
      ScalarKernel * kernel = *it;

      kernel->reinit();
      kernel->computeJacobian();
      _fe_problem.addJacobianOffDiagScalar(jacobian, kernel->variable().index());
    }
    _fe_problem.addJacobianScalar(jacobian);
  }
}

void
NonlinearSystem::computeJacobianInternal(SparseMatrix<Number> &  jacobian)
{
  _currently_computing_jacobian = true;

#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
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
  // PETSc 3.3.0
  MatSetOption(static_cast<PetscMatrix<Number> &>(*sys().matrix).mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
#endif

#endif

  // jacobianSetup /////
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _kernels[i].jacobianSetup();
    _bcs[i].jacobianSetup();
    _dirac_kernels[i].jacobianSetup();
    _constraints[i].jacobianSetup();
    if (_doing_dg) _dg_kernels[i].jacobianSetup();
  }

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
      }
      break;

    default:
    case Moose::COUPLING_CUSTOM:
      {
        ComputeFullJacobianThread cj(_fe_problem, *this, jacobian);
        Threads::parallel_reduce(elem_range, cj);
      }
      break;
    }

    computeDiracContributions(&jacobian);
    computeScalarKernelsJacobians(jacobian);

    static bool first = true;

    // This adds zeroes into geometric coupling entries to ensure they stay in the matrix
    if(first && (_add_implicit_geometric_coupling_entries_to_jacobian))
    {
      first = false;
      addImplicitGeometricCouplingEntries(jacobian, _fe_problem.geomSearchData());

      if(_fe_problem.getDisplacedProblem())
        addImplicitGeometricCouplingEntries(jacobian, _fe_problem.getDisplacedProblem()->geomSearchData());
    }
  }
  PARALLEL_CATCH;

  jacobian.close();

  PARALLEL_TRY {
    // do nodal BC
    std::vector<numeric_index_type> zero_rows;

    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if(node->processor_id() == libMesh::processor_id())
      {
        _fe_problem.reinitNodeFace(node, boundary_id, 0);

        for (std::vector<NodalBC *>::iterator it = _bcs[0].getNodalBCs(boundary_id).begin(); it != _bcs[0].getNodalBCs(boundary_id).end(); ++it)
        {
          NodalBC * bc = *it;
          if (bc->shouldApply() && bc->variable().isNodalDefined())
            zero_rows.push_back(bc->variable().nodalDofIndex());
        }
      }
    }

    jacobian.zero_rows(zero_rows, 1.0);
  }
  PARALLEL_CATCH;
  jacobian.close();

  PARALLEL_TRY {
    // Add in Jacobian contributions from Constraints
    if(_fe_problem._has_constraints)
    {
      // Nodal Constraints
      enforceNodalConstraintsJacobian(jacobian);

      // Undisplaced Constraints
      constraintJacobians(jacobian, false);

      // Displaced Constraints
      if(_fe_problem.getDisplacedProblem())
        constraintJacobians(jacobian, true);
    }
  }
  PARALLEL_CATCH;

  _currently_computing_jacobian = false;
}

void
NonlinearSystem::computeJacobian(SparseMatrix<Number> & jacobian)
{
  Moose::perf_log.push("compute_jacobian()","Solve");

  Moose::enableFPE();

  try {
    jacobian.zero();
    computeJacobianInternal(jacobian);
  }
  catch (MooseException & e)
  {
    jacobian.close();
    // tell solver to stop
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,0,0)
#else
    PetscNonlinearSolver<Real> & solver = static_cast<PetscNonlinearSolver<Real> &>(*_sys.nonlinear_solver);
    SNESSetFunctionDomainError(solver.snes());
#endif
#endif
    throw;
  }

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_jacobian()","Solve");
}

void
NonlinearSystem::computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar)
{
  Moose::perf_log.push("compute_jacobian_block()","Solve");

  Moose::enableFPE();

#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
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
  // PETSc 3.3.0
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
#endif

#endif

  jacobian.zero();

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeJacobianBlockThread cjb(_fe_problem, precond_system, jacobian, ivar, jvar);
    Threads::parallel_reduce(elem_range, cjb);
  }
  PARALLEL_CATCH;
  jacobian.close();

  //Dirichlet BCs
  std::vector<numeric_index_type> zero_rows;
  PARALLEL_TRY {
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      std::vector<NodalBC *> bcs = _bcs[0].activeNodal(boundary_id);
      if (bcs.size() > 0)
      {
        if (node->processor_id() == libMesh::processor_id())
        {
          _fe_problem.reinitNodeFace(node, boundary_id, 0);

          for (std::vector<NodalBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
          {
            NodalBC * bc = *it;
            if (bc->variable().index() == ivar && bc->shouldApply())
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
  PARALLEL_CATCH;

  jacobian.close();

  //This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
  if(ivar == jvar)
    jacobian.zero_rows(zero_rows, 1.0);
  else
    jacobian.zero_rows(zero_rows, 0.0);

  jacobian.close();

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_jacobian_block()","Solve");
}

Real
NonlinearSystem::computeDamping(const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()","Solve");

  // Default to no damping
  Real damping = 1.0;

  if(_dampers[0].all().size() > 0)
  {
    *_increment_vec = update;
    ComputeDampingThread cid(_fe_problem, *this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cid);
    damping = cid.damping();
  }

  Parallel::min(damping);

  Moose::perf_log.pop("compute_dampers()","Solve");

  return damping;
}

void
NonlinearSystem::computeDiracContributions(SparseMatrix<Number> * jacobian)
{
  Moose::perf_log.push("computeDiracContributions()","Solve");

  _fe_problem.clearDiracInfo();

  std::set<const Elem *> dirac_elements;

  // TODO: Need a threading fix... but it's complicated!
  for (std::vector<DiracKernel *>::const_iterator dirac_kernel_it = _dirac_kernels[0].all().begin();
      dirac_kernel_it != _dirac_kernels[0].all().end();
      ++dirac_kernel_it)
  {
    (*dirac_kernel_it)->clearPoints();
    (*dirac_kernel_it)->addPoints();
  }

  if (_dirac_kernels[0].all().size() > 0)
  {
    ComputeDiracThread cd(_fe_problem, *this, jacobian);

    _fe_problem.getDiracElements(dirac_elements);

    DistElemRange range(dirac_elements.begin(),
                        dirac_elements.end(),
                        1);
    // TODO: Make Dirac work thread!
    //Threads::parallel_reduce(range, cd);

    cd(range);
  }

  Moose::perf_log.pop("computeDiracContributions()","Solve");

  if (jacobian == NULL)
  {
    Moose::perf_log.push("residual.close3()","Solve");
    residualVector(Moose::KT_NONTIME).close();
    Moose::perf_log.pop("residual.close3()","Solve");
  }
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
                                 std::vector<unsigned int> & n_nz,
                                 std::vector<unsigned int> & n_oz)
{

  if(_add_implicit_geometric_coupling_entries_to_jacobian)
  {
    _fe_problem.updateGeomSearch();

    std::map<dof_id_type, std::vector<dof_id_type> > graph;

    findImplicitGeometricCouplingEntries(_fe_problem.geomSearchData(), graph);

    if(_fe_problem.getDisplacedProblem())
      findImplicitGeometricCouplingEntries(_fe_problem.getDisplacedProblem()->geomSearchData(), graph);

    const dof_id_type first_dof_on_proc = dofMap().first_dof(libMesh::processor_id());
    const dof_id_type end_dof_on_proc   = dofMap().end_dof(libMesh::processor_id());

    for(std::map<dof_id_type, std::vector<dof_id_type> >::iterator git=graph.begin(); git != graph.end(); ++git)
    {
      dof_id_type dof = git->first;
      dof_id_type local_dof = dof - first_dof_on_proc;

      if(dof < first_dof_on_proc || dof >= end_dof_on_proc)
        continue;

      std::vector<unsigned int> & row = git->second;

      SparsityPattern::Row & sparsity_row = sparsity[local_dof];

      unsigned int original_row_length = sparsity_row.size();

      sparsity_row.insert(sparsity_row.end(), row.begin(), row.end());

      SparsityPattern::sort_row(sparsity_row.begin(), sparsity_row.begin()+original_row_length, sparsity_row.end());

      // Fix up nonzero arrays
      for(unsigned int i=0; i<row.size(); i++)
      {
        dof_id_type coupled_dof = row[i];

        if(coupled_dof < first_dof_on_proc || coupled_dof >= end_dof_on_proc)
          n_oz[local_dof]++;
        else
          n_nz[local_dof]++;
      }
    }
  }
}

void
NonlinearSystem::serializeSolution()
{
  if(_need_serialized_solution)
    _current_solution->localize(_serialized_solution);
 }

void
NonlinearSystem::setSolution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  if(_need_serialized_solution)
    serializeSolution();
}

void
NonlinearSystem::setSolutionUDot(const NumericVector<Number> & udot)
{
  _u_dot = udot;
}

void
NonlinearSystem::setSolutionDuDotDu(Real value)
{
  _du_dot_du = value;
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
NonlinearSystem::printVarNorms()
{
  TransientNonlinearImplicitSystem &s = static_cast<TransientNonlinearImplicitSystem &>(_sys);

  std::map<std::string, Real> other_var_norms;
  std::map<std::string, Real> outlier_var_norms;

  unsigned int n_vars = _sys.n_vars();

  Real average_norm = (_last_nl_rnorm*_last_nl_rnorm) / n_vars;

  for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
  {
    Real var_norm = s.calculate_norm(*s.rhs,var_num,DISCRETE_L2);
    var_norm *= var_norm; // use the norm squared

    if(var_norm > 2 * average_norm)
      outlier_var_norms[s.variable_name(var_num)] = var_norm;
    else
      other_var_norms[s.variable_name(var_num)] = var_norm;
  }

  if(outlier_var_norms.size())
  {
    Moose::out << "Outlier Variable Residual Norms:\n";
    for(std::map<std::string, Real>::iterator it = outlier_var_norms.begin();
        it != outlier_var_norms.end();
        ++it)
    {
      std::string color(YELLOW);

      if(it->second > 0.8 * _last_nl_rnorm*_last_nl_rnorm)
        color = RED;

      Moose::out << " " << it->first << ": " <<
        _fe_problem.colorText(color, std::sqrt(it->second)) << '\n';
    }
  }

  if(_print_all_var_norms)
  {
    Moose::out << "Variable Residual Norms:\n";
    for(std::map<std::string, Real>::iterator it = other_var_norms.begin();
        it != other_var_norms.end();
        ++it)
      Moose::out << " " << it->first << ": " <<
        _fe_problem.colorText(GREEN, std::sqrt(it->second)) << '\n';
  }

  Moose::out.flush();
}

void
NonlinearSystem::setPreconditioner(MoosePreconditioner *pc)
{
  if (_preconditioner != NULL)
    mooseError("More than one active Preconditioner detected");

  _preconditioner = pc;
}

void
NonlinearSystem::setupDampers()
{
  // Set the callback for user_final_solver_defaults in libMesh
#ifdef LIBMESH_HAVE_PETSC
  _sys.nonlinear_solver->user_presolve = Moose::PetscSupport::petscSetupDampers;
#endif

  _increment_vec = &_sys.add_vector("u_increment", true, GHOSTED);
}

void
NonlinearSystem::reinitDampers(THREAD_ID tid)
{
  // FIXME: be smart here and compute only variables with dampers (need to add some book keeping)
  const std::vector<MooseVariable *> & vars = _vars[tid].variables();
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeDamping(*_increment_vec);
  }
}

void
NonlinearSystem::checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains, bool check_kernel_coverage) const
{
  // Check kernel coverage of subdomains (blocks) in your mesh
  std::set<SubdomainID> input_subdomains;
  std::set<std::string> kernel_variables;

  bool global_kernels_exist = _kernels[0].subdomainsCovered(input_subdomains, kernel_variables);
  if (!global_kernels_exist && check_kernel_coverage)
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

void
NonlinearSystem::checkBCCoverage() const
{
  // Check that BCs used in your simulation exist in your mesh
  std::set<short> input_bcs, difference;

  // get the boundaries from the simulation (input file)
  _bcs[0].activeBoundaries(input_bcs);

  // _mesh is from SystemBase...
  std::set_difference (input_bcs.begin(), input_bcs.end(),
                       _mesh.meshBoundaryIds().begin(), _mesh.meshBoundaryIds().end(),
                       std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::stringstream extra_boundary_ids;

    std::copy (difference.begin(), difference.end(), std::ostream_iterator<unsigned short>( extra_boundary_ids, " "));

    mooseError("The following boundary ids from your input file do not exist in the input mesh "
               + extra_boundary_ids.str());
  }
}



bool
NonlinearSystem::containsTimeKernel()
{
  bool time_kernels = false;
  for (std::vector<Kernel *>::const_iterator it = _kernels[0].all().begin(); it != _kernels[0].all().end(); ++it)
    if (dynamic_cast<TimeKernel *>(*it) != NULL)
      time_kernels = true;

  return time_kernels;
}

// DEBUGGING --------------------------------------------------------------------------------------

struct st
{
  unsigned int _var;
  unsigned int _nd;
  Real _residual;

  st() { _var = 0; _nd = 0; _residual = 0.; }

  st(unsigned int var, unsigned int nd, Real residual)
  {
    _var = var;
    _nd = nd;
    _residual = residual;
  }
};

struct st_sort_res
{
  bool operator() (st i, st j) { return (fabs(i._residual) > fabs(j._residual)); }
} dbg_sort_residuals;

void
NonlinearSystem::printTopResiduals(const NumericVector<Number> & residual, unsigned int n)
{
  std::vector<st> vec;
  vec.resize(residual.local_size());

  unsigned int j = 0;
  MeshBase::node_iterator it = _mesh.getMesh().local_nodes_begin();
  const MeshBase::node_iterator end = _mesh.getMesh().local_nodes_end();
  for (; it != end; ++it)
  {
    Node & node = *(*it);
    dof_id_type nd = node.id();

    for (unsigned int var = 0; var < node.n_vars(_sys.number()); ++var)
    {
      if (node.n_dofs(_sys.number(), var) > 0)
      {
        dof_id_type dof_idx = node.dof_number(_sys.number(), var, 0);
        vec[j] = st(var, nd, residual(dof_idx));
        j++;
      }
    }
  }
  // sort vec by residuals
  std::sort(vec.begin(), vec.end(), dbg_sort_residuals);
  // print out
  Moose::err << "[DBG][" << libMesh::processor_id() << "] Max " << n << " residuals";
  if (j < n)
  {
    n = j;
    Moose::err << " (Only " << n << " available)";
  }
  Moose::err << std::endl;

  for (unsigned int i = 0; i < n; ++i)
  {
    fprintf(stderr, "[DBG][%d]  % .15e '%s' at node %d\n", libMesh::processor_id(), vec[i]._residual, _sys.variable_name(vec[i]._var).c_str(), vec[i]._nd);
  }
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
