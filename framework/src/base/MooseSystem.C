#include "MooseSystem.h"
#include "ElementData.h"
#include "FaceData.h"
#include "AuxData.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "MaterialFactory.h"
#include "StabilizerFactory.h"
#include "InitialConditionFactory.h"
#include "AuxKernel.h"
#include "ParallelUniqueId.h"

//libMesh includes
#include "numeric_vector.h"
#include "exodusII_io.h"
#include "parallel.h"
#include "gmv_io.h"
#include "tecplot_io.h"
#include "boundary_info.h"

// FIXME: remove me when libmesh solver problem is fixed
namespace Moose {
void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual);
void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian);
void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar);
}

MooseSystem::MooseSystem()
  :_es(NULL),
   _system(NULL),
   _aux_system(NULL),
   _mesh(NULL),
   _delete_mesh(true),
   _dim(0),
   _mesh_changed(false),
   _exreader(NULL),
   _is_valid(false),
   _kernels(*this),
   _bcs(*this),
   _materials(*this),
   _stabilizers(*this),
   _auxs(*this),
   _ics(*this),
   _no_fe_reinit(false),
   _preconditioner(NULL)
{
  Moose::g_system = this;     // FIXME: this will eventually go away
  sizeEverything();
}

MooseSystem::MooseSystem(Mesh &mesh)
  : _mesh(&mesh),
    _delete_mesh(false),
    _dim(_mesh->mesh_dimension()),
    _mesh_changed(false),
    _exreader(NULL),
    _is_valid(false),
    _kernels(*this),
    _bcs(*this),
    _materials(*this),
    _stabilizers(*this),
    _auxs(*this),
    _ics(*this),
    _no_fe_reinit(false),
    _preconditioner(NULL)
{
  Moose::g_system = this;     // FIXME: this will eventually go away

  sizeEverything();
  initEquationSystems();
  // setup solver
  _system->nonlinear_solver->residual = Moose::compute_residual;
  _system->nonlinear_solver->jacobian = Moose::compute_jacobian;

  _mesh->prepare_for_use(false);
  _mesh->boundary_info->build_node_list_from_side_list();

  initDataStructures();
}

MooseSystem::~MooseSystem()
{
  if (_is_valid)
    for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid) 
    {
      delete _element_data[tid];
      delete _face_data[tid];
      delete _aux_data[tid];
    }

  if (_es != NULL)
    delete _es;

  if (_delete_mesh && _mesh != NULL)
    delete _mesh;
}

Mesh *
MooseSystem::initMesh(unsigned int dim) 
{
  if (_mesh != NULL)
    mooseError("Mesh already initialized for this MooseSystem");

  _dim = dim;
  _mesh = new Mesh(dim);
  return _mesh;
}

Mesh *
MooseSystem::getMesh() 
{
  checkValid();
  return _mesh;
}

void
MooseSystem::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  // Kernels::sizeEverythin
  _bdf2_wei.resize(3);
  _current_elem.resize(n_threads);
  _dof_indices.resize(n_threads);
  _aux_dof_indices.resize(n_threads);
  _fe.resize(n_threads);
  _qrule.resize(n_threads);

  _JxW.resize(n_threads);
  _phi.resize(n_threads);
  _test.resize(n_threads);
  _dphi.resize(n_threads);
  _d2phi.resize(n_threads);
  _q_point.resize(n_threads);
  _var_dof_indices.resize(n_threads);
  _aux_var_dof_indices.resize(n_threads);
  _var_Res.resize(n_threads);
  _var_Kes.resize(n_threads);
  _var_vals.resize(n_threads);
  _var_grads.resize(n_threads);
  _var_seconds.resize(n_threads);
  _var_vals_old.resize(n_threads);
  _var_vals_older.resize(n_threads);
  _var_grads_old.resize(n_threads);
  _var_grads_older.resize(n_threads);
  _aux_var_vals.resize(n_threads);
  _aux_var_grads.resize(n_threads);
  _aux_var_vals_old.resize(n_threads);
  _aux_var_vals_older.resize(n_threads);
  _aux_var_grads_old.resize(n_threads);
  _aux_var_grads_older.resize(n_threads);

  _material.resize(n_threads);
  _real_zero.resize(n_threads);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);

  // bcs::sizeEverything
  _current_node.resize(n_threads);
  _current_residual.resize(n_threads);
  _current_side.resize(n_threads);
  _fe_face.resize(n_threads);
  _qface.resize(n_threads);
  _q_point_face.resize(n_threads);
  _JxW_face.resize(n_threads);
  _phi_face.resize(n_threads);
  _dphi_face.resize(n_threads);
  _d2phi_face.resize(n_threads);
  _normals_face.resize(n_threads);

  _nodal_bc_var_dofs.resize(n_threads);
  _var_vals_face.resize(n_threads);
  _var_grads_face.resize(n_threads);
  _var_seconds_face.resize(n_threads);
  _var_vals_face_nodal.resize(n_threads);

  // AuxKernels::sizeEverything
  _current_node.resize(n_threads);

  _var_vals_nodal.resize(n_threads);
  _var_vals_old_nodal.resize(n_threads);
  _var_vals_older_nodal.resize(n_threads);

  _aux_var_dofs.resize(n_threads);
  _aux_var_vals_nodal.resize(n_threads);
  _aux_var_vals_old_nodal.resize(n_threads);
  _aux_var_vals_older_nodal.resize(n_threads);

  _var_vals_element.resize(n_threads);
  _var_vals_old_element.resize(n_threads);
  _var_vals_older_element.resize(n_threads);
  _var_grads_element.resize(n_threads);
  _var_grads_old_element.resize(n_threads);
  _var_grads_older_element.resize(n_threads);
  _aux_var_vals_element.resize(n_threads);
  _aux_var_vals_old_element.resize(n_threads);
  _aux_var_vals_older_element.resize(n_threads);
  _aux_var_grads_element.resize(n_threads);
  _aux_var_grads_old_element.resize(n_threads);
  _aux_var_grads_older_element.resize(n_threads);
}

