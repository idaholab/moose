#include "ImplicitSystem.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "SubProblem.h"
#include "Variable.h"
#include "PetscSupport.h"
#include "Factory.h"
#include "ParallelUniqueId.h"
#include "ThreadedElementLoop.h"
#include "MaterialData.h"

#include "nonlinear_solver.h"
#include "quadrature_gauss.h"
#include "dense_vector.h"
#include "boundary_info.h"
#include "petsc_matrix.h"

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

  // threads ////////////////////////////

  class ComputeResidualThread : public ThreadedElementLoop<ConstElemRange>
  {
  protected:
    NumericVector<Number> & _residual;
    ImplicitSystem & _sys;

  public:
    ComputeResidualThread(NumericVector<Number> & residual, ImplicitSystem & sys) :
      ThreadedElementLoop<ConstElemRange>(sys),
      _residual(residual),
      _sys(sys)
    {
    }

    // Splitting Constructor
    ComputeResidualThread(ComputeResidualThread & x, Threads::split split) :
      ThreadedElementLoop<ConstElemRange>(x, split),
      _residual(x._residual),
      _sys(x._sys)
    {
    }

    virtual void onElement(const Elem *elem)
    {
      unsigned int cur_subdomain = elem->subdomain_id();

      _problem.reinitElem(elem, _tid);
      _problem.reinitMaterials(cur_subdomain, _tid);

      KernelIterator kernel_begin = _sys._kernels[_tid].activeKernelsBegin();
      KernelIterator kernel_end = _sys._kernels[_tid].activeKernelsEnd();
      KernelIterator kernel_it = kernel_begin;
      for (kernel_it = kernel_begin; kernel_it != kernel_end; ++kernel_it)
        (*kernel_it)->computeResidual();

      for (std::vector<Variable *>::iterator it = _sys._vars[_tid].all().begin(); it != _sys._vars[_tid].all().end(); ++it)
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        (*it)->add(_residual);
      }
    }

    virtual void onDomainChanged(short int subdomain)
    {
      _problem.subdomainSetup(subdomain, _tid);
      _sys._kernels[_tid].updateActiveKernels(_problem.time(), _problem.dt(), subdomain);
    }

    virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
    {
      _problem.reinitElemFace(elem, side, bnd_id, _tid);
      _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
      for (std::vector<IntegratedBC *>::iterator it = _sys._bcs[_tid].getBCs(bnd_id).begin(); it != _sys._bcs[_tid].getBCs(bnd_id).end(); ++it)
        (*it)->computeResidual();

      for (std::set<Variable *>::iterator it = _sys._vars[_tid].boundaryVars(bnd_id).begin(); it != _sys._vars[_tid].boundaryVars(bnd_id).end(); ++it)
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        (*it)->add(_residual);
      }
    }

    void join(const ComputeResidualThread & /*y*/)
    {
    }
  };

  // Jacobian ////////

  class ComputeJacobianThread : public ThreadedElementLoop<ConstElemRange>
  {
  protected:
    SparseMatrix<Number> & _jacobian;
    ImplicitSystem & _sys;
    Problem & _problem;

  public:
    ComputeJacobianThread(SparseMatrix<Number> & jacobian, ImplicitSystem & sys) :
      ThreadedElementLoop<ConstElemRange>(sys),
      _jacobian(jacobian),
      _sys(sys),
      _problem(sys.problem())
    {
    }

    // Splitting Constructor
    ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split) :
      ThreadedElementLoop<ConstElemRange>(x, split),
      _jacobian(x._jacobian),
      _sys(x._sys),
      _problem(x._problem)
    {
    }

    virtual void onElement(const Elem *elem)
    {
      unsigned int cur_subdomain = elem->subdomain_id();

      _problem.reinitElem(elem, _tid);
      _problem.reinitMaterials(cur_subdomain, _tid);

      KernelIterator kernel_begin = _sys._kernels[_tid].activeKernelsBegin();
      KernelIterator kernel_end = _sys._kernels[_tid].activeKernelsEnd();
      KernelIterator kernel_it = kernel_begin;
      for (kernel_it = kernel_begin; kernel_it != kernel_end; ++kernel_it)
        (*kernel_it)->computeJacobian(0, 0);

      for (std::vector<Variable *>::iterator it = _sys._vars[_tid].all().begin(); it != _sys._vars[_tid].all().end(); ++it)
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        (*it)->add(_jacobian);
      }
    }

    virtual void onDomainChanged(short int subdomain)
    {
      _problem.subdomainSetup(subdomain, _tid);
      _sys._kernels[_tid].updateActiveKernels(_problem.time(), _problem.dt(), subdomain);
    }

    virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
    {
      _problem.reinitElemFace(elem, side, bnd_id, _tid);
      _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);
      for (std::vector<IntegratedBC *>::iterator it = _sys._bcs[_tid].getBCs(bnd_id).begin(); it != _sys._bcs[_tid].getBCs(bnd_id).end(); ++it)
        (*it)->computeJacobian(0, 0);

      for (std::set<Variable *>::iterator it = _sys._vars[_tid].boundaryVars(bnd_id).begin(); it != _sys._vars[_tid].boundaryVars(bnd_id).end(); ++it)
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        (*it)->add(_jacobian);
      }
    }

    void join(const ComputeJacobianThread & /*y*/)
    {
    }
  };


