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
#include "PostprocessorFactory.h"
#include "AuxKernel.h"
#include "ParallelUniqueId.h"
#include "ComputeQPSolution.h"
#include "ComputeResidual.h"
#include "ComputeJacobian.h"
#include "ComputeInitialConditions.h"

//libMesh includes
#include "numeric_vector.h"
#include "exodusII_io.h"
#include "parallel.h"
#include "gmv_io.h"
#include "tecplot_io.h"
#include "boundary_info.h"
#include "mesh_refinement.h"
#include "error_estimator.h"
#include "error_vector.h"
#include "kelly_error_estimator.h"
#include "fourth_error_estimators.h"

MooseSystem::MooseSystem()
 : _element_data(*this),
   _face_data(*this),
   _aux_data(*this, _element_data),
   _material_data(*this),
   _postprocessor_data(*this),
   _es(NULL),
   _system(NULL),
   _aux_system(NULL),
   _geom_type(Moose::XYZ),
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
   _pps(*this),
   _no_fe_reinit(false),
   _preconditioner(NULL),
   _exreader(NULL),
   _is_valid(false),
   _mesh_refinement(NULL),
   _error_estimator(NULL),
   _error(NULL),
   _t(0),
   _dt(0),
   _dt_old(0),
   _is_transient(false),
   _is_eigenvalue(false),
   _t_step(0),
   _t_scheme(0),
   _n_of_rk_stages(0),
   _active_local_elem_range(NULL),
   _auto_scaling(false),
   _print_mesh_changed(false),
   _file_base ("out"),
   _interval(1),
   _exodus_output(true),
   _gmv_output(false),
   _tecplot_output(false),
   _print_out_info(false),
   _output_initial(false),
   _l_abs_step_tol(1e-10)
{
  sizeEverything();
}