void
MooseSystem::init()
{
  if (_mesh == NULL)
    mooseError("Mesh is not set.");

  _es->init();

  _dof_map = &_system->get_dof_map();
  _aux_dof_map = &_aux_system->get_dof_map();

  unsigned int n_vars = _system->n_vars();
  unsigned int n_aux_vars = _aux_system->n_vars();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    // kernels
    _var_dof_indices[tid].resize(n_vars);
    _var_Res[tid].resize(n_vars);
    _var_Kes[tid].resize(n_vars);
    _var_vals[tid].resize(n_vars);
    _var_grads[tid].resize(n_vars);
    _var_seconds[tid].resize(n_vars);
    _var_vals_old[tid].resize(n_vars);
    _var_vals_older[tid].resize(n_vars);
    _var_grads_old[tid].resize(n_vars);
    _var_grads_older[tid].resize(n_vars);

    // aux var
    _aux_var_dof_indices[tid].resize(n_aux_vars);
    _aux_var_vals[tid].resize(n_aux_vars);
    _aux_var_grads[tid].resize(n_aux_vars);
    _aux_var_vals_old[tid].resize(n_aux_vars);
    _aux_var_vals_older[tid].resize(n_aux_vars);
    _aux_var_grads_old[tid].resize(n_aux_vars);
    _aux_var_grads_older[tid].resize(n_aux_vars);
  }

  // Kernels
  _max_quadrature_order = CONSTANT;

  //Set the default variable scaling to 1
  for(unsigned int i=0; i < _system->n_vars(); i++)
    _scaling_factor.push_back(1.0);

  //Find the largest quadrature order necessary... all variables _must_ use the same rule!
  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);
    if(fe_type.default_quadrature_order() > _max_quadrature_order)
      _max_quadrature_order = fe_type.default_quadrature_order();
  }

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _qrule[tid] = new QGauss(_dim, _max_quadrature_order);

  initKernels();
  initBCs();
  initAuxKernels();

  _t = 0;
  _dt = 0;
  _is_transient = false;
  _n_of_rk_stages = 1;
  _t_scheme = 0;
  _t_step       = 0;
  _dt_old       = _dt;
  _bdf2_wei[0]  = 1.;
  _bdf2_wei[1]  =-1.;
  _bdf2_wei[2]  = 0.;
}

void
MooseSystem::initKernels()
{
  //This allows for different basis functions / orders for each variable
  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(_dim, fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_qrule[tid]);

        _JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _d2phi[tid][fe_type] = &_fe[tid][fe_type]->get_d2phi();
      }
    }
  }

  //This allows for different basis functions / orders for each Aux variable
  for(unsigned int var=0; var < _aux_system->n_vars(); var++)
  {
    FEType fe_type = _aux_dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(_dim, fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_qrule[tid]);

        _JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();
      }
    }
  }
}

void
MooseSystem::initBCs()
{
  unsigned int n_vars = _system->n_vars();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    // BCs
    _boundary_to_var_nums[tid].resize(n_vars);
    _boundary_to_var_nums_nodal[tid].resize(n_vars);
    _nodal_bc_var_dofs[tid].resize(n_vars);
    _var_vals_face[tid].resize(n_vars);
    _var_grads_face[tid].resize(n_vars);
    _var_seconds_face[tid].resize(n_vars);
    _var_vals_face_nodal[tid].resize(n_vars);
  }

  //Max quadrature order was already found by Kernel::init()
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _qface[tid] = new QGauss(_dim-1,_max_quadrature_order);

  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe_face[tid][fe_type])
      {
        _fe_face[tid][fe_type] = FEBase::build(_dim, fe_type).release();
        _fe_face[tid][fe_type]->attach_quadrature_rule(_qface[tid]);

        _q_point_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_xyz();
        _JxW_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_JxW();
        _phi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_phi();
        _dphi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_dphi();
        _normals_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_normals();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _d2phi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_d2phi();
      }
    }
  }
}

void
MooseSystem::initAuxKernels()
{
  _nonlinear_old_soln = _system->old_local_solution.get();
  _nonlinear_older_soln = _system->older_local_solution.get();

  _aux_soln = _aux_system->solution.get();
  _aux_old_soln = _aux_system->old_local_solution.get();
  _aux_older_soln = _aux_system->older_local_solution.get();

  unsigned int n_vars = _system->n_vars();
  unsigned int n_aux_vars = _aux_system->n_vars();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _var_vals_nodal[tid].resize(n_vars);
    _var_vals_old_nodal[tid].resize(n_vars);
    _var_vals_older_nodal[tid].resize(n_vars);

    _aux_var_dofs[tid].resize(n_aux_vars);
    _aux_var_vals_nodal[tid].resize(n_aux_vars);
    _aux_var_vals_old_nodal[tid].resize(n_aux_vars);
    _aux_var_vals_older_nodal[tid].resize(n_aux_vars);

    _var_vals_element[tid].resize(n_vars);
    _var_vals_old_element[tid].resize(n_vars);
    _var_vals_older_element[tid].resize(n_vars);
    _var_grads_element[tid].resize(n_vars);
    _var_grads_old_element[tid].resize(n_vars);
    _var_grads_older_element[tid].resize(n_vars);

    _aux_var_vals_element[tid].resize(n_aux_vars);
    _aux_var_vals_old_element[tid].resize(n_aux_vars);
    _aux_var_vals_older_element[tid].resize(n_aux_vars);
    _aux_var_grads_element[tid].resize(n_aux_vars);
    _aux_var_grads_old_element[tid].resize(n_aux_vars);
    _aux_var_grads_older_element[tid].resize(n_aux_vars);
  }
}

