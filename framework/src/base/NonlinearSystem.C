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
#include "MProblem.h"
#include "MooseVariable.h"
#include "PetscSupport.h"
#include "Factory.h"
#include "ParallelUniqueId.h"
#include "ThreadedElementLoop.h"
#include "MaterialData.h"
#include "ComputeResidualThread.h"
#include "ComputeJacobianThread.h"
#include "ComputeFullJacobianThread.h"
#include "ComputeDiracThread.h"
#include "ComputeDampingThread.h"
#include "TimeKernel.h"
#include "FP.h"
#include "DisplacedProblem.h"
#include "NearestNodeLocator.h"

// libMesh
#include "nonlinear_solver.h"
#include "quadrature_gauss.h"
#include "dense_vector.h"
#include "boundary_info.h"
#include "petsc_matrix.h"
#include "numeric_vector.h"
#include "mesh.h"
#include "dense_subvector.h"
#include "dense_submatrix.h"
#include "dof_map.h"


namespace Moose {

  void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, NonlinearImplicitSystem& sys)
  {
    Problem * p = sys.get_equation_systems().parameters.get<Problem *>("_problem");
    p->computeJacobian(sys, soln, jacobian);
  }

  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual, NonlinearImplicitSystem& sys)
  {
    Problem * p = sys.get_equation_systems().parameters.get<Problem *>("_problem");
    p->computeResidual(sys, soln, residual);
  }

} // namespace Moose

NonlinearSystem::NonlinearSystem(MProblem & subproblem, const std::string & name) :
    SystemTempl<TransientNonlinearImplicitSystem>(subproblem, name),
    _mproblem(subproblem),
    _last_rnorm(0),
    _l_abs_step_tol(1e-10),
    _initial_residual(0),
    _coupling(Moose::COUPLING_DIAG),
    _cm(NULL),
    _current_solution(NULL),
    _solution_u_dot(_sys.add_vector("u_dot", false, GHOSTED)),
    _solution_du_dot_du(_sys.add_vector("du_dot_du", false, GHOSTED)),
    _residual_old(NULL),
    _residual_ghosted(_sys.add_vector("residual_ghosted", false, GHOSTED)),
    _serialized_solution(*NumericVector<Number>::build().release()),
    _residual_copy(*NumericVector<Number>::build().release()),
    _t(subproblem.time()),
    _dt(subproblem.dt()),
    _dt_old(subproblem.dtOld()),
    _t_step(subproblem.timeStep()),
    _time_weight(subproblem.timeWeights()),
    _increment_vec(NULL),
    _preconditioner(NULL),
    _use_finite_differenced_preconditioner(false),
    _add_implicit_geometric_coupling_entries_to_jacobian(false),
    _need_serialized_solution(false),
    _need_residual_copy(false),
    _need_residual_ghosted(false),
    _doing_dg(false),
    _n_iters(0),
    _final_residual(0.)
{
  _sys.nonlinear_solver->residual = Moose::compute_residual;
  _sys.nonlinear_solver->jacobian = Moose::compute_jacobian;

  _sys.attach_init_function(Moose::initial_condition);

  _time_weight.resize(3);
  timeSteppingScheme(Moose::IMPLICIT_EULER);                   // default time stepping scheme

  unsigned int n_threads = libMesh::n_threads();
  _asm_block.resize(n_threads);
  _kernels.resize(n_threads);
  _bcs.resize(n_threads);
  _dirac_kernels.resize(n_threads);
  _dg_kernels.resize(n_threads);
  _dampers.resize(n_threads);

  for (THREAD_ID tid = 0; tid < n_threads; ++tid)
    _asm_block[tid] = new AsmBlock(*this, couplingMatrix(), tid);
}

NonlinearSystem::~NonlinearSystem()
{
  delete _cm;

  delete _preconditioner;
  delete &_serialized_solution;
  delete &_residual_copy;

  unsigned int n_threads = libMesh::n_threads();
  for (THREAD_ID tid = 0; tid < n_threads; ++tid)
    delete _asm_block[tid];
}

void
NonlinearSystem::setCoupling(Moose::CouplingType type)
{
  _coupling = type;
}