ImplicitSystem::ImplicitSystem(SubProblem & problem, const std::string & name) :
    SystemTempl<TransientNonlinearImplicitSystem>(problem, name),
    _last_rnorm(0),
    _l_abs_step_tol(1e-10),
    _initial_residual(0),
    _dt(problem.dt()),
    _dt_old(problem.dtOld()),
    _t_step(problem.timeStep())
{
  _sys.nonlinear_solver->residual = Moose::compute_residual;
  _sys.nonlinear_solver->jacobian = Moose::compute_jacobian;

  _sys.attach_init_function(Moose::initial_condition);

  _time_weight.resize(3);
  timeSteppingScheme(IMPLICIT_EULER);                   // default time stepping scheme

  _kernels.resize(libMesh::n_threads());
  _bcs.resize(libMesh::n_threads());
}

ImplicitSystem::~ImplicitSystem()
{
}

bool
ImplicitSystem::converged()
{
  return _sys.nonlinear_solver->converged;
}

void
ImplicitSystem::addKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<System *>("_sys") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _problem._material_data[tid];

    Kernel *kernel = static_cast<Kernel *>(Factory::instance()->create(kernel_name, name, parameters));
    mooseAssert(kernel != NULL, "Not a Kernel object");

    std::set<unsigned int> blk_ids;
    if (!parameters.isParamValid("block"))
      blk_ids = _var_map[&kernel->variable()];
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
      for (unsigned int i=0; i<blocks.size(); ++i)
      {
        if (_var_map[&kernel->variable()].count(blocks[i]) > 0 || _var_map[&kernel->variable()].count(Moose::ANY_BLOCK_ID) > 0)
          blk_ids.insert(blocks[i]);
        else
          mooseError("Kernel (" + kernel->name() + "): block outside of the domain of the variable");
      }
    }

    _kernels[tid].addKernel(kernel, blk_ids);
  }
}

void
ImplicitSystem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<System *>("_sys") = this;
  std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

  for (unsigned int i=0; i<boundaries.size(); ++i)
  {
    parameters.set<unsigned int>("_boundary_id") = boundaries[i];
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      parameters.set<THREAD_ID>("_tid") = tid;
      parameters.set<MaterialData *>("_material_data") = _problem._bnd_material_data[tid];

      BoundaryCondition * bc = static_cast<BoundaryCondition *>(Factory::instance()->create(bc_name, name, parameters));
      mooseAssert(bc != NULL, "Not a BoundaryCondition object");

      if (dynamic_cast<NodalBC *>(bc) != NULL)
        _bcs[tid].addNodalBC(boundaries[i], dynamic_cast<NodalBC *>(bc));
      else if (dynamic_cast<IntegratedBC *>(bc) != NULL)
        _bcs[tid].addBC(boundaries[i], dynamic_cast<IntegratedBC *>(bc));
      else
        mooseError("Unknown type of BoudaryCondition object");
      _vars[tid].addBoundaryVar(boundaries[i], &bc->variable());
      _vars[tid].addBoundaryVars(boundaries[i], bc->getCoupledVars());
    }
  }
}

void
ImplicitSystem::computeResidual(NumericVector<Number> & residual)
{
  computeTimeDeriv();
  computeResidualInternal(residual);
  finishResidual(residual);
}

void
ImplicitSystem::timeSteppingScheme(TimeSteppingScheme scheme)
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
ImplicitSystem::onTimestepBegin()
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

    solution(solutionOld());                    // use old_solution for computing with correct solution vector
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
ImplicitSystem::computeTimeDeriv()
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
ImplicitSystem::computeResidualInternal(NumericVector<Number> & residual)
{
  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

  ComputeResidualThread cr(residual, *this);
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
ImplicitSystem::finishResidual(NumericVector<Number> & residual)
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
ImplicitSystem::computeJacobian(SparseMatrix<Number> & jacobian)
{
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

  ComputeJacobianThread cj(jacobian, *this);
  Threads::parallel_reduce(elem_range, cj);
  jacobian.close();

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
}

void
ImplicitSystem::printVarNorms()
{
  TransientNonlinearImplicitSystem &s = static_cast<TransientNonlinearImplicitSystem &>(_sys);

  std::cout << "Norm of each nonlinear variable:" << std::endl;
  for (std::vector<Variable *>::iterator it = _vars[0].all().begin(); it != _vars[0].all().end(); ++it)
  {
    Variable *var = *it;
    unsigned int var_num = var->number();
    std::cout << s.variable_name(var_num) << ": "
              << s.calculate_norm(*s.rhs,var_num,DISCRETE_L2) << std::endl;

  }
}

} // namespace