EquationSystems *
MooseSystem::getEquationSystems()
{
//  checkValid();
  return _es;
}

TransientNonlinearImplicitSystem *
MooseSystem::getNonlinearSystem()
{
  checkValid();
  return _system;
}

TransientExplicitSystem *
MooseSystem::getAuxSystem()
{
  checkValid();
  return _aux_system;
}

EquationSystems *
MooseSystem::initEquationSystems()
{
  if (_es != NULL)
    mooseError("EquationSystems Object already initialized for this MooseSystem");
  
  _es = new EquationSystems(*_mesh);
  _system = &_es->add_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  _aux_system = &_es->add_system<TransientExplicitSystem>("AuxiliarySystem");
  
  return _es;
}

void
MooseSystem::initDataStructures()
{
  if (_mesh == NULL)
    mooseError("Mesh is uninitialized in call to initialize data structures");
  if (_es == NULL)
    mooseError("EquationsSystems is uninitialized in call to initialize data structures");

  unsigned int n_threads = libMesh::n_threads();
  _element_data.resize(n_threads);
  _face_data.resize(n_threads);
  _aux_data.resize(n_threads);

  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _element_data[tid] = new ElementData(*this);
    _face_data[tid] = new FaceData(*this);
    _aux_data[tid] = new AuxData(*this, *_element_data[tid]);
  }

  // Need to initialize data
  _is_valid = true;
}

void
MooseSystem::checkValid()
{
  if (!_is_valid)
    mooseError("MooseSystem has not been properly initialized before accessing data");
}

ElementData *
MooseSystem::getElementData(THREAD_ID tid)
{
  checkValid();
  return _element_data[tid];
}

FaceData *
MooseSystem::getFaceData(THREAD_ID tid)
{
  checkValid();
  return _face_data[tid];
}

AuxData *
MooseSystem::getAuxData(THREAD_ID tid)
{
  checkValid();
  return _aux_data[tid];
}

ExodusII_IO *
MooseSystem::getExodusReader()
{
  if(!_exreader)
    _exreader = new ExodusII_IO(*_mesh);

  return _exreader;
}

void
MooseSystem::solve()
{
  _system->solve();
}

unsigned int
MooseSystem::addVariable(const std::string &var, const FEType  &type, const std::set< subdomain_id_type  > *const active_subdomains)
{
  unsigned int var_num = _system->add_variable(var, type, active_subdomains);
  _var_nums.push_back(var_num);
  return var_num;
}

unsigned int
MooseSystem::addVariable(const std::string &var, const Order order, const FEFamily family, const std::set< subdomain_id_type > *const active_subdomains)
{
  unsigned int var_num = _system->add_variable(var, order, family, active_subdomains);
  _var_nums.push_back(var_num);
  return var_num;
}

// Kernels ////

void
MooseSystem::addKernel(std::string kernel_name,
                       std::string name,
                       InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    Kernel * kernel = KernelFactory::instance()->create(kernel_name, name, *this, parameters);

    _kernels.all_kernels[tid].push_back(kernel);
  }
}

void MooseSystem::addKernel(std::string kernel_name,
                            std::string name,
                            InputParameters parameters,
                            unsigned int block_id)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    Kernel * kernel = KernelFactory::instance()->create(kernel_name, name, *this, parameters);

    _kernels.all_block_kernels[tid][block_id].push_back(kernel);
  }

}

void
MooseSystem::addBC(std::string bc_name,
                   std::string name,
                   InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;
    std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

    for (unsigned int i=0; i<boundaries.size(); ++i)
    {
      parameters.set<unsigned int>("_boundary_id") = boundaries[i];
      BoundaryCondition * bc = BCFactory::instance()->create(bc_name, name, *this, parameters);

      if(bc->isIntegrated())
        _bcs.active_bcs[tid][boundaries[i]].push_back(bc);
      else
        _bcs.active_nodal_bcs[tid][boundaries[i]].push_back(bc);
    }
  }
}


void
MooseSystem::addAuxKernel(std::string aux_name,
                          std::string name,
                          InputParameters parameters)
{
  AuxKernel * aux;
  AuxKernelIterator curr_aux, end_aux;
  unsigned int size;
  std::string var_name = parameters.get<std::string>("variable");
  std::vector<std::string> coupled_to = parameters.get<std::vector<std::string> >("coupled_to");

  std::vector<std::list<AuxKernel *>::iterator > dependent_auxs;
  std::vector<AuxKernel *> *aux_ptr;
  std::list<AuxKernel *>::iterator new_aux_iter;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    aux = AuxFactory::instance()->create(aux_name, name, *this, parameters);

    if (aux->isNodal())
      aux_ptr = &_auxs.active_NodalAuxKernels[tid];
    else
      aux_ptr = &_auxs.active_ElementAuxKernels[tid];

    // Copy the active AuxKernels into a list for manipulation
    std::list<AuxKernel *> active_auxs(aux_ptr->begin(), aux_ptr->end());

    // Get a list of all the dependent variables that this AuxKernel will act on to
    // place it in the vector in the appropriate location
    for (std::list<AuxKernel *>::iterator i=active_auxs.begin(); i != active_auxs.end(); ++i)
      for (std::vector<std::string>::iterator j=coupled_to.begin(); j != coupled_to.end(); ++j)
        if ((*i)->varName() == *j)
          dependent_auxs.push_back(i);

    // Insert the AuxKernel preserving its dependents (last iterator in dependent_auxs)
    if (dependent_auxs.empty())
    {
      active_auxs.push_front(aux);
      new_aux_iter = active_auxs.begin();
    }
    else
      new_aux_iter = active_auxs.insert(++(*dependent_auxs.rbegin()), aux);

    // Now check to see if any of the existing AuxKernels depend on this newly inserted kernel
    dependent_auxs.clear();
    for (std::list<AuxKernel *>::iterator i=active_auxs.begin(); i != new_aux_iter; ++i)
    {
      const std::vector<std::string> & curr_coupled = (*i)->coupledTo();
      for (std::vector<std::string>::const_iterator j=curr_coupled.begin(); j != curr_coupled.end(); ++j)
        if (var_name == *j)
          dependent_auxs.push_back(i);
    }

    // Move the dependent items to the point after the insertion of this aux kernel
    ++new_aux_iter;
    for (std::vector<std::list<AuxKernel *>::iterator >::iterator i=dependent_auxs.begin();
         i != dependent_auxs.end(); ++i)
      active_auxs.splice(new_aux_iter, active_auxs, *i);

    // Copy the list back into the Auxilary Vector
    aux_ptr->assign(active_auxs.begin(), active_auxs.end());
  }
}

