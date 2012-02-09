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
#include "ComputeDiracThread.h"
#include "ComputeDampingThread.h"
#include "TimeKernel.h"
#include "FP.h"
#include "DisplacedProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "ScalarKernel.h"

// libMesh
#include "nonlinear_solver.h"
#include "quadrature_gauss.h"
#include "dense_vector.h"
#include "boundary_info.h"
#include "petsc_matrix.h"
#include "petsc_vector.h"
#include "petsc_nonlinear_solver.h"
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

NonlinearSystem::NonlinearSystem(FEProblem & subproblem, const std::string & name) :
    SystemTempl<TransientNonlinearImplicitSystem>(subproblem, name, Moose::VAR_NONLINEAR),
    _mproblem(subproblem),
    _last_rnorm(0),
    _l_abs_step_tol(1e-10),
    _initial_residual(0),
    _current_solution(NULL),
    _older_solution(solutionOlder()),
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
    _debugging_residuals(false),
    _doing_dg(false),
    _n_iters(0),
    _n_linear_iters(0),
    _final_residual(0.),
    _use_predictor(false),
    _predictor_scale(0.0)
{
  _sys.nonlinear_solver->residual = Moose::compute_residual;
  _sys.nonlinear_solver->jacobian = Moose::compute_jacobian;

  _time_weight.resize(3);
  timeSteppingScheme(Moose::IMPLICIT_EULER);                   // default time stepping scheme

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
  delete _preconditioner;
  delete &_serialized_solution;
  delete &_residual_copy;
}

void
NonlinearSystem::init()
{
  dofMap().attach_extra_send_list_function(&extraSendList, this);

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
  //Calculate the initial residual for use in the convergence criterion.  The initial
  //residual
  _mproblem.computeResidual(_sys, *_current_solution, *_sys.rhs);
  _sys.rhs->close();
  _initial_residual = _sys.rhs->l2_norm();
  std::cout <<std::scientific<<std::setprecision(6);
  std::cout << "  Initial |residual|_2 = "<<_initial_residual<<"\n";

  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;

  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  if(_use_finite_differenced_preconditioner)
    setupFiniteDifferencedPreconditioner();

  _sys.solve();
  // store info about the solve
  _n_iters = _sys.n_nonlinear_iterations();
  _final_residual = _sys.final_nonlinear_residual();

#ifdef LIBMESH_HAVE_PETSC
  _n_linear_iters = static_cast<PetscNonlinearSolver<Real> &>(*_sys.nonlinear_solver).get_total_linear_iterations();
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

  PetscVector<Number>* petsc_vec =
    dynamic_cast<PetscVector<Number>*>(_sys.solution.get());

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

#if PETSC_VERSION_LESS_THAN(3,2,0)
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORING_LF, &iscoloring);
#else
  ierr = MatGetColoring(petsc_mat->mat(), MATCOLORINGLF, &iscoloring);
#endif
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  MatFDColoringCreate(petsc_mat->mat(),iscoloring,&fdcoloring);
#if PETSC_VERSION_LESS_THAN(3,2,0)
  ISColoringDestroy(iscoloring);
#else
  ISColoringDestroy(&iscoloring);
#endif
  MatFDColoringSetFromOptions(fdcoloring);
  MatFDColoringSetFunction(fdcoloring,
                           (PetscErrorCode (*)(void))&libMesh::__libmesh_petsc_snes_residual,
                           &petsc_nonlinear_solver);
  SNESSetJacobian(petsc_nonlinear_solver.snes(),
                  petsc_mat->mat(),
                  petsc_mat->mat(),
                  SNESDefaultComputeJacobianColor,
                  fdcoloring);

    Mat my_mat = petsc_mat->mat();
  MatStructure my_struct;

  SNESComputeJacobian(petsc_nonlinear_solver.snes(),
                      petsc_vec->vec(),
                      &my_mat,
                      &my_mat,
                      &my_struct);