void NonlinearSystem::setCouplingMatrix(CouplingMatrix * cm)
{
  _coupling = Moose::COUPLING_CUSTOM;
  delete _cm;
  _cm = cm;
}

void
NonlinearSystem::preInit()
{
  unsigned int n_vars = _sys.n_vars();
  switch (_coupling)
  {
  case Moose::COUPLING_DIAG:
    _cm = new CouplingMatrix(n_vars);
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*_cm)(i, j) = (i == j ? 1 : 0);
    break;

  // for full jacobian
  case Moose::COUPLING_FULL:
    _cm = new CouplingMatrix(n_vars);
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*_cm)(i, j) = 1;
    break;

  case Moose::COUPLING_CUSTOM:
    // do nothing, _cm was already set through couplingMatrix() call
    break;
  }

  _sys.get_dof_map()._dof_coupling = _cm;
}

void
NonlinearSystem::init()
{
  dofMap().attach_extra_send_list_function(&extraSendList, this);
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

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _asm_block[tid]->init();
}

void
NonlinearSystem::prepareAssembly(THREAD_ID tid)
{
  _asm_block[tid]->prepare();
}

void
NonlinearSystem::prepareAssemblyNeighbor(THREAD_ID tid)
{
  _asm_block[tid]->prepareNeighbor();
}

void
NonlinearSystem::prepareAssembly(unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _asm_block[tid]->prepareBlock(ivar, jvar, dof_indices);
}

void
NonlinearSystem::prepareAssemblyNeighbor(unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _asm_block[tid]->prepareBlock(ivar, jvar, dof_indices);
}

void
NonlinearSystem::addResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _asm_block[tid]->addResidual(residual);
}

void
NonlinearSystem::addResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _asm_block[tid]->addResidualNeighbor(residual);
}

void
NonlinearSystem::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _asm_block[tid]->addJacobian(jacobian);
}

void
NonlinearSystem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _asm_block[tid]->addJacobianNeighbor(jacobian);
}

void
NonlinearSystem::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  _asm_block[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
}

void
NonlinearSystem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid)
{
  _asm_block[tid]->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
}

void
NonlinearSystem::solve()
{
  // Initialize the solution vector with known values from nodal bcs
  setInitialSolution();

  if(_use_finite_differenced_preconditioner)
    setupFiniteDifferencedPreconditioner();

  _sys.solve();
  // store info about the solve
  _n_iters = _sys.n_nonlinear_iterations();
  _final_residual = _sys.final_nonlinear_residual();
}

void
NonlinearSystem::initialSetup()
{
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
    if (_doing_dg) _dg_kernels[i].timestepSetup();
  }
}

void
NonlinearSystem::setupFiniteDifferencedPreconditioner()
{
  // Make sure that libMesh isn't going to override our preconditioner
  _sys.nonlinear_solver->jacobian = NULL;

  PetscNonlinearSolver<Number> & petsc_nonlinear_solver =
    dynamic_cast<PetscNonlinearSolver<Number>&>(*_sys.nonlinear_solver);

  // Pointer to underlying PetscMatrix type
  PetscMatrix<Number>* petsc_mat =
    dynamic_cast<PetscMatrix<Number>*>(_sys.matrix);

  Moose::compute_jacobian(*_sys.current_local_solution,
                          *petsc_mat,
                          _sys);

  petsc_mat->close();

  if (!petsc_mat)
  {
    std::cerr << "Could not convert to Petsc matrix." << std::endl;
    libmesh_error();
  }

  PetscErrorCode ierr=0;
  ISColoring iscoloring;
  MatFDColoring fdcoloring;

  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORING_LF, &iscoloring);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  MatFDColoringCreate(petsc_mat->mat(),iscoloring,&fdcoloring);
  ISColoringDestroy(iscoloring);
  MatFDColoringSetFromOptions(fdcoloring);
  MatFDColoringSetFunction(fdcoloring,
                           (PetscErrorCode (*)(void))&libMesh::__libmesh_petsc_snes_residual,
                           &petsc_nonlinear_solver);
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESDefaultComputeJacobianColor,
                  fdcoloring);
}