void
MooseSystem::addAuxBC(std::string aux_name,
                      std::string name,
                      InputParameters parameters)
{
  AuxKernel * aux;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;
    std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

    aux = AuxFactory::instance()->create(aux_name, name, *this, parameters);

    for (unsigned int i=0; i<boundaries.size(); ++i)
      _auxs.active_bcs[tid][boundaries[i]].push_back(aux);
  }
}

void
MooseSystem::addMaterial(std::string mat_name,
                         std::string name,
                         InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;
    std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");

    // TODO: Remove this hack when Material no longer inherits from Kernel!
    parameters.set<std::string>("variable") = _es->get_system(0).variable_name(0);

    for (unsigned int i=0; i<blocks.size(); ++i)
      _materials.active_materials[tid][blocks[i]] = MaterialFactory::instance()->create(mat_name, name, *this, parameters);
  }
}

void
MooseSystem::addStabilizer(std::string stabilizer_name,
                           std::string name,
                           InputParameters parameters)
{
  Stabilizer * stabilizer;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    stabilizer = StabilizerFactory::instance()->create(stabilizer_name, name, *this, parameters);

    if (parameters.have_parameter<unsigned int>("block_id"))
      _stabilizers.block_stabilizers[tid][parameters.get<unsigned int>("block_id")][stabilizer->variable()] = stabilizer;
    else
      _stabilizers.active_stabilizers[tid][stabilizer->variable()] = stabilizer;

    _stabilizers._is_stabilized[stabilizer->variable()] = true;
  }
}

void
MooseSystem::addInitialCondition(std::string ic_name,
                                 std::string name,
                                 InputParameters parameters,
                                 std::string var_name)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    // The var_name needs to be added to the parameters object for any InitialCondition derived objects
    parameters.set<std::string>("var_name") = var_name;

    _ics.active_initial_conditions[tid][var_name] = InitialConditionFactory::instance()->create(ic_name, name, *this, parameters);
  }
}

void
MooseSystem::computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi)
{
  u=0;

  unsigned int phi_size = phi.size();

  //All of this stuff so that the loop will vectorize
  std::vector<Real> sol_vals(phi_size);
  std::vector<Real> phi_vals(phi_size);

  for (unsigned int i=0; i<phi_size; i++)
  {
    sol_vals[i] = soln(dof_indices[i]);
    phi_vals[i] = phi[i][qp];
  }

  for (unsigned int i=0; i<phi_size; i++)
  {
    u +=  phi_vals[i]*sol_vals[i];
  }
}

void
MooseSystem::computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                             std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                             std::vector<RealTensor> &second_u,
                             const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp] = 0;
    u_old[qp] = 0;
    u_older[qp] = 0;

    grad_u[qp] = 0;
    grad_u_old[qp] = 0;
    grad_u_older[qp] = 0;

    second_u[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    Real soln_old_local   = soln_old(indx);
    Real soln_older_local = soln_older(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      u_old[qp]    += phi_local*soln_old_local;
      u_older[qp]  += phi_local*soln_older_local;

      grad_u[qp]       += dphi_local*soln_local;
      grad_u_old[qp]   += dphi_local*soln_old_local;
      grad_u_older[qp] += dphi_local*soln_older_local;

      second_u[qp] +=d2phi[i][qp]*soln_local;
    }

  }
}

void
MooseSystem::computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                             std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                             const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                             const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                             const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi)
{

  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    u_old[qp]=0;
    u_older[qp] = 0;

    grad_u[qp] = 0;
    grad_u_old[qp] = 0;
    grad_u_older[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);
    Real soln_old_local   = soln_old(indx);
    Real soln_older_local = soln_older(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      u_old[qp]    += phi_local*soln_old_local;
      u_older[qp]  += phi_local*soln_older_local;

      grad_u[qp]       += dphi_local*soln_local;
      grad_u_old[qp]   += dphi_local*soln_old_local;
      grad_u_older[qp] += dphi_local*soln_older_local;
    }

  }
}

void
MooseSystem::computeQpSolutionAll(std::vector<Real> & u,
                                  std::vector<RealGradient> &grad_u,
                                  std::vector<RealTensor> &second_u,
                                  const NumericVector<Number> & soln,
                                  const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                                  const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    grad_u[qp] = 0;
    second_u[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local       = soln(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      grad_u[qp]   += dphi_local*soln_local;
      second_u[qp] += d2phi[i][qp]*soln_local;
    }

  }
}


