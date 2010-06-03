#include "MooseSystem.h"
#include "ElementData.h"
#include "FaceData.h"
#include "AuxData.h"
#include "MaterialData.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "MaterialFactory.h"
#include "StabilizerFactory.h"
#include "InitialConditionFactory.h"
#include "AuxKernel.h"
#include "ParallelUniqueId.h"
#include "ComputeQPSolution.h"

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
 : _element_data(*this),
   _face_data(*this),
   _aux_data(*this, _element_data),
   _material_data(*this),
   _es(NULL),
   _system(NULL),
   _aux_system(NULL),
   _mesh(NULL),
   _delete_mesh(true),
   _dim(0),
   _mesh_changed(false),
   _kernels(*this),
   _bcs(*this),
   _auxs(*this),
   _materials(*this),
   _stabilizers(*this),
   _ics(*this),
   _no_fe_reinit(false),
   _preconditioner(NULL),
   _exreader(NULL),
   _is_valid(false),
   _t(0),
   _dt(0),
   _dt_old(0),
   _is_transient(false),
   _is_eigenvalue(false),
   _t_step(0),
   _t_scheme(0),
   _n_of_rk_stages(0),
   _active_local_elem_range(NULL)
{
  Moose::g_system = this;     // FIXME: this will eventually go away
  sizeEverything();
}

MooseSystem::MooseSystem(Mesh &mesh)
  : _element_data(*this),
    _face_data(*this),
    _aux_data(*this, _element_data),
    _material_data(*this),
    _es(NULL),
    _system(NULL),
    _aux_system(NULL),
    _mesh(&mesh),
    _delete_mesh(false),
    _dim(_mesh->mesh_dimension()),
    _mesh_changed(false),
    _kernels(*this),
    _bcs(*this),
    _auxs(*this),
    _materials(*this),
    _stabilizers(*this),
    _ics(*this),
    _no_fe_reinit(false),
    _preconditioner(NULL),
    _exreader(NULL),
    _is_valid(false),
    _t(0),
    _dt(0),
    _dt_old(0),
    _is_transient(false),
    _is_eigenvalue(false),
    _t_step(0),
    _t_scheme(0),
    _n_of_rk_stages(0),
    _active_local_elem_range(NULL)
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
  {
    /*
    for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid) 
    {
      delete _element_data[tid];
      delete _face_data[tid];
      delete _aux_data[tid];
    }
    */
  }

  if (_es != NULL)
    delete _es;

  if (_delete_mesh && _mesh != NULL)
    delete _mesh;

  delete _active_local_elem_range;
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
MooseSystem::getMesh(bool skip_full_check) 
{
  if (!skip_full_check)
    checkValid();
  else if (_mesh == NULL)
    mooseError("Full check skipped but Mesh is not initialized");
  return _mesh;
}

Material *
MooseSystem::getMaterial(THREAD_ID tid, unsigned int block_id)
{
  return _materials.getMaterial(tid,block_id);
}

void
MooseSystem::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  // Kernels::sizeEverything
  _bdf2_wei.resize(3);

  _dof_indices.resize(n_threads);
  _var_dof_indices.resize(n_threads);

  _aux_var_dof_indices.resize(n_threads);
  _aux_var_dofs.resize(n_threads);

  // Single Instance Variables
  _real_zero.resize(n_threads);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);

  // AuxKernels::sizeEverything
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
    // aux var
    _aux_var_dofs[tid].resize(n_aux_vars);
    _aux_var_dof_indices[tid].resize(n_aux_vars);
  }

  //Find the largest quadrature order necessary... all variables _must_ use the same rule!
  _max_quadrature_order = CONSTANT;
  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);
    if(fe_type.default_quadrature_order() > _max_quadrature_order)
      _max_quadrature_order = fe_type.default_quadrature_order();
  }

  _element_data.init();
  _face_data.init();
  _aux_data.init();

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

  //Set the default variable scaling to 1
  for(unsigned int i=0; i < _system->n_vars(); i++)
    _scaling_factor.push_back(1.0);

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

  // TODO: Make multiple copies of the data objects instead of select
  // members inside of these objects
/*
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
*/

  // Need to initialize data
  _is_valid = true;
}

void
MooseSystem::checkValid()
{
  if (!_is_valid)
    mooseError("MooseSystem has not been properly initialized before accessing data");
}

/*
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
*/

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
  _element_data._var_nums[0].push_back(var_num);
  return var_num;
}

unsigned int
MooseSystem::addVariable(const std::string &var, const Order order, const FEFamily family, const std::set< subdomain_id_type > *const active_subdomains)
{
  unsigned int var_num = _system->add_variable(var, order, family, active_subdomains);
  _element_data._var_nums[0].push_back(var_num);
  return var_num;
}