bool
NonlinearSystem::converged()
{
  return _sys.nonlinear_solver->converged;
}

void
NonlinearSystem::addKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _mproblem._material_data[tid];

    Kernel *kernel = static_cast<Kernel *>(Factory::instance()->create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Kernel object");

    std::set<subdomain_id_type> blk_ids;
    if (!parameters.isParamValid("block"))
      blk_ids = _var_map[kernel->variable().number()];
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
      for (unsigned int i=0; i<blocks.size(); ++i)
      {
        if (_var_map[kernel->variable().number()].count(blocks[i]) > 0 || _var_map[kernel->variable().number()].size() == 0)
          blk_ids.insert(blocks[i]);
        else
          mooseError("Kernel (" + kernel->name() + "): block outside of the domain of the variable");
      }
    }
    _kernels[tid].addKernel(kernel, blk_ids);
  }
}

void
NonlinearSystem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

  for (unsigned int i=0; i<boundaries.size(); ++i)
  {
    parameters.set<unsigned int>("_boundary_id") = boundaries[i];
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      parameters.set<THREAD_ID>("_tid") = tid;
      parameters.set<MaterialData *>("_material_data") = _mproblem._bnd_material_data[tid];

      BoundaryCondition * bc = static_cast<BoundaryCondition *>(Factory::instance()->create(bc_name, name, parameters));
      mooseAssert(bc != NULL, "Not a BoundaryCondition object");

      if (dynamic_cast<PresetNodalBC*>(bc) != NULL)
      {
        PresetNodalBC * pnbc = dynamic_cast<PresetNodalBC*>(bc);
        _bcs[tid].addPresetNodalBC(boundaries[i], pnbc);
      }

      if (dynamic_cast<NodalBC *>(bc) != NULL)
      {
        NodalBC * nbc = dynamic_cast<NodalBC *>(bc);
        _bcs[tid].addNodalBC(boundaries[i], nbc);
        _vars[tid].addBoundaryVars(boundaries[i], nbc->getCoupledVars());
      }
      else if (dynamic_cast<IntegratedBC *>(bc) != NULL)
      {
        IntegratedBC * ibc = dynamic_cast<IntegratedBC *>(bc);
        _bcs[tid].addBC(boundaries[i], ibc);
        _vars[tid].addBoundaryVars(boundaries[i], ibc->getCoupledVars());
      }
      else
        mooseError("Unknown type of BoudaryCondition object");

      _vars[tid].addBoundaryVar(boundaries[i], &bc->variable());
    }
  }
}

void
NonlinearSystem::addDiracKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _mproblem._material_data[tid];

    DiracKernel *kernel = static_cast<DiracKernel *>(Factory::instance()->create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Dirac Kernel object");

    _dirac_kernels[tid].addDiracKernel(kernel);
  }
}

void
NonlinearSystem::addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _mproblem._bnd_material_data[tid];
    parameters.set<MaterialData *>("_neighbor_material_data") = _mproblem._neighbor_material_data[tid];

    DGKernel *dg_kernel = static_cast<DGKernel *>(Factory::instance()->create(dg_kernel_name, name, parameters));
    mooseAssert(dg_kernel != NULL, "Not a DG Kernel object");

    _dg_kernels[tid].addDGKernel(dg_kernel);
  }

  _doing_dg = true;
}

void
NonlinearSystem::addDamper(const std::string & damper_name, const std::string & name, InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _mproblem._material_data[tid];

    Damper * damper = static_cast<Damper *>(Factory::instance()->create(damper_name, name, parameters));
    _dampers[tid].addDamper(damper);
  }
}

void
NonlinearSystem::addVector(const std::string & vector_name, const bool project, const ParallelType type, bool zero_for_residual)
{
  NumericVector<Number> * vec = &_sys.add_vector(vector_name, project, type);

  if(zero_for_residual)
    _vecs_to_zero_for_residual.push_back(vec);
}


void
NonlinearSystem::computeResidual(NumericVector<Number> & residual)
{
  Moose::perf_log.push("compute_residual()","Solve");

  Moose::enableFPE();

  for(std::vector<NumericVector<Number> *>::iterator it = _vecs_to_zero_for_residual.begin();
      it != _vecs_to_zero_for_residual.end();
      ++it)
    (*it)->zero();

  computeTimeDeriv();
  computeResidualInternal(residual);
  finishResidual(residual);

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_residual()","Solve");
}