void
MooseSystem::computeQpSolutionAll(std::vector<Real> & u,
                                  std::vector<RealGradient> &grad_u,
                                  const NumericVector<Number> & soln,
                                  const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                                  const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi)
{
  for (unsigned int qp =0;qp<n_qp;qp++)
  {
    u[qp]=0;
    grad_u[qp] = 0;
  }

  unsigned int phi_size = phi.size();

  for (unsigned int i=0; i<phi_size; i++)
  {
    int indx = dof_indices[i];
    Real soln_local = soln(indx);

    for (unsigned int qp =0; qp<n_qp; qp++)
    {
      Real phi_local = phi[i][qp];
      RealGradient dphi_local = dphi[i][qp];

      u[qp]        += phi_local*soln_local;
      grad_u[qp]   += dphi_local*soln_local;
    }

  }
}

void
MooseSystem::computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi)
{
  grad_u=0;

  unsigned int dphi_size = dphi.size();

  for (unsigned int i=0; i<dphi_size; i++)
  {
    grad_u += dphi[i][qp]*soln(dof_indices[i]);
  }
}

void
MooseSystem::computeQpSecondSolution(RealTensor & second_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealTensor> > & d2phi)
{
  second_u=0;

  unsigned int d2phi_size = d2phi.size();

  for (unsigned int i=0; i<d2phi_size; i++)
  {
    second_u += d2phi[i][qp]*soln(dof_indices[i]);
  }
}

void
MooseSystem::reinitKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
//  Moose::perf_log.push("reinit()","Kernel");
//  Moose::perf_log.push("reinit() - dof_indices","Kernel");

  _current_elem[tid] = elem;

  _dof_map->dof_indices(elem, _dof_indices[tid]);

  std::map<FEType, FEBase*>::iterator fe_it = _fe[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe[tid].end();

//  Moose::perf_log.pop("reinit() - dof_indices","Kernel");
//  Moose::perf_log.push("reinit() - fereinit","Kernel");

  static std::vector<bool> first(libMesh::n_threads(), true);

  if(_no_fe_reinit)
  {
    if(first[tid])
    {
      for(;fe_it != fe_end; ++fe_it)
        fe_it->second->reinit(elem);
    }
  }
  else
  {
    for(;fe_it != fe_end; ++fe_it)
      fe_it->second->reinit(elem);
  }

  first[tid] = false;

//  Moose::perf_log.pop("reinit() - fereinit","Kernel");

//  Moose::perf_log.push("reinit() - resizing","Kernel");
  if(Re)
    Re->resize(_dof_indices[tid].size());

  if(Ke)
    Ke->resize(_dof_indices[tid].size(),_dof_indices[tid].size());

  unsigned int position = 0;

  for(unsigned int i=0; i<_var_nums.size();i++)
  {
    _dof_map->dof_indices(elem, _var_dof_indices[tid][i], i);

    unsigned int num_dofs = _var_dof_indices[tid][i].size();

    if(Re)
    {
      if(_var_Res[tid][i])
        delete _var_Res[tid][i];

      _var_Res[tid][i] = new DenseSubVector<Number>(*Re,position, num_dofs);
    }

    if(Ke)
    {
      if(_var_Kes[tid][i])
        delete _var_Kes[tid][i];

      _var_Kes[tid][i] = new DenseMatrix<Number>(num_dofs,num_dofs);
    }
    position+=num_dofs;
  }

  unsigned int num_q_points = _qrule[tid]->n_points();

  _real_zero[tid] = 0;
  _zero[tid].resize(num_q_points,0);
  _grad_zero[tid].resize(num_q_points,0);
  _second_zero[tid].resize(num_q_points,0);

  std::vector<unsigned int>::iterator var_num_it = _var_nums.begin();
  std::vector<unsigned int>::iterator var_num_end = _var_nums.end();

//  Moose::perf_log.pop("reinit() - resizing","Kernel");
//  Moose::perf_log.push("reinit() - compute vals","Kernel");

  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    unsigned int num_dofs = _var_dof_indices[tid][var_num].size();

    _var_vals[tid][var_num].resize(num_q_points);
    _var_grads[tid][var_num].resize(num_q_points);

    if(has_second_derivatives)
      _var_seconds[tid][var_num].resize(num_q_points);

    if(_is_transient)
    {
      _var_vals_old[tid][var_num].resize(num_q_points);
      _var_grads_old[tid][var_num].resize(num_q_points);

      _var_vals_older[tid][var_num].resize(num_q_points);
      _var_grads_older[tid][var_num].resize(num_q_points);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_dphi[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi= *_d2phi[tid][fe_type];

    // Copy phi to the test functions.
    _test[tid][var_num] = static_phi;

    if (_is_transient)
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             _var_seconds[tid][var_num],
                             soln, *_system->old_local_solution, *_system->older_local_solution,
                             _var_dof_indices[tid][var_num], _qrule[tid]->n_points(),
                             static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_vals_old[tid][var_num], _var_vals_older[tid][var_num],
                             _var_grads[tid][var_num], _var_grads_old[tid][var_num], _var_grads_older[tid][var_num],
                             soln, *_system->old_local_solution, *_system->older_local_solution,
                             _var_dof_indices[tid][var_num], _qrule[tid]->n_points(),
                             static_phi, static_dphi);
    }
    else
    {
      if( has_second_derivatives )
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],  _var_seconds[tid][var_num],
                             soln, _var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi, static_d2phi);
      else
        computeQpSolutionAll(_var_vals[tid][var_num], _var_grads[tid][var_num],
                             soln, _var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi);
    }
  }

//  Moose::perf_log.pop("reinit() - compute vals","Kernel");