// Kernels ////
void MooseSystem::addKernel(std::string kernel_name,
                            std::string name,
                            InputParameters parameters)
{
  Kernel * kernel;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    if (!parameters.isParamValid("block"))
    {
      kernel = KernelFactory::instance()->create(kernel_name, name, *this, parameters);
      _kernels._all_kernels[tid].push_back(kernel);
    }
    
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
    
      for (unsigned int i=0; i<blocks.size(); ++i)
      {
        kernel = KernelFactory::instance()->create(kernel_name, name, *this, parameters);
        _kernels._all_block_kernels[tid][blocks[i]].push_back(kernel);
      }
    }
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
        _bcs._active_bcs[tid][boundaries[i]].push_back(bc);
      else
        _bcs._active_nodal_bcs[tid][boundaries[i]].push_back(bc);
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
      aux_ptr = &_auxs._active_nodal_aux_kernels[tid];
    else
      aux_ptr = &_auxs._active_element_aux_kernels[tid];

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
      _auxs._active_bcs[tid][boundaries[i]].push_back(aux);
    _auxs._aux_bcs[tid].push_back(aux);
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

    for (unsigned int i=0; i<blocks.size(); ++i) {
      parameters.set<bool>("_is_boudary_material") = false;
      _materials._active_materials[tid][blocks[i]] = MaterialFactory::instance()->create(mat_name, name, *this, parameters);

      parameters.set<bool>("_is_boudary_material") = true;
      _materials._active_boundary_materials[tid][blocks[i]] = MaterialFactory::instance()->create(mat_name, name, *this, parameters);
    }
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
      _stabilizers._block_stabilizers[tid][parameters.get<unsigned int>("block_id")][stabilizer->variable()] = stabilizer;
    else
      _stabilizers._active_stabilizers[tid][stabilizer->variable()] = stabilizer;

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

    _ics._active_initial_conditions[tid][var_name] = InitialConditionFactory::instance()->create(ic_name, name, *this, parameters);
  }
}

void
MooseSystem::reinitKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
  _element_data.reinitKernels(tid, soln, elem, Re, Ke);
}

void
MooseSystem::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const unsigned int boundary_id)
{
  _face_data.reinit(tid, soln, elem, side, boundary_id);
}

void
MooseSystem::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
  _face_data.reinit(tid, soln, node, boundary_id, residual);
}

void
MooseSystem::reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node)
{
  _face_data._current_node[tid] = &node;
  _aux_data.reinit(tid, soln, node);
}

void
MooseSystem::reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem)
{
  _aux_data.reinit(tid, soln, elem);
}

void
MooseSystem::subdomainSetup(THREAD_ID tid, unsigned int block_id)
{
  _element_data._material[tid] = _materials.getMaterial(tid, block_id);
  _face_data._material[tid] = _materials.getBoundaryMaterial(tid, block_id);

  // call subdomainSetup
  _element_data._material[tid]->subdomainSetup();
  _face_data._material[tid]->subdomainSetup();

  //Global Kernels
  KernelIterator kernel_begin = _kernels.activeKernelsBegin(tid);
  KernelIterator kernel_end = _kernels.activeKernelsEnd(tid);
  for(KernelIterator kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
    (*kernel_it)->subdomainSetup();

  //Kernels on this block
  KernelIterator block_kernel_begin = _kernels.blockKernelsBegin(tid, block_id);
  KernelIterator block_kernel_end = _kernels.blockKernelsEnd(tid, block_id);
  for(KernelIterator block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
    (*block_kernel_it)->subdomainSetup();

  //Stabilizers
  StabilizerIterator stabilizer_begin = _stabilizers.activeStabilizersBegin(tid);
  StabilizerIterator stabilizer_end = _stabilizers.activeStabilizersEnd(tid);
  for(StabilizerIterator stabilizer_it=stabilizer_begin;stabilizer_it!=stabilizer_end;stabilizer_it++)
    stabilizer_it->second->subdomainSetup();
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

  bool adaptivity = _es->parameters.have_parameter<bool>("adaptivity");

  // Check materials
  MaterialIterator end = _materials.activeMaterialsEnd(0);
  for (MaterialIterator i = _materials.activeMaterialsBegin(0); i != end; ++i)
  {
    if(i->second->hasStatefulProperties() && adaptivity)
      mooseError("Cannot use Material classes with stateful properties while utilizing adaptivity!");
    
    if (element_subdomains.find(i->first) == element_subdomains.end())
    {
      std::stringstream oss;
      oss << "Material block \"" << i->first << "\" specified in the input file does not exist";
      mooseError (oss.str());
    }
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
  delete _active_local_elem_range;
  _active_local_elem_range = NULL;

  // Calling this function will rebuild the range.
  getActiveLocalElementRange();

  // Lets the output system know that the mesh has changed recently.
  _mesh_changed = true;
}

ConstElemRange *
MooseSystem::getActiveLocalElementRange()
{
  if(!_active_local_elem_range)
  {
    _active_local_elem_range = new ConstElemRange(_mesh->active_local_elements_begin(),
                                                 _mesh->active_local_elements_end(), 1);
  }

  return _active_local_elem_range;
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