void
NonlinearSystem::timeSteppingScheme(Moose::TimeSteppingScheme scheme)
{
  _time_stepping_scheme = scheme;
  switch (_time_stepping_scheme)
  {
  case Moose::IMPLICIT_EULER:
    _time_weight[0] = 1;
    _time_weight[1] = 0;
    _time_weight[2] = 0;
    _time_stepping_order = 1;
    break;

  case Moose::CRANK_NICOLSON:
    _residual_old = &_sys.add_vector("residual_old", true, GHOSTED);

    _time_weight[0] = 1;
    _time_weight[1] = 0;
    _time_weight[2] = 0;
    _time_stepping_order = 2;
    break;

  case Moose::BDF2:
    _time_weight[0] = 0;
    _time_weight[1] = -1.;
    _time_weight[2] = 1.;
    _time_stepping_order = 2;
    break;
  }
}

void
NonlinearSystem::onTimestepBegin()
{
  Real sum;

  switch (_time_stepping_scheme)
  {
    case Moose::CRANK_NICOLSON:
      _solution_u_dot = solutionOld();
      _solution_u_dot *= -2.0 / _dt;
      _solution_u_dot.close();

      _solution_du_dot_du.zero();
      _solution_du_dot_du.close();

      {
        const NumericVector<Real> * current_solution = currentSolution();
        set_solution(solutionOld());                    // use old_solution for computing with correct solution vector
        computeResidualInternal(*_residual_old);
        set_solution(*current_solution);                    // reset the solution vector
      }
    break;

  case Moose::BDF2:
    sum = _dt+_dt_old;
    _time_weight[0] = 1.+_dt/sum;
    _time_weight[1] =-sum/_dt_old;
    _time_weight[2] =_dt*_dt/_dt_old/sum;
    break;

  default:
    break;
  }

}

void
NonlinearSystem::setInitialSolution()
{

  NumericVector<Number> & initial_solution( solution() );

  // do nodal BC
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    short int boundary_id = bnode->_bnd_id;
    Node * node = bnode->_node;

    if(node->processor_id() == libMesh::processor_id())
    {
      // reinit variables in nodes
      _problem.reinitNodeFace(node, boundary_id, 0);

      for (std::vector<PresetNodalBC *>::iterator it = _bcs[0].getPresetNodalBCs(boundary_id).begin(); it != _bcs[0].getPresetNodalBCs(boundary_id).end(); ++it)
        (*it)->computeValue(initial_solution);
    }
  }

  _sys.solution->close();
  update();
}

void
NonlinearSystem::subdomainSetup(unsigned int /*subdomain*/, THREAD_ID tid)
{
  //Global Kernels
  for(std::vector<Kernel *>::const_iterator kernel_it = _kernels[tid].active().begin(); kernel_it != _kernels[tid].active().end(); kernel_it++)
    (*kernel_it)->subdomainSetup();
}

void
NonlinearSystem::computeTimeDeriv()
{
  if (!_problem.isTransient())
    return;

  switch (_time_stepping_scheme)
  {
  case Moose::IMPLICIT_EULER:
    _solution_u_dot = *currentSolution();
    _solution_u_dot -= solutionOld();
    _solution_u_dot /= _dt;

    _solution_du_dot_du = 1.0 / _dt;
    break;

  case Moose::CRANK_NICOLSON:
    _solution_u_dot = *currentSolution();
    _solution_u_dot *= 2. / _dt;

    _solution_du_dot_du = 1.0/_dt;
    break;

  case Moose::BDF2:
    if (_t_step == 1)
    {
      // Use backward-euler for the first step
      _solution_u_dot = *currentSolution();
      _solution_u_dot -= solutionOld();
      _solution_u_dot /= _dt;

      _solution_du_dot_du = 1.0/_dt;
    }
    else
    {
      _solution_u_dot.zero();
      _solution_u_dot.add(_time_weight[0], *currentSolution());
      _solution_u_dot.add(_time_weight[1], solutionOld());
      _solution_u_dot.add(_time_weight[2], solutionOlder());
      _solution_u_dot.scale(1./_dt);

      _solution_du_dot_du = _time_weight[0]/_dt;
    }
    break;
  }

  _solution_u_dot.close();
  _solution_du_dot_du.close();
}

