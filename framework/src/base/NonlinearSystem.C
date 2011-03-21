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
#include "ComputeDiracThread.h"
#include "ComputeDampingThread.h"

#include "nonlinear_solver.h"
#include "quadrature_gauss.h"
#include "dense_vector.h"
#include "boundary_info.h"
#include "petsc_matrix.h"
#include "numeric_vector.h"
#include "mesh.h"

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
    _nl_solution(_sys.add_vector("curr_sln", false, GHOSTED)),
    _t(subproblem.time()),
    _dt(subproblem.dt()),
    _dt_old(subproblem.dtOld()),
    _t_step(subproblem.timeStep()),
    _time_weight(subproblem.timeWeights()),
    _increment_vec(NULL),
    _preconditioner(NULL)
{
  _sys.nonlinear_solver->residual = Moose::compute_residual;
  _sys.nonlinear_solver->jacobian = Moose::compute_jacobian;

  _sys.attach_init_function(Moose::initial_condition);

  _time_weight.resize(3);
  timeSteppingScheme(Moose::IMPLICIT_EULER);                   // default time stepping scheme

  unsigned int n_threads = libMesh::n_threads();
  _kernels.resize(n_threads);
  _bcs.resize(n_threads);
  _stabilizers.resize(n_threads);
  _dirac_kernels.resize(libMesh::n_threads());
  _dampers.resize(n_threads);
}

NonlinearSystem::~NonlinearSystem()
{
  delete _preconditioner;
}