//  Moose::perf_log.push("reinit() - compute aux vals","Kernel");
  const NumericVector<Number>& aux_soln = (*_aux_system->current_local_solution);

  std::vector<unsigned int>::iterator aux_var_num_it = _aux_var_nums.begin();
  std::vector<unsigned int>::iterator aux_var_num_end = _aux_var_nums.end();

  for(;aux_var_num_it != aux_var_num_end; ++aux_var_num_it)
  {
    unsigned int var_num = *aux_var_num_it;

    FEType fe_type = _aux_dof_map->variable_type(var_num);

    _aux_dof_map->dof_indices(elem, _aux_var_dof_indices[tid][var_num], var_num);

    unsigned int num_dofs = _aux_var_dof_indices[tid][var_num].size();

    _aux_var_vals[tid][var_num].resize(num_q_points);
    _aux_var_grads[tid][var_num].resize(num_q_points);

    if(_is_transient)
    {
      _aux_var_vals_old[tid][var_num].resize(num_q_points);
      _aux_var_grads_old[tid][var_num].resize(num_q_points);

      _aux_var_vals_older[tid][var_num].resize(num_q_points);
      _aux_var_grads_older[tid][var_num].resize(num_q_points);
    }

    const std::vector<std::vector<Real> > & static_phi = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi= *_dphi[tid][fe_type];


    if (_is_transient)
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_vals_old[tid][var_num], _aux_var_vals_older[tid][var_num],
                           _aux_var_grads[tid][var_num], _aux_var_grads_old[tid][var_num], _aux_var_grads_older[tid][var_num],
                           aux_soln, *_aux_system->old_local_solution, *_aux_system->older_local_solution,
                           _aux_var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi);
    }
    else
    {
      computeQpSolutionAll(_aux_var_vals[tid][var_num], _aux_var_grads[tid][var_num],
                           aux_soln, _aux_var_dof_indices[tid][var_num], _qrule[tid]->n_points(), static_phi, static_dphi);
    }
  }

//  Moose::perf_log.pop("reinit() - compute aux vals","Kernel");
//  Moose::perf_log.push("reinit() - material","Kernel");

  _material[tid] = _materials.getMaterial(tid,elem->subdomain_id());
  _material[tid]->materialReinit();

//  Moose::perf_log.pop("reinit() - material","Kernel");
//  Moose::perf_log.pop("reinit()","Kernel");
}

void MooseSystem::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id)
{
//  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side[tid] = side;

  std::map<FEType, FEBase*>::iterator fe_it = _fe_face[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe_face[tid].end();

  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(_current_elem[tid], _current_side[tid]);

  std::vector<unsigned int>::iterator var_nums_it = _boundary_to_var_nums[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_end = _boundary_to_var_nums[boundary_id].end();

  for(;var_nums_it != var_nums_end; ++var_nums_it)
  {
    unsigned int var_num = *var_nums_it;

    FEType fe_type = _dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[tid][var_num];

    _var_vals_face[tid][var_num].resize(_qface[tid]->n_points());
    _var_grads_face[tid][var_num].resize(_qface[tid]->n_points());

    if(has_second_derivatives)
      _var_seconds_face[tid][var_num].resize(_qface[tid]->n_points());

    const std::vector<std::vector<Real> > & static_phi_face = *_phi_face[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi_face= *_dphi_face[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi_face= *_d2phi_face[tid][fe_type];

    for (unsigned int qp=0; qp<_qface[tid]->n_points(); qp++)
    {
      computeQpSolution(_var_vals_face[tid][var_num][qp], soln, var_dof_indices, qp, static_phi_face);
      computeQpGradSolution(_var_grads_face[tid][var_num][qp], soln, var_dof_indices, qp, static_dphi_face);

      if(has_second_derivatives)
        computeQpSecondSolution(_var_seconds_face[tid][var_num][qp], soln, var_dof_indices, qp, static_d2phi_face);
    }
  }

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[tid][var_num];

    _var_vals_face_nodal[tid][var_num].resize(_current_elem[tid]->n_nodes());

    for(unsigned int i=0; i<_current_elem[tid]->n_nodes(); i++)
      _var_vals_face_nodal[tid][var_num][i] = soln(var_dof_indices[i]);
  }

//  Moose::perf_log.pop("reinit()","BoundaryCondition");
}

void MooseSystem::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
//  Moose::perf_log.push("reinit(node)","BoundaryCondition");

  _current_node[tid] = &node;

  _current_residual[tid] = &residual;

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  unsigned int nonlinear_system_number = _system->number();

  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _nodal_bc_var_dofs[tid][var_num] = dof_number;

    _var_vals_face_nodal[tid][var_num].resize(1);

    _var_vals_face_nodal[tid][var_num][0] = soln(dof_number);
  }

//  Moose::perf_log.pop("reinit(node)","BoundaryCondition");
}

void
MooseSystem::reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node)
{
  Moose::perf_log.push("reinit(node)","AuxKernel");

  _current_node[tid] = &node;

  unsigned int nonlinear_system_number = _system->number();
  unsigned int aux_system_number = _aux_system->number();

  //Non Aux vars first
  for(unsigned int i=0; i<_var_nums.size(); i++)
  {
    unsigned int var_num = _var_nums[i];

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _var_vals_nodal[tid][var_num] = soln(dof_number);

    if(_is_transient)
    {
      _var_vals_old_nodal[tid][var_num] = (*_nonlinear_old_soln)(dof_number);
      _var_vals_older_nodal[tid][var_num] = (*_nonlinear_older_soln)(dof_number);
    }
  }

  const NumericVector<Number>& aux_soln = *_aux_system->solution;
  const NumericVector<Number>& aux_old_soln = *_aux_system->old_local_solution;
  const NumericVector<Number>& aux_older_soln = *_aux_system->older_local_solution;

  //Now Nodal Aux vars
  for(unsigned int i=0; i<_nodal_var_nums.size(); i++)
  {
    unsigned int var_num = _nodal_var_nums[i];

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(aux_system_number, var_num, 0);

    _aux_var_dofs[tid][var_num] = dof_number;
    _aux_var_vals_nodal[tid][var_num] = (*_aux_soln)(dof_number);

    if(_is_transient)
    {
      _aux_var_vals_old_nodal[tid][var_num] = (*_aux_old_soln)(dof_number);
      _aux_var_vals_older_nodal[tid][var_num] = (*_aux_older_soln)(dof_number);
    }
  }

  Moose::perf_log.pop("reinit(node)","AuxKernel");
}