void
NonlinearSystem::computeResidualInternal(NumericVector<Number> & residual)
{
  residual.zero();

  // residualSetup() /////
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _kernels[i].residualSetup();
    _bcs[i].residualSetup();
    _dirac_kernels[i].residualSetup();
    if (_doing_dg) _dg_kernels[i].residualSetup();
  }

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeResidualThread cr(_problem, *this, residual);
  Threads::parallel_reduce(elem_range, cr);

  if(_need_residual_copy)
  {
    Moose::perf_log.push("residual.close1()","Solve");
    residual.close();
    Moose::perf_log.pop("residual.close1()","Solve");
    residual.localize(_residual_copy);
  }

  if(_need_residual_ghosted)
  {
    Moose::perf_log.push("residual.close2()","Solve");
    residual.close();
    Moose::perf_log.pop("residual.close2()","Solve");
    _residual_ghosted = residual;
    _residual_ghosted.close();
  }

  computeDiracContributions(&residual);

  Moose::perf_log.push("residual.close3()","Solve");
  residual.close();
  Moose::perf_log.pop("residual.close3()","Solve");

  // do nodal BC
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    short int boundary_id = bnode->_bnd_id;
    Node * node = bnode->_node;

    if(node->processor_id() == libMesh::processor_id())
    {
      // reinit variables in nodes
      _problem.reinitNodeFace(node, boundary_id, 0);

      for (std::vector<NodalBC *>::iterator it = _bcs[0].getNodalBCs(boundary_id).begin(); it != _bcs[0].getNodalBCs(boundary_id).end(); ++it)
        (*it)->computeResidual(residual);
    }
  }

  Moose::perf_log.push("residual.close4()","Solve");
  residual.close();
  Moose::perf_log.pop("residual.close4()","Solve");
}

void
NonlinearSystem::finishResidual(NumericVector<Number> & residual)
{
  switch (_time_stepping_scheme)
  {
  case Moose::CRANK_NICOLSON:
    residual.add(*_residual_old);
    residual.close();
    break;

  default:
    break;
  }
}