void
NonlinearSystem::init()
{
  // use computed initial condition
  _nl_solution = *_sys.current_local_solution;
  _nl_solution.close();
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

    std::set<unsigned int> blk_ids;
    if (!parameters.isParamValid("block"))
      blk_ids = _var_map[kernel->variable().number()];
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
      for (unsigned int i=0; i<blocks.size(); ++i)
      {
        if (_var_map[kernel->variable().number()].count(blocks[i]) > 0 || _var_map[kernel->variable().number()].count(Moose::ANY_BLOCK_ID) > 0)
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
NonlinearSystem::addStabilizer(const std::string & stabilizer_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _mproblem._material_data[tid];

    Stabilizer * stabilizer = static_cast<Stabilizer *>(Factory::instance()->create(stabilizer_name, name, parameters));
    if (parameters.have_parameter<unsigned int>("block_id"))
      _stabilizers[tid].addBlockStabilizer(parameters.get<unsigned int>("block_id"), stabilizer);
    else
      _stabilizers[tid].addStabilizer(stabilizer);
  }
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
NonlinearSystem::computeResidual(NumericVector<Number> & residual)
{
  Moose::perf_log.push("compute_residual()","Solve");

  computeTimeDeriv();
  computeResidualInternal(residual);
  finishResidual(residual);
  computeDiracContributions(&residual);

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

    set_solution(solutionOld());                    // use old_solution for computing with correct solution vector
    computeResidualInternal(_residual_old);
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
NonlinearSystem::computeTimeDeriv()
{
  switch (_time_stepping_scheme)
  {
  case Moose::IMPLICIT_EULER:
    _solution_u_dot = solution();
    _solution_u_dot -= solutionOld();
    _solution_u_dot /= _dt;

    _solution_du_dot_du = 1.0 / _dt;
    break;

  case Moose::CRANK_NICOLSON:
    _solution_u_dot = solution();
    _solution_u_dot *= 2. / _dt;

    _solution_du_dot_du = 1.0/_dt;
    break;

  case Moose::BDF2:
    if (_t_step == 1)
    {
      // Use backward-euler for the first step
      _solution_u_dot = solution();
      _solution_u_dot -= solutionOld();
      _solution_u_dot /= _dt;

      _solution_du_dot_du = 1.0/_dt;
    }
    else
    {
      _solution_u_dot.zero();
      _solution_u_dot.add(_time_weight[0], solution());
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

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeResidualThread cr(_problem, *this, residual);
  Threads::parallel_reduce(elem_range, cr);
  residual.close();

  // do nodal BC
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.build_node_list(nodes, ids);

  const unsigned int n_nodes = nodes.size();
  for (unsigned int i = 0; i < n_nodes; i++)
  {
    unsigned int boundary_id = ids[i];
    Node * node = &_mesh.node(nodes[i]);

    // reinit variables in nodes
    _problem.reinitNodeFace(node, boundary_id, 0);

    for (std::vector<NodalBC *>::iterator it = _bcs[0].getNodalBCs(boundary_id).begin(); it != _bcs[0].getNodalBCs(boundary_id).end(); ++it)
      (*it)->computeResidual(residual);
  }
  residual.close();
}

void
NonlinearSystem::finishResidual(NumericVector<Number> & residual)
{
  switch (_time_stepping_scheme)
  {
  case Moose::CRANK_NICOLSON:
    residual.add(_residual_old);
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
#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
#else
  // In Petsc 3.0.0, MatSetOption has three args...the third arg
  // determines whether the option is set (true) or unset (false)
  MatSetOption(static_cast<PetscMatrix<Number> &>(_jacobian).mat(),
   MAT_KEEP_ZEROED_ROWS,
   PETSC_TRUE);
#endif
#endif

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

  ComputeJacobianThread cj(_problem, *this, jacobian);
  Threads::parallel_reduce(elem_range, cj);
  jacobian.close();

  computeDiracContributions(NULL, &jacobian);
  
  // do nodal BC
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.build_node_list(nodes, ids);

  const unsigned int n_nodes = nodes.size();
  for (unsigned int i = 0; i < n_nodes; i++)
  {
    unsigned int boundary_id = ids[i];
    Node * node = &_mesh.node(nodes[i]);

    _problem.reinitNodeFace(node, boundary_id, 0);

    for (std::vector<NodalBC *>::iterator it = _bcs[0].getNodalBCs(boundary_id).begin(); it != _bcs[0].getNodalBCs(boundary_id).end(); ++it)
      (*it)->computeJacobian(jacobian);
  }
  jacobian.close();

  Moose::perf_log.pop("compute_jacobian()","Solve");
}

void
NonlinearSystem::computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar)
{
  Moose::perf_log.push("compute_jacobian_block()","Solve");

#ifdef LIBMESH_HAVE_PETSC
  //Necessary for speed
#if PETSC_VERSION_LESS_THAN(3,0,0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),MAT_KEEP_ZEROED_ROWS);
#else
  // In Petsc 3.0.0, MatSetOption has three args...the third arg
  // determines whether the option is set (true) or unset (false)
  MatSetOption(static_cast<PetscMatrix<Number> &>(_jacobian).mat(),
   MAT_KEEP_ZEROED_ROWS,
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

    StabilizerIterator stabilizer_begin = _stabilizers[tid].activeStabilizersBegin();
    StabilizerIterator stabilizer_end = _stabilizers[tid].activeStabilizersEnd();
    StabilizerIterator stabilizer_it = stabilizer_begin;

    unsigned int subdomain = std::numeric_limits<unsigned int>::max();

    DofMap & dof_map = precond_system.get_dof_map();
    DenseMatrix<Number> Ke;
    std::vector<unsigned int> dof_indices;

    jacobian.zero();

    for (; el != end_el; ++el)
    {
      const Elem* elem = *el;
      unsigned int cur_subdomain = elem->subdomain_id();

      std::set<MooseVariable *> vars;

      _problem.reinitElem(elem, tid);

      dof_map.dof_indices(elem, dof_indices);

      if(dof_indices.size())
      {
        Ke.resize(dof_indices.size(),dof_indices.size());

        if(cur_subdomain != subdomain)
        {
          subdomain = cur_subdomain;
          _problem.subdomainSetup(subdomain, tid);
          _kernels[tid].updateActiveKernels(_t, _dt, cur_subdomain);
        }

        _problem.parent()->reinitMaterials(cur_subdomain, tid);

        //Stabilizers
        for(stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
          (*stabilizer_it)->computeTestFunctions();

        //Kernels
        KernelIterator kernel_begin = _kernels[tid].activeKernelsBegin();
        KernelIterator kernel_end = _kernels[tid].activeKernelsEnd();
        KernelIterator kernel_it = kernel_begin;

        for(kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
        {
          Kernel * kernel = *kernel_it;

          if(kernel->variable().number() == ivar)
            kernel->computeOffDiagJacobian(Ke, jvar);
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
                    bc->computeJacobianBlock(Ke,ivar,jvar);
                }
              }
            }
          }
        }

        dof_map.constrain_element_matrix (Ke, dof_indices, false);
        jacobian.add_matrix(Ke, dof_indices);
      }
    }
  }

  jacobian.close();

  //Dirichlet BCs
  std::vector<int> zero_rows;

  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.build_node_list(nodes, ids);

  const std::set<short int> & boundary_ids = _mesh.get_boundary_ids();

  for(std::set<short int>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
  {
    short int id = *it;

    std::vector<NodalBC *> & bcs = _bcs[0].getNodalBCs(id);
    for (std::vector<NodalBC *>::iterator it = bcs.begin(); it != bcs.end(); ++it)
      (*it)->setup();
  }

  const unsigned int n_nodes = nodes.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = ids[i];

    std::vector<NodalBC *> & bcs = _bcs[0].getNodalBCs(boundary_id);
    if(bcs.size() > 0)
    {
      Node & node = _mesh.node(nodes[i]);

      if(node.processor_id() == libMesh::processor_id())
      {
        _problem.parent()->reinitNodeFace(&node, boundary_id, 0);

        for (std::vector<NodalBC *>::iterator it = _bcs[0].getNodalBCs(boundary_id).begin(); it != _bcs[0].getNodalBCs(boundary_id).end(); ++it)
        {
          //The first zero is for the variable number... there is only one variable in each mini-system
          //The second zero only works with Lagrange elements!
          if((*it)->variable().number() == ivar)
            zero_rows.push_back(node.dof_number(precond_system.number(), 0, 0));
        }
      }
    }
  }

  jacobian.close();

  //This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
  jacobian.zero_rows(zero_rows, 1.0);

  jacobian.close();

  Moose::perf_log.pop("compute_jacobian_block()","Solve");
}

Real
NonlinearSystem::computeDamping(const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()","Solve");

  // Default to no damping
  Real damping = 1.0;

  DamperIterator damper_begin = _dampers[0].dampersBegin();
  DamperIterator damper_end = _dampers[0].dampersEnd();
  DamperIterator damper_it = damper_begin;

  if(damper_begin != damper_end)
  {
    ComputeDampingThread cid(_problem, *this, update);
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
  DiracKernelIterator dirac_kernel_begin = _dirac_kernels[0].diracKernelsBegin();
  DiracKernelIterator dirac_kernel_end = _dirac_kernels[0].diracKernelsEnd();
  DiracKernelIterator dirac_kernel_it = dirac_kernel_begin;

  for(dirac_kernel_it=dirac_kernel_begin;dirac_kernel_it!=dirac_kernel_end;++dirac_kernel_it)
  {
    (*dirac_kernel_it)->clearPoints();
    (*dirac_kernel_it)->addPoints();
  }

  if(dirac_kernel_begin != dirac_kernel_end)
  {
    ComputeDiracThread cd(_mproblem, *this, residual, jacobian);

    _mproblem.getDiracElements(dirac_elements);

    DistElemRange range(dirac_elements.begin(),
                        dirac_elements.end(),
                        1);
    
    Threads::parallel_reduce(range, cd);
  }

  if(residual)
    residual->close();
  else if(jacobian)
    jacobian->close();

  Moose::perf_log.pop("computeDiracContributions()","Solve");

}

void
NonlinearSystem::setVarScaling(std::vector<Real> scaling)
{
//  if(scaling.size() != _system->n_vars())
//    mooseError("Error: Size of scaling factor vector not the same as the number of variables in the system!\n");
//
//  _scaling_factor = scaling;
}

void
NonlinearSystem::setScaling()
{
  std::vector<Real> one_scaling;

  // Reset the scaling to all 1's so we can compute the true residual
  for(unsigned int var = 0; var < _sys.n_vars(); var++)
    one_scaling.push_back(1.0);
  setVarScaling(one_scaling);

  computeResidual(*_sys.rhs);

  // FIXME: fix this
//  _old_initial_residual_norm = _initial_residual_norm;
  Real _initial_residual_norm = _sys.rhs->l2_norm();

  std::cout<<"  True Initial Nonlinear Residual: "<<_initial_residual_norm<<std::endl;

  // FIXME: fix this
  // Set the scaling to manual scaling
//  setVarScaling(_manual_scaling);

//  if (_moose_system._auto_scaling)
//  {
//    std::vector<Real> scaling;
//
//    // Compute the new scaling
//    for(unsigned int var = 0; var < _moose_system.getNonlinearSystem()->n_vars(); var++)
//    {
//      Real norm = _moose_system.getNonlinearSystem()->calculate_norm(*_moose_system.getNonlinearSystem()->rhs,var,DISCRETE_L2);
//
//      if(norm != 0)
//        scaling.push_back(1.0/norm);
//      else
//        scaling.push_back(1.0);
//    }
//
//    _moose_system.setVarScaling(scaling);
//  }
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
NonlinearSystem::reinitDampers(const NumericVector<Number>& increment, THREAD_ID tid)
{
  *_increment_vec = increment;

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
  std::set<unsigned int> input_subdomains;

  bool global_kernels_exist = _kernels[0].subdomains_covered(input_subdomains);

  if (!global_kernels_exist)
  {
    std::set<unsigned int> difference;
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
NonlinearSystem::checkBCCoverage(const std::set<short> & mesh_bcs) const
{
  // Check that BCs used in your simulation exist in your mesh
  std::set<short> input_bcs, difference;
  
  _bcs[0].activeBoundaries(input_bcs);  // get the boundaries from the simulation (input file)
  std::set_difference (input_bcs.begin(), input_bcs.end(),
                       mesh_bcs.begin(), mesh_bcs.end(),
                       std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::stringstream extra_boundary_ids;
    
    std::copy (difference.begin(), difference.end(), std::ostream_iterator<unsigned short>( extra_boundary_ids, " "));
    
    mooseError("The following boundary ids from your input file do not exist in the input mesh "
               + extra_boundary_ids.str());
  } 
}