MooseSystem::MooseSystem(Mesh &mesh)
  : _element_data(*this),
    _face_data(*this),
    _aux_data(*this, _element_data),
    _material_data(*this),
    _postprocessor_data(*this),
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
    _pps(*this),
    _no_fe_reinit(false),
    _preconditioner(NULL),
    _exreader(NULL),
    _is_valid(false),
    _mesh_refinement(NULL),
    _error_estimator(NULL),
    _error(NULL),
    _t(0),
    _dt(0),
    _dt_old(0),
    _is_transient(false),
    _is_eigenvalue(false),
    _t_step(0),
    _t_scheme(0),
    _n_of_rk_stages(0),
    _active_local_elem_range(NULL),
    _auto_scaling(false),
    _print_mesh_changed(false),
    _file_base ("out"),
    _interval(1),
    _exodus_output(true),
    _gmv_output(false),
    _tecplot_output(false),
    _print_out_info(false),
    _output_initial(false),
    _l_abs_step_tol(1e-10)
{
  sizeEverything();
  initEquationSystems();

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

  delete _mesh_refinement;
  delete _error_estimator;
  delete _error;
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

std::vector<Material *>
MooseSystem::getMaterials(THREAD_ID tid, unsigned int block_id)
{
  return _materials.getMaterials(tid, block_id);
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
MooseSystem::setPrintMeshChanged(bool print_mesh_changed)
{
  _print_mesh_changed = print_mesh_changed;
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
    mooseError("Error: Size of scaling factor vector not the same as the number of variables in the system!\n");
  
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

QuadraturePointData &
MooseSystem::getQuadraturePointData(bool is_boundary)
{
  if (is_boundary)
    return _face_data;
  else
    return _element_data;
}

bool
MooseSystem::hasVariable(const std::string &var_name)
{
  return _system->has_variable(var_name);
}

bool
MooseSystem::hasAuxVariable(const std::string &var_name)
{
  return _aux_system->has_variable(var_name);
}

unsigned int
MooseSystem::getVariableNumber(const std::string &var_name)
{
  return _system->variable_number(var_name);
}

unsigned int
MooseSystem::getAuxVariableNumber(const std::string &var_name)
{
  return _aux_system->variable_number(var_name);
}

unsigned int
MooseSystem::modifiedAuxVarNum(unsigned int var_num)
{
  return MAX_VARS + var_num;
}

EquationSystems *
MooseSystem::initEquationSystems()
{
  if (_es != NULL)
    mooseError("EquationSystems Object already initialized for this MooseSystem");
  
  _es = new EquationSystems(*_mesh);

  // Store off the MooseSystem so we can get access to it.
  _es->parameters.set<MooseSystem *>("moose_system") = this;
  
  _system = &_es->add_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  _system->nonlinear_solver->residual = Moose::compute_residual;
  _system->nonlinear_solver->jacobian = Moose::compute_jacobian;
  _system->attach_init_function(Moose::initial_condition);

  _aux_system = &_es->add_system<TransientExplicitSystem>("AuxiliarySystem");
  _aux_system->attach_init_function(Moose::initial_condition);


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
MooseSystem::initAdaptivity(unsigned int max_r_steps, unsigned int initial_steps)
{
  if (_mesh_refinement)
    mooseError("Mesh refinement object has already been initialized!");

  _es->parameters.set<bool>("adaptivity") = true;
  _es->parameters.set<unsigned int>("max_r_steps") = max_r_steps;
  _es->parameters.set<unsigned int>("initial_adaptivity") = initial_steps;

  _mesh_refinement = new MeshRefinement(*_mesh);
  _error = new ErrorVector;
}

unsigned int
MooseSystem::getInitialAdaptivityStepCount()
{
  if (_es->parameters.have_parameter<unsigned int>("initial_adaptivity"))
    return _es->parameters.get<unsigned int>("initial_adaptivity");
  else
    return 0;
}

void
MooseSystem::setErrorEstimator(const std::string &error_estimator_name)
{
  if(error_estimator_name == "KellyErrorEstimator")
    _error_estimator = new KellyErrorEstimator;
  else if(error_estimator_name == "LaplacianErrorEstimator")
    _error_estimator = new LaplacianErrorEstimator;
  else
    mooseError("Unknown error_estimator selection: " + error_estimator_name);
}


void
MooseSystem::setErrorNorm(SystemNorm &sys_norm)
{
  mooseAssert(_error_estimator != NULL, "error_estimator not initialized. Did you call init_adaptivity()?");
  _error_estimator->error_norm = sys_norm;
}

void
MooseSystem::adaptMesh()
{
  if (_mesh_refinement)
  {
    // Compute the error for each active element
    _error_estimator->estimate_error(*_system, *_error);

    // Flag elements to be refined and coarsened
    _mesh_refinement->flag_elements_by_error_fraction (*_error);

    // Perform refinement and coarsening
    _mesh_refinement->refine_and_coarsen_elements();

    // Tell MOOSE that the Mesh has changed
    meshChanged();
  }
}

void
MooseSystem::doAdaptivityStep()
{
  // Compute the error for each active element
  _error_estimator->estimate_error(*_system, *_error);

  // Flag elements to be refined and coarsened
  _mesh_refinement->flag_elements_by_error_fraction (*_error);

  // Perform refinement and coarsening
  _mesh_refinement->refine_and_coarsen_elements();
}

Real &
MooseSystem::getPostprocessorValue(std::string name)
{
  return _postprocessor_data._values[name];
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
  _system->update();
  compute_postprocessors(*(_system->current_local_solution));
}

unsigned int
MooseSystem::addVariable(const std::string &var, const FEType  &type, const std::set< subdomain_id_type  > *const active_subdomains)
{
  unsigned int var_num = _system->add_variable(var, type, active_subdomains);
  _element_data._var_nums[0].insert(var_num);
  return var_num;
}

unsigned int
MooseSystem::addVariable(const std::string &var, const Order order, const FEFamily family, const std::set< subdomain_id_type > *const active_subdomains)
{
  unsigned int var_num = _system->add_variable(var, order, family, active_subdomains);
  _element_data._var_nums[0].insert(var_num);
  return var_num;
}

// Kernels ////
void MooseSystem::addKernel(std::string kernel_name,
                            std::string name,
                            InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if (!parameters.isParamValid("block"))
      _kernels.addKernel(tid, KernelFactory::instance()->create(kernel_name, name, *this, parameters));
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
    
      for (unsigned int i=0; i<blocks.size(); ++i)
        _kernels.addBlockKernel(tid, blocks[i], KernelFactory::instance()->create(kernel_name, name, *this, parameters));
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
    parameters.set<THREAD_ID>("_tid") = tid;
    std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

    for (unsigned int i=0; i<boundaries.size(); ++i)
    {
      parameters.set<unsigned int>("_boundary_id") = boundaries[i];
      BoundaryCondition * bc = BCFactory::instance()->create(bc_name, name, *this, parameters);

      if(bc->isIntegrated())
        _bcs.addBC(tid, boundaries[i], bc);
      else
        _bcs.addNodalBC(tid, boundaries[i], bc);
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

  std::vector<std::list<AuxKernel *>::iterator > dependent_auxs;
  std::vector<AuxKernel *> *aux_ptr;
  std::list<AuxKernel *>::iterator new_aux_iter;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    aux = AuxFactory::instance()->create(aux_name, name, *this, parameters);
    std::vector<std::string> coupled_to = aux->coupledTo();

    // Copy the active AuxKernels into a list for manipulation
    std::list<AuxKernel *> active_auxs;
    if (aux->isNodal())
      active_auxs = _auxs.getActiveNodalKernels(tid);
    else
      active_auxs = _auxs.getActiveElementKernels(tid);

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

    // Copy the list back into the Auxiliary Vector
    if (aux->isNodal())
      _auxs.setActiveNodalKernels(tid, active_auxs);
    else
      _auxs.setActiveElementKernels(tid, active_auxs);
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
    parameters.set<THREAD_ID>("_tid") = tid;
    std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

    aux = AuxFactory::instance()->create(aux_name, name, *this, parameters);

    for (unsigned int i=0; i<boundaries.size(); ++i)
      _auxs.addActiveBC(tid, boundaries[i], aux);
    _auxs.addBC(tid, aux);
  }
}

void
MooseSystem::addMaterial(std::string mat_name,
                         std::string name,
                         InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");

    // We need to set 'variable' in order to initialize other member data, since Material inherits from PDEBase
    parameters.set<std::string>("variable") = _es->get_system(0).variable_name(0);

    for (unsigned int i=0; i<blocks.size(); ++i) {
      parameters.set<int>("_bid") = blocks[i];

      parameters.set<bool>("_is_boudary_material") = false;
      _materials.addMaterial(tid, blocks[i], MaterialFactory::instance()->create(mat_name, name, *this, parameters));

      parameters.set<bool>("_is_boudary_material") = true;
      _materials.addBoundaryMaterial(tid, blocks[i], MaterialFactory::instance()->create(mat_name, name, *this, parameters));
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
    parameters.set<THREAD_ID>("_tid") = tid;

    stabilizer = StabilizerFactory::instance()->create(stabilizer_name, name, *this, parameters);

    if (parameters.have_parameter<unsigned int>("block_id"))
      _stabilizers.addBlockStabilizer(tid, parameters.get<unsigned int>("block_id"), stabilizer->variable(), stabilizer);
    else
      _stabilizers.addStabilizer(tid, stabilizer->variable(), stabilizer);
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
    parameters.set<THREAD_ID>("_tid") = tid;

    // The var_name needs to be added to the parameters object for any InitialCondition derived objects
    parameters.set<std::string>("var_name") = var_name;

    _ics.addIC(tid, var_name, InitialConditionFactory::instance()->create(ic_name, name, *this, parameters));
  }
}

void
MooseSystem::addPostprocessor(std::string pp_name,
                              std::string name,
                              InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    _pps.addPostprocessor(tid, PostprocessorFactory::instance()->create(pp_name, name, *this, parameters));
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
  _element_data._material[tid] = _materials.getMaterials(tid, block_id);
  _face_data._material[tid] = _materials.getBoundaryMaterials(tid, block_id);

  // call subdomainSetup
  for (std::vector<Material *>::iterator it = _element_data._material[tid].begin(); it != _element_data._material[tid].end(); ++it)
    (*it)->subdomainSetup();
  for (std::vector<Material *>::iterator it = _face_data._material[tid].begin(); it != _face_data._material[tid].end(); ++it)
    (*it)->subdomainSetup();

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
  std::set<subdomain_id_type> element_subdomains;

  // Build a set of active subdomains from the mesh in MOOSE
  const MeshBase::element_iterator el_end = _mesh->elements_end();
  for (MeshBase::element_iterator el = _mesh->active_elements_begin(); el != el_end; ++el)
    element_subdomains.insert((*el)->subdomain_id());

  bool adaptivity = _es->parameters.have_parameter<bool>("adaptivity");

  // Check materials
  for (MaterialIterator i = _materials.activeMaterialsBegin(0); i != _materials.activeMaterialsEnd(0); ++i)
  {
    for (std::vector<Material *>::iterator j = i->second.begin(); j != i->second.end(); ++j)
      if ((*j)->hasStatefulProperties() && adaptivity)
        mooseError("Cannot use Material classes with stateful properties while utilizing adaptivity!");
    
    if (element_subdomains.find(i->first) == element_subdomains.end())
    {
      std::stringstream oss;
      oss << "Material block \"" << i->first << "\" specified in the input file does not exist";
      mooseError (oss.str());
    }
  }

  // Check kernel coverage of subdomains (blocks) in your mesh
  {
    std::set<subdomain_id_type> input_subdomains;
    std::set<unsigned short> difference;
    bool global_kernels_exist = false;
    
    
    _kernels.updateActiveKernels(0);
    global_kernels_exist = _kernels.activeKernelBlocks(input_subdomains);
    std::set_difference (element_subdomains.begin(), element_subdomains.end(),
                       input_subdomains.begin(), input_subdomains.end(),
                       std::inserter(difference, difference.end()));

    if (!global_kernels_exist && !difference.empty())
    {
      std::stringstream missing_block_ids;

      std::copy (difference.begin(), difference.end(), std::ostream_iterator<unsigned short>( missing_block_ids, " "));
      
      mooseError("Each subdomain must contain at least one Kernel.\nThe following blocks lack an active kernel "
                 + missing_block_ids.str());
    }
  }

  // Check that BCs used in your simulation exist in your mesh
  {
    std::set<short> input_bcs, difference;
    const std::set<short> & simulation_bcs = _mesh->boundary_info->get_boundary_ids();

    _bcs.activeBoundaries(input_bcs);  // get the boundaries from the simulation (input file)
    std::set_difference (input_bcs.begin(), input_bcs.end(),
                         simulation_bcs.begin(), simulation_bcs.end(),
                         std::inserter(difference, difference.end()));
    if (!difference.empty())
    {
      std::stringstream extra_boundary_ids;

      std::copy (difference.begin(), difference.end(), std::ostream_iterator<unsigned short>( extra_boundary_ids, " "));
      
      mooseError("The following boundary ids from your input file do not exist in the input mesh "
                 + extra_boundary_ids.str());
    }
  }
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

  // Print out information about the adapated mesh if requested
  if (_print_mesh_changed)
  {
    std::cout << "\nMesh Changed:\n";
    _mesh->print_info();
  }
  
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
  OStringStream stream_file_base;

  stream_file_base << _file_base << "_";
  OSSRealzeroright(stream_file_base,3,0,t_step);

  std::string file_name = stream_file_base.str();

  if (_exodus_output)
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
      exodus_file_name = _file_base;
    else
    {
      OStringStream exodus_stream_file_base;

      exodus_stream_file_base << _file_base << "_";

      // -1 is so that the first one that comes out is 000
      OSSRealzeroright(exodus_stream_file_base,4,0,num_files-1);

      exodus_file_name = exodus_stream_file_base.str();
    }

    // The +1 is because Exodus starts timesteps at 1 and we start at 0
    ex_out->write_timestep(exodus_file_name + ".e", *_es, num_in_current_file, time);
  }
  if(_gmv_output)
    GMVIO(*_mesh).write_equation_systems(file_name + ".gmv", *_es);
  if(_tecplot_output)
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

void
MooseSystem::project_solution(Number fptr(const Point& p,
                                          const Parameters& parameters,
                                          const std::string& sys_name,
                                          const std::string& unknown_name),
                              Gradient gptr(const Point& p,
                                            const Parameters& parameters,
                                            const std::string& sys_name,
                                            const std::string& unknown_name))
{
  _system->project_solution(fptr, gptr, _es->parameters);
}