void
NonlinearSystem::computeJacobian(SparseMatrix<Number> & jacobian)
{
  Moose::perf_log.push("compute_jacobian()","Solve");

  Moose::enableFPE();

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
#endif

  // jacobianSetup /////
  for(unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _kernels[i].jacobianSetup();
    _bcs[i].jacobianSetup();
    _dirac_kernels[i].jacobianSetup();
    if (_doing_dg) _dg_kernels[i].jacobianSetup();
  }

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  switch (_coupling)
  {
  case Moose::COUPLING_DIAG:
    {
      ComputeJacobianThread cj(_problem, *this, jacobian);
      Threads::parallel_reduce(elem_range, cj);
    }
    break;

  default:
  case Moose::COUPLING_CUSTOM:
    {
      ComputeFullJacobianThread cj(_problem, *this, jacobian);
      Threads::parallel_reduce(elem_range, cj);
    }
    break;
  }

  computeDiracContributions(NULL, &jacobian);

  jacobian.close();

  // do nodal BC
  std::vector<int> zero_rows;

  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    unsigned int boundary_id = bnode->_bnd_id;
    Node * node = bnode->_node;

    if(node->processor_id() == libMesh::processor_id())
    {
      _problem.reinitNodeFace(node, boundary_id, 0);

      for (std::vector<NodalBC *>::iterator it = _bcs[0].getNodalBCs(boundary_id).begin(); it != _bcs[0].getNodalBCs(boundary_id).end(); ++it)
        zero_rows.push_back((*it)->variable().nodalDofIndex());
    }
  }

  jacobian.zero_rows(zero_rows, 1.0);

  jacobian.close();

  if(_add_implicit_geometric_coupling_entries_to_jacobian)
  {
    {
      GeometricSearchData & geom_search_data = _mproblem.geomSearchData();
      std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> & nearest_node_locators = geom_search_data._nearest_node_locators;
      unsigned int n_vars = _sys.n_vars();

      for(std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator it = nearest_node_locators.begin();
          it != nearest_node_locators.end();
          ++it)
      {
        std::vector<unsigned int> & slave_nodes = it->second->_slave_nodes;

        for(unsigned int i=0; i<slave_nodes.size(); i++)
        {
          unsigned int slave_node = slave_nodes[i];

          std::vector<unsigned int> slave_dof_indices(n_vars);

          for(unsigned int j=0; j<n_vars; j++)
            slave_dof_indices[j] = _mesh.node(slave_node).dof_number(_sys.number(), j, 0);

          std::vector<unsigned int> master_nodes = it->second->_neighbor_nodes[slave_node];

          for(unsigned int k=0; k<master_nodes.size(); k++)
          {
            unsigned int master_node = master_nodes[k];

            std::vector<unsigned int> master_dof_indices(n_vars);

            for(unsigned int j=0; j<n_vars; j++)
              master_dof_indices[j] = _mesh.node(master_node).dof_number(_sys.number(), j, 0);

            for(unsigned int l=0; l<slave_dof_indices.size(); l++)
            {
              for(unsigned int m=0; m<master_dof_indices.size(); m++)
              {
                jacobian.set(slave_dof_indices[l], master_dof_indices[m], 0);
                jacobian.set(master_dof_indices[m], slave_dof_indices[l], 0);
              }
            }
          }
        }
      }
    }

    if(_mproblem.getDisplacedProblem())
    {
      GeometricSearchData & geom_search_data = _mproblem.getDisplacedProblem()->geomSearchData();
      std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> & nearest_node_locators = geom_search_data._nearest_node_locators;
      unsigned int n_vars = _sys.n_vars();

      for(std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator it = nearest_node_locators.begin();
          it != nearest_node_locators.end();
          ++it)
      {
        std::vector<unsigned int> & slave_nodes = it->second->_slave_nodes;

        for(unsigned int i=0; i<slave_nodes.size(); i++)
        {
          unsigned int slave_node = slave_nodes[i];

          std::vector<unsigned int> slave_dof_indices(n_vars);

          for(unsigned int j=0; j<n_vars; j++)
            slave_dof_indices[j] = _mesh.node(slave_node).dof_number(_sys.number(), j, 0);

          std::vector<unsigned int> master_nodes = it->second->_neighbor_nodes[slave_node];

          for(unsigned int k=0; k<master_nodes.size(); k++)
          {
            unsigned int master_node = master_nodes[k];

            std::vector<unsigned int> master_dof_indices(n_vars);

            for(unsigned int j=0; j<n_vars; j++)
              master_dof_indices[j] = _mesh.node(master_node).dof_number(_sys.number(), j, 0);

            for(unsigned int l=0; l<slave_dof_indices.size(); l++)
            {
              for(unsigned int m=0; m<master_dof_indices.size(); m++)
              {
                jacobian.set(slave_dof_indices[l], master_dof_indices[m], 0);
                jacobian.set(master_dof_indices[m], slave_dof_indices[l], 0);
              }
            }
          }
        }
      }
    }
  }

  _currently_computing_jacobian = false;

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
#endif