void
MooseSystem::reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem)
{
  Moose::perf_log.push("reinit(elem)","AuxKernel");

  unsigned int nonlinear_system_number = _system->number();
  unsigned int aux_system_number = _aux_system->number();

  //Compute the area of the element
  Real area = 0;
  //Just use any old JxW... they are all actually the same
  const std::vector<Real> & jxw = *(_JxW[tid].begin()->second);

  if( Moose::geom_type == Moose::XYZ)
  {
    for (unsigned int qp=0; qp<_qrule[tid]->n_points(); qp++)
      area += jxw[qp];
  }
  else if (Moose::geom_type == Moose::CYLINDRICAL)
  {
    const std::vector<Point> & q_point = *(_q_point[tid].begin()->second);
    for (unsigned int qp=0; qp<_qrule[tid]->n_points(); qp++)
      area += q_point[qp](0)*jxw[qp];
  }
  else
  {
    std::cerr << "geom_type must either XYZ or CYLINDRICAL" << std::endl;
    mooseError("");
  }

  //Compute the average value of each variable on the element

  //Non Aux vars first
  for(unsigned int i=0; i<_var_nums.size(); i++)
  {
    unsigned int var_num = _var_nums[i];

    FEType fe_type = _dof_map->variable_type(var_num);

    const std::vector<Real> & JxW = *_JxW[tid][fe_type];
    const std::vector<Point> & q_point = *_q_point[tid][fe_type];

    _var_vals_element[tid][var_num] = integrateValueAux(_var_vals[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _var_vals_old_element[tid][var_num] = integrateValueAux(_var_vals_old[tid][var_num], JxW, q_point) / area;
      _var_vals_older_element[tid][var_num] = integrateValueAux(_var_vals_older[tid][var_num], JxW, q_point) / area;
    }

    _var_grads_element[tid][var_num] = integrateGradientAux(_var_grads[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _var_grads_old_element[tid][var_num] = integrateGradientAux(_var_grads_old[tid][var_num], JxW, q_point) / area;
      _var_grads_older_element[tid][var_num] = integrateGradientAux(_var_grads_older[tid][var_num], JxW, q_point) / area;
    }
  }

  //Now Aux vars
  for(unsigned int i=0; i<_aux_var_nums.size(); i++)
  {
    unsigned int var_num = _aux_var_nums[i];

    FEType fe_type = _aux_dof_map->variable_type(var_num);

    const std::vector<Real> & JxW = *_JxW[tid][fe_type];
    const std::vector<Point> & q_point = *_q_point[tid][fe_type];

    _aux_var_vals_element[tid][var_num] = integrateValueAux(_aux_var_vals[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _aux_var_vals_old_element[tid][var_num] = integrateValueAux(_aux_var_vals_old[tid][var_num], JxW, q_point) / area;
      _aux_var_vals_older_element[tid][var_num] = integrateValueAux(_aux_var_vals_older[tid][var_num], JxW, q_point) / area;
    }

    _aux_var_grads_element[tid][var_num] = integrateGradientAux(_aux_var_grads[tid][var_num], JxW, q_point) / area;

    if(_is_transient)
    {
      _aux_var_grads_old_element[tid][var_num] = integrateGradientAux(_aux_var_grads_old[tid][var_num], JxW, q_point) / area;
      _aux_var_grads_older_element[tid][var_num] = integrateGradientAux(_aux_var_grads_older[tid][var_num], JxW, q_point) / area;
    }
  }

  //Grab the dof numbers for the element variables
  for(unsigned int i=0; i<_element_var_nums.size(); i++)
  {
    unsigned int var_num = _element_var_nums[i];

    //The zero is the component... that works fine for FIRST order monomials
    unsigned int dof_number = elem.dof_number(aux_system_number, var_num, 0);

    _aux_var_dofs[tid][var_num] = dof_number;
  }

  Moose::perf_log.pop("reinit(elem)","AuxKernel");
}


Real
MooseSystem::integrateValueAux(const std::vector<Real> & vals, const std::vector<Real> & JxW, const std::vector<Point> & q_point)
{
  Real value = 0;

  if( Moose::geom_type == Moose::XYZ)
  {
    for (unsigned int qp=0; qp<_qrule[0]->n_points(); qp++)
      value += vals[qp]*JxW[qp];
  }
  else if( Moose::geom_type == Moose::CYLINDRICAL )
  {
    for (unsigned int qp=0; qp<_qrule[0]->n_points(); qp++)
      value += q_point[qp](0)*vals[qp]*JxW[qp];
  }
  else
  {
    std::cerr << "geom_type must either XYZ or CYLINDRICAL" << std::endl;
    mooseError("");
  }

  return value;
}

RealGradient
MooseSystem::integrateGradientAux(const std::vector<RealGradient> & grads, const std::vector<Real> & JxW, const std::vector<Point> & q_point)
{
  RealGradient value = 0;

  if( Moose::geom_type == Moose::XYZ )
  {
    for (unsigned int qp=0; qp<_qrule[0]->n_points(); qp++)
      value += grads[qp]*JxW[qp];
  }
  else if( Moose::geom_type == Moose::CYLINDRICAL )
  {
    for (unsigned int qp=0; qp<_qrule[0]->n_points(); qp++)
      value += q_point[qp](0)*grads[qp]*JxW[qp];
  }
  else
  {
    std::cerr << "geom_type must either XYZ or CYLINDRICAL" << std::endl;
    mooseError("");
  }


  return value;
}

void
MooseSystem::checkSystemsIntegrity()
{
  parallel_only();
  std::set<subdomain_id_type> element_subdomains, input_subdomains, difference;
  bool global_kernels_exist = false;

  // Build a set of active subdomains from the mesh in MOOSE
  const MeshBase::element_iterator el_end = _mesh->elements_end();
  for (MeshBase::element_iterator el = _mesh->active_elements_begin(); el != el_end; ++el)
    element_subdomains.insert((*el)->subdomain_id());

  // Check materials
  MaterialIterator end = _materials.activeMaterialsEnd(0);
  for (MaterialIterator i = _materials.activeMaterialsBegin(0); i != end; ++i)
    if (element_subdomains.find(i->first) == element_subdomains.end())
    {
      std::stringstream oss;
      oss << "Material block \"" << i->first << "\" specified in the input file does not exist";
      mooseError (oss.str());
    }

  // Check kernels
  _kernels.updateActiveKernels(0);
  global_kernels_exist = _kernels.activeKernelBlocks(input_subdomains);
  std::set_difference (element_subdomains.begin(), element_subdomains.end(),
                       input_subdomains.begin(), input_subdomains.end(),
                       std::inserter(difference, difference.end()));

  if (!global_kernels_exist && !difference.empty())
    mooseError("Each subdomain must contain at least one Kernel.");

  // Check BCs
  // TODO: Check Boundaries
}

void
MooseSystem::setVarScaling(std::vector<Real> scaling)
{
  if(scaling.size() != _system->n_vars())
  {
    std::cout<<"Error: size of scaling factor vector not the same as the number of variables in the system!"<<std::endl;
    mooseError("");
  }

  _scaling_factor = scaling;
}

void
MooseSystem::reinitDT()
{
  _is_transient = true;

  _t = _es->parameters.get<Real>("time");
  _t_step = _es->parameters.get<int>("t_step");
  _dt_old = _dt;
  _dt = _es->parameters.get<Real>("dt");
  Real sum = _dt+_dt_old;
  _bdf2_wei[2] = 1.+_dt/sum;
  _bdf2_wei[1] =-sum/_dt_old;
  _bdf2_wei[0] =_dt*_dt/_dt_old/sum;
}

void
MooseSystem::reinitEigen()
{

}

void
MooseSystem::meshChanged()
{
  // Reinitialize the equation_systems object for the newly refined
  // mesh. One of the steps in this is project the solution onto the
  // new mesh
  _es->reinit();

  // Rebuild the boundary conditions
  _mesh->boundary_info->build_node_list_from_side_list();

  // Rebuild the active local element range
  delete active_local_elem_range;
  active_local_elem_range = NULL;

  // Calling this function will rebuild the range.
  getActiveLocalElementRange();

  // Lets the output system know that the mesh has changed recently.
  _mesh_changed = true;
}

ConstElemRange *
MooseSystem::getActiveLocalElementRange()
{
  if(!active_local_elem_range)
  {
    active_local_elem_range = new ConstElemRange(_mesh->active_local_elements_begin(),
                                                 _mesh->active_local_elements_end(), 1);
  }

  return active_local_elem_range;
}

/**
 * Outputs the system.
 */
void
MooseSystem::output_system(unsigned int t_step, Real time)
{
  std::string file_base = Moose::file_base;

  OStringStream stream_file_base;

  stream_file_base << file_base << "_";
  OSSRealzeroright(stream_file_base,3,0,t_step);

  std::string file_name = stream_file_base.str();

  if(Moose::exodus_output)
  {
    std::string exodus_file_name;

    static ExodusII_IO * ex_out = NULL;
    static unsigned int num_files = 0;
    static unsigned int num_in_current_file = 0;

    bool adaptivity = _es->parameters.have_parameter<bool>("adaptivity");

    //if the mesh changed we need to write to a new file
    if(_mesh_changed || !ex_out)
    {
      num_files++;

      if(ex_out)
        delete ex_out;

      ex_out = new ExodusII_IO(_es->get_mesh());

      // We've captured this change... let's reset the changed bool and then see if it's changed again next time.
      _mesh_changed = false;

      // We're starting over
      num_in_current_file = 0;
    }

    num_in_current_file++;

    if(!adaptivity)
      exodus_file_name = file_base;
    else
    {
      OStringStream exodus_stream_file_base;

      exodus_stream_file_base << file_base << "_";

      // -1 is so that the first one that comes out is 000
      OSSRealzeroright(exodus_stream_file_base,4,0,num_files-1);

      exodus_file_name = exodus_stream_file_base.str();
    }

    // The +1 is because Exodus starts timesteps at 1 and we start at 0
    ex_out->write_timestep(exodus_file_name + ".e", *_es, num_in_current_file, time);
  }
  if(Moose::gmv_output)
    GMVIO(*_mesh).write_equation_systems(file_name + ".gmv", *_es);
  if(Moose::tecplot_output)
    TecplotIO(*_mesh).write_equation_systems(file_name + ".plt", *_es);
}

bool & MooseSystem::dontReinitFE()
{
  return _no_fe_reinit;
}

Parameters &
MooseSystem::parameters()
{
  static Parameters blank_pars;
  if (_es == NULL) return blank_pars;
  else return _es->parameters;
}

void
MooseSystem::copy_old_solutions()
{
  *_system->older_local_solution = *_system->old_local_solution;
  *_system->old_local_solution   = *_system->current_local_solution;
  *_aux_system->older_local_solution = *_aux_system->old_local_solution;
  *_aux_system->old_local_solution   = *_aux_system->current_local_solution;
}