//  std::cout<<*_sys.matrix<<std::endl;
#endif
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
NonlinearSystem::addScalarKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    ScalarKernel *kernel = static_cast<ScalarKernel *>(Factory::instance()->create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Kernel object");

    _kernels[tid].addScalarKernel(kernel);
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
NonlinearSystem::addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters)
{
  parameters.set<THREAD_ID>("_tid") = 0;

  MooseObject * obj = Factory::instance()->create(c_name, name, parameters);

  NodalConstraint    * nc = dynamic_cast<NodalConstraint *>(obj);
  NodeFaceConstraint * nfc = dynamic_cast<NodeFaceConstraint *>(obj);
  if (nfc != NULL)
  {
    unsigned int slave = parameters.get<unsigned int>("slave");
    unsigned int master = parameters.get<unsigned int>("master");
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

  computeTimeDerivatives();
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

  if (_use_predictor)
    applyPredictor(initial_solution,_older_solution);

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

  // Set constraint slave values
  setConstraintSlaveValues(initial_solution, false);

  if(_mproblem.getDisplacedProblem())
    setConstraintSlaveValues(initial_solution, true);
}

void
NonlinearSystem::applyPredictor(NumericVector<Number> & initial_solution,
                                NumericVector<Number> & previous_solution)
{
  // A Predictor is an algorithm that will predict the next solution based on
  // previous solutions.  Basically, it works like:
  //
  //             sol - prev_sol
  // sol = sol + -------------- * dt * scale_factor
  //                 dt_old
  //
  // The scale factor can be set to 1 for times when the solution is expected
  // to change linearly or smoothly.  If the solution is less continuous over
  // time, it may be better to set to to 0.
  //   In the ideal case of a linear model with linearly changing bcs, the Predictor
  // can determine the solution before the solver is invoked (a solution is computed
  // in zero solver iterations).  Even outside the ideal case, a good Predictor
  // significantly reduces the number of solver iterations required.
  //   It is important to compute the initial residual to be used as a relative
  // convergence criterion before applying the predictor.  If this is not done,
  // the residual is likely to be much lower after applying the predictor, which would
  // result in a much more stringent criterion for convergence than would have been
  // used if the predictor were not enabled.

  if (_dt_old > 0)
  {
    std::streamsize cur_precision(std::cout.precision());
    std::cout << "  Applying predictor with scale factor = "<<std::fixed<<std::setprecision(2)<<_predictor_scale<<"\n";
    std::cout << std::scientific << std::setprecision(cur_precision);
    Real dt_adjusted_scale_factor = _predictor_scale * _dt / _dt_old;
    initial_solution *= 1 + dt_adjusted_scale_factor;
    previous_solution *= dt_adjusted_scale_factor;
    initial_solution -= previous_solution;
  }
}

void
NonlinearSystem::subdomainSetup(unsigned int /*subdomain*/, THREAD_ID tid)
{
  //Global Kernels
  for(std::vector<Kernel *>::const_iterator kernel_it = _kernels[tid].active().begin(); kernel_it != _kernels[tid].active().end(); kernel_it++)
    (*kernel_it)->subdomainSetup();
}

void
NonlinearSystem::computeTimeDerivatives()
{
  if (!_mproblem.isTransient())
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
    _problem.reinitNode(&master_node, tid);
    _problem.prepareAssembly(tid);

    // go over slave nodes
    std::vector<unsigned int> slave_nodes = nc->getSlaveNodeId();
    for (std::vector<unsigned int>::iterator it = slave_nodes.begin(); it != slave_nodes.end(); ++it)
    {
      Node & slave_node = _mesh.node(*it);
      // reinit variables on the slave node
      _problem.reinitNodeNeighbor(&slave_node, tid);
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
    _problem.reinitNode(&master_node, tid);
    _problem.prepareAssembly(tid);

    // go over slave nodes
    std::vector<unsigned int> slave_nodes = nc->getSlaveNodeId();
    for (std::vector<unsigned int>::iterator it = slave_nodes.begin(); it != slave_nodes.end(); ++it)
    {
      Node & slave_node = _mesh.node(*it);
      // reinit variables on the slave node
      _problem.reinitNodeNeighbor(&slave_node, tid);
      // compute residual
      nc->computeJacobian(jacobian);
    }
  }
}

void
NonlinearSystem::setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced)
{
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = NULL;

  if(!displaced)
  {
    GeometricSearchData & geom_search_data = _mproblem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _mproblem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied = false;

  for(std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {
    PenetrationLocator & pen_loc = *it->second;

    std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    unsigned int slave_boundary = pen_loc._slave_boundary;

    std::vector<NodeFaceConstraint *> constraints;

    if(!displaced)
      constraints = _constraints[0].getNodeFaceConstraints(slave_boundary);
    else
      constraints = _constraints[0].getDisplacedNodeFaceConstraints(slave_boundary);

    if(constraints.size())
    {
      for(unsigned int i=0; i<slave_nodes.size(); i++)
      {
        unsigned int slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if(slave_node.processor_id() == libMesh::processor_id())
        {
          if(pen_loc._penetration_info[slave_node_num])
          {
            PenetrationLocator::PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _problem.reinitNeighborPhys(master_elem, master_side, points, 0);

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
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = NULL;

  if(!displaced)
  {
    GeometricSearchData & geom_search_data = _mproblem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _mproblem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied = false;

  for(std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {
    PenetrationLocator & pen_loc = *it->second;

    std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    unsigned int slave_boundary = pen_loc._slave_boundary;

    std::vector<NodeFaceConstraint *> constraints;

    if(!displaced)
      constraints = _constraints[0].getNodeFaceConstraints(slave_boundary);
    else
      constraints = _constraints[0].getDisplacedNodeFaceConstraints(slave_boundary);

    if(constraints.size())
    {
      for(unsigned int i=0; i<slave_nodes.size(); i++)
      {
        unsigned int slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if(slave_node.processor_id() == libMesh::processor_id())
        {
          if(pen_loc._penetration_info[slave_node_num])
          {
            PenetrationLocator::PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _problem.reinitNeighborPhys(master_elem, master_side, points, 0);

            for(unsigned int c=0; c < constraints.size(); c++)
            {
              NodeFaceConstraint * nfc = constraints[c];

              if(nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeResidual();

                if(nfc->overwriteSlaveResidual())
                  _problem.setResidual(residual, 0);
                else
                  _problem.cacheResidual(0);
                _problem.cacheResidualNeighbor(0);
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
    residual.close();
    _problem.addCachedResidual(residual, 0);
  }
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
  ComputeResidualThread cr(_mproblem, *this, residual);
  Threads::parallel_reduce(elem_range, cr);
  // do scalar kernels (not sure how to thread this)
  {
    const std::vector<ScalarKernel *> & scalars = _kernels[0].scalars();
    for (std::vector<ScalarKernel *>::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
    {
      ScalarKernel * kernel = *it;

      kernel->reinit();
      kernel->computeResidual();
      _mproblem.addResidualScalar(residual);
    }
  }

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

  if(_mproblem._has_constraints)
  {
    enforceNodalConstraintsResidual(residual);
  }
  residual.close();

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
      {
        NodalBC * bc = *it;
        if (bc->shouldApply())
          bc->computeResidual(residual);
      }
    }
  }

  Moose::perf_log.push("residual.close4()","Solve");
  residual.close();
  Moose::perf_log.pop("residual.close4()","Solve");

  // Add in Residual contributions from Constraints
  if(_mproblem._has_constraints)
  {
    // Undisplaced Constraints
    constraintResiduals(residual, false);

    // Displaced Constraints
    if(_mproblem.getDisplacedProblem())
      constraintResiduals(residual, true);
  }


  //std::cerr << "--" << std::endl;
  //residual.print(std::cerr);


  // If we are debugging residuals we need one more assignment to have the ghosted copy up to date
  if(_need_residual_ghosted && _debugging_residuals)
  {
    Moose::perf_log.push("residual.close5()","Solve");
    residual.close();
    Moose::perf_log.pop("residual.close5()","Solve");
    _residual_ghosted = residual;
    _residual_ghosted.close();
  }
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
NonlinearSystem::findImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data, std::map<unsigned int, std::vector<unsigned int> > & graph)
{
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> & nearest_node_locators = geom_search_data._nearest_node_locators;

  for(std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator it = nearest_node_locators.begin();
      it != nearest_node_locators.end();
      ++it)
  {
    std::vector<unsigned int> & slave_nodes = it->second->_slave_nodes;

    for(unsigned int i=0; i<slave_nodes.size(); i++)
    {
      std::set<unsigned int> unique_slave_indices;
      std::set<unsigned int> unique_master_indices;

      unsigned int slave_node = slave_nodes[i];

      {
        std::vector<unsigned int> & elems = _mesh.nodeToElemMap()[slave_node];

        // Get the dof indices from each elem connected to the node
        for(unsigned int el=0; el < elems.size(); ++el)
        {
          unsigned int cur_elem = elems[el];

          std::vector<unsigned int> dof_indices;
          dofMap().dof_indices(_mesh.elem(cur_elem), dof_indices);

          for(unsigned int di=0; di < dof_indices.size(); di++)
            unique_slave_indices.insert(dof_indices[di]);
        }
      }

      std::vector<unsigned int> master_nodes = it->second->_neighbor_nodes[slave_node];

      for(unsigned int k=0; k<master_nodes.size(); k++)
      {
        unsigned int master_node = master_nodes[k];

        {
          std::vector<unsigned int> & elems = _mesh.nodeToElemMap()[master_node];

          // Get the dof indices from each elem connected to the node
          for(unsigned int el=0; el < elems.size(); ++el)
          {
            unsigned int cur_elem = elems[el];

            std::vector<unsigned int> dof_indices;
            dofMap().dof_indices(_mesh.elem(cur_elem), dof_indices);

            for(unsigned int di=0; di < dof_indices.size(); di++)
              unique_master_indices.insert(dof_indices[di]);
          }
        }
      }

      for(std::set<unsigned int>::iterator sit=unique_slave_indices.begin(); sit != unique_slave_indices.end(); ++sit)
      {
        unsigned int slave_id = *sit;

        for(std::set<unsigned int>::iterator mit=unique_master_indices.begin(); mit != unique_master_indices.end(); ++mit)
        {
          unsigned int master_id = *mit;

          graph[slave_id].push_back(master_id);
          graph[master_id].push_back(slave_id);
        }
      }
    }
  }

  // Make every entry sorted and unique
  for(std::map<unsigned int, std::vector<unsigned int> >::iterator git=graph.begin(); git != graph.end(); ++git)
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
  std::map<unsigned int, std::vector<unsigned int> > graph;

  findImplicitGeometricCouplingEntries(geom_search_data, graph);

  for(std::map<unsigned int, std::vector<unsigned int> >::iterator git=graph.begin(); git != graph.end(); ++git)
  {
    unsigned int dof = git->first;
    std::vector<unsigned int> & row = git->second;

    for(unsigned int i=0; i<row.size(); i++)
    {
      unsigned int coupled_dof = row[i];
//          if(slave_dof_indices[l] >= _sys.get_dof_map().first_dof() && slave_dof_indices[l] < _sys.get_dof_map().end_dof())
      jacobian.add(dof, coupled_dof, 0);
    }
  }
}

void
NonlinearSystem::constraintJacobians(SparseMatrix<Number> & jacobian, bool displaced)
{
  std::vector<int> zero_rows;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators = NULL;

  if(!displaced)
  {
    GeometricSearchData & geom_search_data = _mproblem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data = _mproblem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied = false;

  for(std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = penetration_locators->begin();
      it != penetration_locators->end();
      ++it)
  {
    PenetrationLocator & pen_loc = *it->second;

    std::vector<unsigned int> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    unsigned int slave_boundary = pen_loc._slave_boundary;

    std::vector<NodeFaceConstraint *> constraints;

    if(!displaced)
      constraints = _constraints[0].getNodeFaceConstraints(slave_boundary);
    else
      constraints = _constraints[0].getDisplacedNodeFaceConstraints(slave_boundary);

    if(constraints.size())
    {
      for(unsigned int i=0; i<slave_nodes.size(); i++)
      {
        unsigned int slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.node(slave_node_num);

        if(slave_node.processor_id() == libMesh::processor_id())
        {
          if(pen_loc._penetration_info[slave_node_num])
          {
            PenetrationLocator::PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _problem.reinitNeighborPhys(master_elem, master_side, points, 0);
            for(unsigned int c=0; c < constraints.size(); c++)
            {
              NodeFaceConstraint * nfc = constraints[c];

              nfc->_jacobian = &jacobian;

              if(nfc->shouldApply())
              {
                constraints_applied = true;

                nfc->subProblem().prepareShapes(nfc->variable().number(), 0);
                nfc->subProblem().prepareNeighborShapes(nfc->variable().number(), 0);

                nfc->computeJacobian();

                if(nfc->overwriteSlaveResidual())
                {
                  // Add this variable's dof's row to be zeroed
                  zero_rows.push_back(nfc->variable().nodalDofIndex());
                }

                std::vector<unsigned int> slave_dofs(1,nfc->variable().nodalDofIndex());

                // Cache the jacobian block for the slave size
                _subproblem.assembly(0).cacheJacobianBlock(nfc->_Kee, slave_dofs, nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                // Cache the jacobian block for the master side
                _subproblem.assembly(0).cacheJacobianBlock(nfc->_Kne, nfc->variable().dofIndicesNeighbor(), nfc->_connected_dof_indices, nfc->variable().scalingFactor());

                _problem.cacheJacobian(0);
                _problem.cacheJacobianNeighbor(0);
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
    _problem.addCachedJacobian(jacobian, 0);
    jacobian.close();
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
    _constraints[i].jacobianSetup();
    if (_doing_dg) _dg_kernels[i].jacobianSetup();
  }

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  switch (_mproblem.coupling())
  {
  case Moose::COUPLING_DIAG:
    {
      ComputeJacobianThread cj(_mproblem, *this, jacobian);
      Threads::parallel_reduce(elem_range, cj);
    }
    break;

  default:
  case Moose::COUPLING_CUSTOM:
    {
      ComputeFullJacobianThread cj(_mproblem, *this, jacobian);
      Threads::parallel_reduce(elem_range, cj);
    }
    break;
  }

  computeDiracContributions(NULL, &jacobian);

  // do scalar kernels (not sure how to thread this)
  {
    const std::vector<ScalarKernel *> & scalars = _kernels[0].scalars();
    for (std::vector<ScalarKernel *>::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
    {
      ScalarKernel * kernel = *it;

      kernel->reinit();
      kernel->computeJacobian();
      _mproblem.addJacobianScalar(jacobian);
    }
  }

  static bool first = true;

  // This adds zeroes into geometric coupling entries to ensure they stay in the matrix
  if(first && (_add_implicit_geometric_coupling_entries_to_jacobian || _mproblem._has_constraints))
  {
    first = false;
    addImplicitGeometricCouplingEntries(jacobian, _mproblem.geomSearchData());

    if(_mproblem.getDisplacedProblem())
      addImplicitGeometricCouplingEntries(jacobian, _mproblem.getDisplacedProblem()->geomSearchData());
  }

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
      {
        NodalBC * bc = *it;
        if (bc->shouldApply())
          zero_rows.push_back(bc->variable().nodalDofIndex());
      }
    }
  }

  jacobian.zero_rows(zero_rows, 1.0);

  jacobian.close();

  // Add in Jacobian contributions from Constraints
  if(_mproblem._has_constraints)
  {
    // Nodal Constraints
    enforceNodalConstraintsJacobian(jacobian);

    // Undisplaced Constraints
    constraintJacobians(jacobian, false);

    // Displaced Constraints
    if(_mproblem.getDisplacedProblem())
      constraintJacobians(jacobian, true);
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
                    if (bc->shouldApply())
                    {
                      bc->subProblem().prepareFaceShapes(jvar, tid);
                      bc->computeJacobianBlock(jvar);
                    }
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
          NodalBC * bc = *it;
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
NonlinearSystem::augmentSparsity(SparsityPattern::Graph & sparsity,
                                 std::vector<unsigned int> & n_nz,
                                 std::vector<unsigned int> & n_oz)
{

  if(_add_implicit_geometric_coupling_entries_to_jacobian || _mproblem._has_constraints)
  {
    _mproblem.updateGeomSearch();

    std::map<unsigned int, std::vector<unsigned int> > graph;

    findImplicitGeometricCouplingEntries(_mproblem.geomSearchData(), graph);

    if(_mproblem.getDisplacedProblem())
      findImplicitGeometricCouplingEntries(_mproblem.getDisplacedProblem()->geomSearchData(), graph);

    const unsigned int first_dof_on_proc = dofMap().first_dof(libMesh::processor_id());
    const unsigned int end_dof_on_proc   = dofMap().end_dof(libMesh::processor_id());

    for(std::map<unsigned int, std::vector<unsigned int> >::iterator git=graph.begin(); git != graph.end(); ++git)
    {
      unsigned int dof = git->first;
      unsigned int local_dof = dof - first_dof_on_proc;

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
        unsigned int coupled_dof = row[i];

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
NonlinearSystem::set_solution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  if(_need_serialized_solution)
    serializeSolution();
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

  std::cout << "Norm of each nonlinear variable's residual:" << std::endl;
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
  vec.resize(residual.local_size());

  unsigned int j = 0;
  for (MeshBase::node_iterator it = _mesh._mesh.local_nodes_begin(); it != _mesh._mesh.local_nodes_end(); ++it)
  {
    Node & node = *(*it);
    unsigned int nd = node.id();

    for (unsigned int var = 0; var < node.n_vars(_sys.number()); ++var)
    {
      if (node.n_dofs(_sys.number(), var) > 0)
      {
        unsigned int dof_idx = node.dof_number(_sys.number(), var, 0);
        vec[j] = st(var, nd, residual(dof_idx));
        j++;
      }
    }
  }
  // sort vec by residuals
  std::sort(vec.begin(), vec.end(), dbg_sort_residuals);
  // print out
  std::cerr << "[DBG][" << libMesh::processor_id() << "] Max " << n << " residuals";
  if (j < n)
  {
    n = j;
    std::cerr << " (Only " << n << " available)";
  }
  std::cerr << std::endl;

  for (unsigned int i = 0; i < n; ++i)
  {
    fprintf(stderr, "[DBG][%d]  % .15e '%s' at node %d\n", libMesh::processor_id(), vec[i]._residual, _sys.variable_name(vec[i]._var).c_str(), vec[i]._nd);
  }
}