/*
    Threads::parallel_for(ConstElemRange(Moose::mesh->active_local_elements_begin(),
                                         Moose::mesh->active_local_elements_end(),1),
                          ComputeInternalJacobianBlocks(soln, jacobian, precond_system, ivar, jvar));
*/
  {
    unsigned int tid = 0;

    MeshBase::const_element_iterator el = _mesh.active_local_elements_begin();
    const MeshBase::const_element_iterator end_el = _mesh.active_local_elements_end();

    unsigned int subdomain = std::numeric_limits<unsigned int>::max();

    const DofMap & dof_map = precond_system.get_dof_map();
    std::vector<unsigned int> dof_indices;

    jacobian.zero();

    for (; el != end_el; ++el)
    {
      const Elem* elem = *el;
      unsigned int cur_subdomain = elem->subdomain_id();

      dof_map.dof_indices(elem, dof_indices);
      if(dof_indices.size())
      {
        _problem.prepare(elem, ivar, jvar, dof_indices, tid);
        _problem.reinitElem(elem, tid);

        if(cur_subdomain != subdomain)
        {
          subdomain = cur_subdomain;
          _problem.subdomainSetup(subdomain, tid);
          _kernels[tid].updateActiveKernels(_t, _dt, cur_subdomain);
        }

        _problem.parent()->reinitMaterials(cur_subdomain, tid);

        //Kernels
        for(std::vector<Kernel *>::const_iterator kernel_it=_kernels[tid].active().begin(); kernel_it != _kernels[tid].active().end(); kernel_it++)
        {
          Kernel * kernel = *kernel_it;

          if(kernel->variable().number() == ivar)
          {
            kernel->subProblem().prepareShapes(jvar, tid);
            kernel->computeOffDiagJacobian(jvar);
          }
        }

        for (unsigned int side=0; side<elem->n_sides(); side++)
        {
          std::vector<short int> boundary_ids = _mesh.boundary_ids(elem, side);

          if (boundary_ids.size() > 0)
          {
            for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
            {
              short int bnd_id = *it;

              std::vector<IntegratedBC *> bcs = _bcs[tid].getBCs(bnd_id);
              if (bcs.size() > 0)
              {
                _problem.reinitElemFace(elem, side, bnd_id, tid);
                _problem.parent()->reinitMaterialsFace(elem->subdomain_id(), side, tid);

                for (std::vector<IntegratedBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
                {
                  IntegratedBC * bc = *it;
                  if(bc->variable().number() == ivar)
                  {
                    bc->subProblem().prepareFaceShapes(jvar, tid);
                    bc->computeJacobianBlock(jvar);
                  }
                }
              }
            }
          }

          if (elem->neighbor(side) != NULL)
          {
            // on internal edge
            // Pointer to the neighbor we are currently working on.
            const Elem * neighbor = elem->neighbor(side);

            // Get the global id of the element and the neighbor
            const unsigned int elem_id = elem->id();
            const unsigned int neighbor_id = neighbor->id();

            if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
            {
              std::vector<DGKernel *> dgks = _dg_kernels[tid].active();
              if (dgks.size() > 0)
              {
                _problem.reinitNeighbor(elem, side, tid);

                _problem.reinitMaterialsFace(elem->subdomain_id(), side, tid);
                _problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), side, tid);

                for (std::vector<DGKernel *>::iterator it = dgks.begin(); it != dgks.end(); ++it)
                {
                  DGKernel * dg = *it;
                  if(dg->variable().number() == ivar)
                  {
                    dg->subProblem().prepareNeighborShapes(jvar, tid);
                    dg->computeOffDiagJacobian(jvar);
                  }
                }

                std::vector<unsigned int> neighbor_dof_indices;
                dof_map.dof_indices(neighbor, neighbor_dof_indices);
                {
                  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
                  _problem.addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices, tid);
                }
              }
            }
          }
        }

        _problem.addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, tid);
      }
    }
  }

  jacobian.close();

  //Dirichlet BCs
  std::vector<int> zero_rows;

  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    unsigned int boundary_id = bnode->_bnd_id;
    Node * node = bnode->_node;

    std::vector<NodalBC *> & bcs = _bcs[0].getNodalBCs(boundary_id);
    if(bcs.size() > 0)
    {
      if(node->processor_id() == libMesh::processor_id())
      {
        _problem.parent()->reinitNodeFace(node, boundary_id, 0);

        for (std::vector<NodalBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
        {
          //The first zero is for the variable number... there is only one variable in each mini-system
          //The second zero only works with Lagrange elements!
          if((*it)->variable().number() == ivar)
          {
            zero_rows.push_back(node->dof_number(precond_system.number(), 0, 0));
          }
        }
      }
    }
  }

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
    ComputeDampingThread cid(_problem, *this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cid);
    damping = cid.damping();
  }

  Parallel::min(damping);

  Moose::perf_log.pop("compute_dampers()","Solve");

  return damping;
}

void
NonlinearSystem::computeDiracContributions(NumericVector<Number> * residual,
                                           SparseMatrix<Number> * jacobian)
{
  Moose::perf_log.push("computeDiracContributions()","Solve");

  _mproblem.clearDiracInfo();

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
    ComputeDiracThread cd(_mproblem, *this, residual, jacobian);

    _mproblem.getDiracElements(dirac_elements);

    DistElemRange range(dirac_elements.begin(),
                        dirac_elements.end(),
                        1);
    // TODO: Make Dirac work thread!
    //Threads::parallel_reduce(range, cd);

    cd(range);
  }

  Moose::perf_log.pop("computeDiracContributions()","Solve");
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
NonlinearSystem::augmentSendList(std::vector<unsigned int> & send_list)
{
  std::set<unsigned int> & ghosted_elems = _mproblem._ghosted_elems;

  DofMap & dof_map = dofMap();

  std::vector<unsigned int> dof_indices;

  for(std::set<unsigned int>::iterator elem_id = ghosted_elems.begin();
      elem_id != ghosted_elems.end();
      ++elem_id)
  {
    dof_map.dof_indices(_mesh.elem(*elem_id), dof_indices);

    for(unsigned int i=0; i<dof_indices.size(); i++)
      // Only need to ghost it if it's actually not on this processor
      if(dof_indices[i] < dof_map.first_dof() || dof_indices[i] >= dof_map.end_dof())
        send_list.push_back(dof_indices[i]);
  }
}

void
NonlinearSystem::serializeSolution()
{
  if(_need_serialized_solution)
    _current_solution->localize(_serialized_solution);
 }

void
NonlinearSystem::set_solution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  if(_need_serialized_solution)
    serializeSolution();
}

NumericVector<Number> &
NonlinearSystem::serializedSolution()
{
  _need_serialized_solution = true;
  return _serialized_solution;
}

void
NonlinearSystem::printVarNorms()
{
  TransientNonlinearImplicitSystem &s = static_cast<TransientNonlinearImplicitSystem &>(_sys);

  std::cout << "Norm of each nonlinear variable:" << std::endl;
  for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
  {
    std::cout << s.variable_name(var_num) << ": "
              << s.calculate_norm(*s.rhs,var_num,DISCRETE_L2) << std::endl;
  }
}

void
NonlinearSystem::setPreconditioner(Preconditioner<Real> *pc)
{
  _preconditioner = pc;

  // We don't want to be computing the big Jacobian!
  _sys.nonlinear_solver->jacobian = NULL;
  _sys.nonlinear_solver->attach_preconditioner(pc);
}

void
NonlinearSystem::setupDampers()
{
  _increment_vec = &_sys.add_vector("u_increment", true, GHOSTED);
}

void
NonlinearSystem::reinitDampers(THREAD_ID tid)
{
  // FIXME: be smart here and compute only variables with dampers (need to add some book keeping)
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeDamping(*_increment_vec);
  }
}

void
NonlinearSystem::checkKernelCoverage(const std::set<subdomain_id_type> & mesh_subdomains) const
{
  // Check kernel coverage of subdomains (blocks) in your mesh
  std::set<subdomain_id_type> input_subdomains;

  bool global_kernels_exist = _kernels[0].subdomains_covered(input_subdomains);

  if (!global_kernels_exist)
  {
    std::set<subdomain_id_type> difference;
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
  vec.resize(residual.size());
  for (unsigned int nd = 0; nd < _mesh.n_nodes(); ++nd)
  {
    const Node & node = _mesh.node(nd);
    for (unsigned int var = 0; var < node.n_vars(_sys.number()); ++var)
    {
      unsigned int dof_idx = node.dof_number(_sys.number(), var, 0);
      vec[dof_idx] = st(var, nd, residual(dof_idx));
    }
  }
  // sort vec by residuals
  std::sort(vec.begin(), vec.end(), dbg_sort_residuals);
  // print out
  std::cerr << "[DBG] Max " << n << " residuals" << std::endl;
  for (unsigned int i = 0; i < n; ++i)
  {
    fprintf(stderr, "[DBG]  % .15e '%s' at node %d\n", vec[i]._residual, _sys.variable_name(vec[i]._var).c_str(), vec[i]._nd);
  }
}
