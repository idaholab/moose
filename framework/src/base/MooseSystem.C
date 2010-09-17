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

#include "MooseSystem.h"
#include "KernelFactory.h"
#include "DGKernelFactory.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "MaterialFactory.h"
#include "StabilizerFactory.h"
#include "InitialConditionFactory.h"
#include "PostprocessorFactory.h"
#include "FunctionFactory.h"
#include "AuxKernel.h"
#include "ParallelUniqueId.h"
#include "ComputeQPSolution.h"
#include "ComputeResidual.h"
#include "ComputeJacobian.h"
#include "ComputeInitialConditions.h"
#include "DamperFactory.h"
#include "Executioner.h"

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
#include "fe_interface.h"
#include "mesh_tools.h"

MooseSystem::MooseSystem() :
  _dof_data(libMesh::n_threads(), DofData(*this)),
  _neighbor_dof_data(libMesh::n_threads(), DofData(*this)),
  _material_data(libMesh::n_threads(), MaterialData(*this)),
  _bnd_material_data(libMesh::n_threads(), MaterialData(*this)),
  _neighbor_material_data(libMesh::n_threads(), MaterialData(*this)),
  _postprocessor_data(libMesh::n_threads(), PostprocessorData(*this)),
  _es(NULL),
  _displaced_es(NULL),
  _system(NULL),
  _aux_system(NULL),
  _geom_type(Moose::XYZ),
  _mesh(NULL),
  _displaced_mesh(NULL),
  _has_displaced_mesh(false),
  _delete_mesh(true),
  _dim(0),
  _has_dampers(false),
  _ex_out(NULL),
  _num_files(0),
  _num_in_current_file(0),
  _num_files_displaced(0),
  _num_in_current_file_displaced(1),
  _need_old_newton(false),
  _newton_soln(NULL),
  _old_newton_soln(NULL),
  _compute_pps_each_residual_evaluation(false),
  _mesh_changed(false),
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
  _active_node_range(NULL),
  _auto_scaling(false),
  _print_mesh_changed(false),
  _file_base ("out"),
  _interval(1),
  _exodus_output(true),
  _gmv_output(false),
  _tecplot_output(false),
  _postprocessor_screen_output(true),
  _postprocessor_csv_output(false),
  _print_out_info(false),
  _output_initial(false),
  _l_abs_step_tol(1e-10),
  _last_rnorm(0),
  _initial_residual(0)
{
  sizeEverything();
}

MooseSystem::MooseSystem(Mesh &mesh) :
  _dof_data(libMesh::n_threads(), DofData(*this)),
  _neighbor_dof_data(libMesh::n_threads(), DofData(*this)),
  _material_data(libMesh::n_threads(), MaterialData(*this)),
  _bnd_material_data(libMesh::n_threads(), MaterialData(*this)),
  _neighbor_material_data(libMesh::n_threads(), MaterialData(*this)),
  _postprocessor_data(libMesh::n_threads(), PostprocessorData(*this)),
  _es(NULL),
  _displaced_es(NULL),
  _system(NULL),
  _aux_system(NULL),
  _mesh(&mesh),
  _displaced_mesh(NULL),
  _has_displaced_mesh(false),
  _delete_mesh(false),
  _dim(_mesh->mesh_dimension()),
  _has_dampers(false),
  _ex_out(NULL),
  _num_files(0),
  _num_in_current_file(0),
  _num_files_displaced(0),
  _num_in_current_file_displaced(1),
  _need_old_newton(false),
  _newton_soln(NULL),
  _old_newton_soln(NULL),
  _compute_pps_each_residual_evaluation(false),
  _mesh_changed(false),
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
  _active_node_range(NULL),
  _auto_scaling(false),
  _print_mesh_changed(false),
  _file_base ("out"),
  _interval(1),
  _exodus_output(true),
  _gmv_output(false),
  _tecplot_output(false),
  _xda_output(false),
  _postprocessor_screen_output(true),
  _postprocessor_csv_output(false),
  _print_out_info(false),
  _output_initial(false),
  _l_abs_step_tol(1e-10),
  _last_rnorm(0),
  _initial_residual(0)
{
  sizeEverything();
  initEquationSystems();

  _mesh->prepare_for_use(false);
  _mesh->boundary_info->build_node_list_from_side_list();
  MeshTools::build_nodes_to_elem_map(*_mesh, node_to_elem_map);
//  meshChanged();

  initDataStructures();
}

MooseSystem::~MooseSystem()
{
  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    delete _element_data[tid];
    delete _face_data[tid];
    delete _neighbor_face_data[tid];
    delete _aux_data[tid];
    delete _damper_data[tid];
  }

  if (_preconditioner != NULL)
    delete _preconditioner;

  if (_es != NULL)
    delete _es;

  if (_displaced_es != NULL)
    delete _displaced_es;

  if (_exreader != NULL)
    delete _exreader;

  if (_delete_mesh && _mesh != NULL)
    delete _mesh;

  delete _active_local_elem_range;
  delete _active_node_range;

  delete _mesh_refinement;
  delete _error_estimator;
  delete _error;

  if (_executioner != NULL)
    delete _executioner;
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
MooseSystem::initDisplacedMesh(std::vector<std::string> displacements) 
{
  if (_mesh == NULL)
    mooseError("The regular mesh must already be initialized before the displaced mesh can be!");

  _has_displaced_mesh = true;
  _displacements = displacements;
  _displaced_mesh = new Mesh(*_mesh);
  
  return _displaced_mesh;
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

Mesh *
MooseSystem::getDisplacedMesh(bool skip_full_check) 
{
  if (!skip_full_check)
    checkValid();
  else if (_mesh == NULL)
    mooseError("Full check skipped but Mesh is not initialized");
  return _displaced_mesh;
}

bool
MooseSystem::hasDisplacedMesh()
{
  return _has_displaced_mesh;
}

bool
MooseSystem::hasDampers()
{
  return _has_dampers;
}


std::vector<std::string>
MooseSystem::getDisplacementVariables()
{
  return _displacements;
}

void
MooseSystem::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _first.resize(n_threads, true);

  _element_data.resize(n_threads);
  _face_data.resize(n_threads);
  _neighbor_face_data.resize(n_threads);
  _aux_data.resize(n_threads);
  _damper_data.resize(n_threads);
  
  for (THREAD_ID tid = 0; tid < n_threads; ++tid)
  {
    _element_data[tid] = new ElementData(*this, _dof_data[tid]);
    _face_data[tid] = new FaceData(*this, _dof_data[tid]);
    _neighbor_face_data[tid] = new FaceData(*this, _neighbor_dof_data[tid]);
    _aux_data[tid] = new AuxData(*this, _dof_data[tid], *_element_data[tid]);
    _damper_data[tid] = new DamperData(*this, *_element_data[tid]);
  }

  _kernels.resize(n_threads);
  _dg_kernels.resize(n_threads);
  _bcs.resize(n_threads);
  _auxs.resize(n_threads);
  _materials.resize(n_threads);
  _stabilizers.resize(n_threads);
  _ics.resize(n_threads);
  _pps.resize(n_threads);
  _functions.resize(n_threads);
  _dampers.resize(n_threads);

  // Kernels::sizeEverything
  _bdf2_wei.resize(3);

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

  if(_has_displaced_mesh)
    _displaced_es->init();
  
  _dof_map = &_system->get_dof_map();
  _aux_dof_map = &_aux_system->get_dof_map();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _dof_data[tid].init();
    _neighbor_dof_data[tid].init();
  }

  //Find the largest quadrature order necessary... all variables _must_ use the same rule!
  _max_quadrature_order = CONSTANT;
  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);
    if(fe_type.default_quadrature_order() > _max_quadrature_order)
      _max_quadrature_order = fe_type.default_quadrature_order();
  }

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _element_data[tid]->init();
    _face_data[tid]->init();
    _neighbor_face_data[tid]->init();
    _aux_data[tid]->init();
    _damper_data[tid]->init();
  }

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

  // Make sure the displaced mesh is consistent with the initial condition
  if(_has_displaced_mesh)
    updateDisplacedMesh(*_system->solution);
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

EquationSystems *
MooseSystem::getDisplacedEquationSystems()
{
//  checkValid();
  return _displaced_es;
}

void
MooseSystem::initExecutioner(Executioner * e)
{
  if (_executioner != NULL)
    mooseError("Executioner already initialized for this MooseSystem");

  _executioner = e;
}

Executioner &
MooseSystem::getExecutioner()
{
  if (_executioner == NULL)
    mooseError("Executioner not available.");
  return *_executioner;
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

ExplicitSystem *
MooseSystem::getDisplacedSystem()
{
  checkValid();
  return _displaced_system;
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

  if(_has_displaced_mesh)
  {
    _displaced_es = new EquationSystems(*_displaced_mesh);
    _displaced_es->parameters.set<MooseSystem *>("moose_system") = this;
    _displaced_system = &_displaced_es->add_system<ExplicitSystem>("DisplacedSystem");
  }

  // Store off the MooseSystem so we can get access to it.
  _es->parameters.set<MooseSystem *>("moose_system") = this;
  
  _system = &_es->add_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  _system->nonlinear_solver->residual = Moose::compute_residual;
  _system->nonlinear_solver->jacobian = Moose::compute_jacobian;
  _system->attach_init_function(Moose::initial_condition);

  _newton_soln = &_system->add_vector("newton_soln", false);
  _old_newton_soln = &_system->add_vector("old_newton_soln", false);

  _aux_system = &_es->add_system<TransientExplicitSystem>("AuxiliarySystem");
  _aux_system->attach_init_function(Moose::initial_condition);

  if(_has_displaced_mesh)
    _displaced_aux_system = &_displaced_es->add_system<ExplicitSystem>("DisplacedAuxiliarySystem");

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
MooseSystem::getPostprocessorValue(const std::string & name)
{
  return _postprocessor_data[0].getPostprocessorValue(name);
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

  if(_has_displaced_mesh)
    updateDisplacedMesh(*_system->solution);

  compute_postprocessors(*(_system->current_local_solution));
  output_postprocessors();
}

unsigned int
MooseSystem::addVariable(const std::string &var, const FEType  &type, const std::set< subdomain_id_type  > *const active_subdomains)
{
  unsigned int var_num = 0;
  
  var_num = _system->add_variable(var, type, active_subdomains);

  if(_has_displaced_mesh)
    _displaced_system->add_variable(var, type, active_subdomains);
  
  return var_num;
}

unsigned int
MooseSystem::addVariable(const std::string &var, const Order order, const FEFamily family, const std::set< subdomain_id_type > *const active_subdomains)
{
  unsigned int var_num = 0;

  var_num = _system->add_variable(var, order, family, active_subdomains);
  
  if(_has_displaced_mesh)
    _displaced_system->add_variable(var, order, family, active_subdomains);

  return var_num;
}


unsigned int
MooseSystem::addAuxVariable(const std::string &var, const FEType  &type, const std::set< subdomain_id_type  > *const active_subdomains)
{
  unsigned int var_num = 0;
  
  var_num = _aux_system->add_variable(var, type, active_subdomains);

  if(_has_displaced_mesh)
    _displaced_aux_system->add_variable(var, type, active_subdomains);
  
  return var_num;
}

unsigned int
MooseSystem::addAuxVariable(const std::string &var, const Order order, const FEFamily family, const std::set< subdomain_id_type > *const active_subdomains)
{
  unsigned int var_num = 0;

  var_num = _aux_system->add_variable(var, order, family, active_subdomains);
  
  if(_has_displaced_mesh)
    _displaced_aux_system->add_variable(var, order, family, active_subdomains);

  return var_num;
}

// Kernels ////
void MooseSystem::addKernel(std::string kernel_name,
                            const std::string & name,
                            InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if (!parameters.isParamValid("block"))
    {
      Kernel *kernel = KernelFactory::instance()->create(kernel_name, name, *this, parameters);
      _kernels[tid].addKernel(kernel);
      if (kernel->getParam<bool>("need_old_newton"))
        _need_old_newton = true;
    }
    else
    {
      std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");
    
      for (unsigned int i=0; i<blocks.size(); ++i)
      {
        Kernel *kernel = KernelFactory::instance()->create(kernel_name, name, *this, parameters);
        _kernels[tid].addBlockKernel(blocks[i], kernel);
        if (kernel->getParam<bool>("need_old_newton"))
          _need_old_newton = true;
      }
    }
  }
}

// DGKernels ////
void MooseSystem::addDGKernel(std::string dg_kernel_name,
                              const std::string & name,
                              InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    _dg_kernels[tid].addDGKernel(DGKernelFactory::instance()->create(dg_kernel_name, name, *this, parameters));
  }
}

void
MooseSystem::addBC(std::string bc_name,
                   const std::string & name,
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
        _bcs[tid].addBC(boundaries[i], bc);
      else
        _bcs[tid].addNodalBC(boundaries[i], bc);
    }
  }
}

void
MooseSystem::addAuxKernel(std::string aux_name,
                          const std::string & name,
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
      active_auxs = _auxs[tid].getActiveNodalKernels();
    else
      active_auxs = _auxs[tid].getActiveElementKernels();

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
      _auxs[tid].setActiveNodalKernels(active_auxs);
    else
      _auxs[tid].setActiveElementKernels(active_auxs);
  }
}

void
MooseSystem::addAuxBC(std::string aux_name,
                      const std::string & name,
                      InputParameters parameters)
{
  AuxKernel * aux;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    std::vector<unsigned int> boundaries = parameters.get<std::vector<unsigned int> >("boundary");

    aux = AuxFactory::instance()->create(aux_name, name, *this, parameters);

    for (unsigned int i=0; i<boundaries.size(); ++i)
      _auxs[tid].addActiveBC(boundaries[i], aux);
    _auxs[tid].addBC(aux);
  }
}

void
MooseSystem::addMaterial(std::string mat_name,
                         const std::string & name,
                         InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    std::vector<unsigned int> blocks = parameters.get<std::vector<unsigned int> >("block");

    // We need to set 'variable' in order to initialize other member data, since Material inherits from PDEBase
    parameters.set<std::string>("variable") = _es->get_system(0).variable_name(0);

    for (unsigned int i=0; i<blocks.size(); ++i) {
      parameters.set<unsigned int>("block_id") = blocks[i];

      parameters.set<QuadraturePointData *>("_qp_data") = _element_data[tid];
      parameters.set<MaterialData *>("_material_data") = &_material_data[tid];
      _materials[tid].addMaterial(blocks[i], MaterialFactory::instance()->create(mat_name, name, *this, parameters));

      parameters.set<QuadraturePointData *>("_qp_data") = _face_data[tid];
      parameters.set<MaterialData *>("_material_data") = &_bnd_material_data[tid];
      _materials[tid].addBoundaryMaterial(blocks[i], MaterialFactory::instance()->create(mat_name, name, *this, parameters));
      parameters.set<MaterialData *>("_material_data") = &_neighbor_material_data[tid];
      _materials[tid].addNeighborMaterial(blocks[i], MaterialFactory::instance()->create(mat_name, name, *this, parameters));
    }
  }
}

void
MooseSystem::addStabilizer(std::string stabilizer_name,
                           const std::string & name,
                           InputParameters parameters)
{
  Stabilizer * stabilizer;

  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    stabilizer = StabilizerFactory::instance()->create(stabilizer_name, name, *this, parameters);

    if (parameters.have_parameter<unsigned int>("block_id"))
      _stabilizers[tid].addBlockStabilizer(parameters.get<unsigned int>("block_id"), stabilizer->variable(), stabilizer);
    else
      _stabilizers[tid].addStabilizer(stabilizer->variable(), stabilizer);
  }
}

void
MooseSystem::addInitialCondition(std::string ic_name,
                                 const std::string & name,
                                 InputParameters parameters,
                                 std::string var_name)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // The var_name needs to be added to the parameters object for any InitialCondition derived objects
    parameters.set<std::string>("var_name") = var_name;

    _ics[tid].addIC(var_name, InitialConditionFactory::instance()->create(ic_name, name, *this, parameters));
  }
}

void
MooseSystem::addPostprocessor(std::string pp_name,
                              const std::string & name,
                              InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if(parameters.have_parameter<std::vector<unsigned int> >("boundary"))
    {
      const std::vector<unsigned int> & boundaries = parameters.get<std::vector<unsigned int> >("boundary");
      
      for (unsigned int i=0; i<boundaries.size(); ++i)
      {
        parameters.set<unsigned int>("_boundary_id") = boundaries[i];
        Postprocessor * pp = PostprocessorFactory::instance()->create(pp_name, name, *this, parameters);
        _pps[tid].addPostprocessor(pp);
      }
    }
    else
    {
      Postprocessor * pp = PostprocessorFactory::instance()->create(pp_name, name, *this, parameters);
      _pps[tid].addPostprocessor(pp);
    }
  }
}

void
MooseSystem::addFunction(std::string func_name,
                              const std::string & name,
                              InputParameters parameters)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    _functions[tid].addFunction(name, FunctionFactory::instance()->create(func_name, name, *this, parameters));
  }
}

void
MooseSystem::addDamper(std::string damper_name,
                       const std::string & name,
                       InputParameters parameters)
{
  _has_dampers = true;
  
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    _dampers[tid].addDamper(DamperFactory::instance()->create(damper_name, name, *this, parameters));
  }
}

void
MooseSystem::reinitKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke)
{
//  Moose::perf_log.push("reinit() - dof_indices","Kernel");

  _dof_data[tid]._current_elem = elem;

  _dof_map->dof_indices(elem, _dof_data[tid]._dof_indices);
  
  std::map<FEType, FEBase*>::iterator fe_it = _element_data[tid]->_fe.begin();
  std::map<FEType, FEBase*>::iterator fe_end = _element_data[tid]->_fe.end();

  
  std::map<FEType, FEBase*>::iterator fe_displaced_it; 
  std::map<FEType, FEBase*>::iterator fe_displaced_end;

  if(_has_displaced_mesh)
  { 
    fe_displaced_it = _element_data[tid]->_fe_displaced.begin();
    fe_displaced_end = _element_data[tid]->_fe_displaced.end();
  }
  
//  Moose::perf_log.pop("reinit() - dof_indices","Kernel");
//  Moose::perf_log.push("reinit() - fereinit","Kernel");

  if(!dontReinitFE() || _first[tid])
  {
    for(;fe_it != fe_end; ++fe_it)
      fe_it->second->reinit(elem);

    if(_has_displaced_mesh)
    {
      for(;fe_displaced_it != fe_displaced_end; ++fe_displaced_it)
        fe_displaced_it->second->reinit(_displaced_mesh->elem(elem->id()));
    }
  }
  
  _first[tid] = false;

//  Moose::perf_log.pop("reinit() - fereinit","Kernel");

//  Moose::perf_log.push("reinit() - resizing","Kernel");
  if(Re) Re->resize(_dof_data[tid]._dof_indices.size());
  if(Ke) Ke->resize(_dof_data[tid]._dof_indices.size(), _dof_data[tid]._dof_indices.size());

  unsigned int position = 0;

  for(unsigned int i=0; i<_element_data[tid]->_var_nums.size();i++)
  {
    _dof_map->dof_indices(elem, _dof_data[tid]._var_dof_indices[i], i);

    unsigned int num_dofs = _dof_data[tid]._var_dof_indices[i].size();
    if(Re) _dof_data[tid].reinitRes(i, *Re, position, num_dofs);
    if(Ke) _dof_data[tid].reinitKes(i, num_dofs);

    position+=num_dofs;
  }

  unsigned int num_q_points = _element_data[tid]->_qrule->n_points();

  _real_zero[tid] = 0;
  _zero[tid].resize(num_q_points,0);
  _grad_zero[tid].resize(num_q_points,0);
  _second_zero[tid].resize(num_q_points,0);

  for(std::set<unsigned int>::iterator it = _element_data[tid]->_var_nums.begin(); it != _element_data[tid]->_var_nums.end(); ++it)
  {
    unsigned int var_num = *it;
    FEType fe_type = _dof_map->variable_type(var_num);
    // Copy phi to the test functions.
    const std::vector<std::vector<Real> > & static_phi = *_element_data[tid]->_phi[fe_type];
    _element_data[tid]->_test[var_num] = static_phi;
  }
//  Moose::perf_log.pop("reinit() - resizing","Kernel");

  _element_data[tid]->reinitKernels(soln, elem, Re, Ke);
  if (_need_old_newton)
    _element_data[tid]->reinitNewtonStep(*_old_newton_soln);
}

void
MooseSystem::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const unsigned int boundary_id)
{
  _face_data[tid]->reinit(soln, elem, side, boundary_id);
  _face_data[tid]->reinitMaterials(_materials[tid].getBoundaryMaterials(elem->subdomain_id()), side);
}

void
MooseSystem::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
  _face_data[tid]->reinit(soln, node, boundary_id, residual);
}

void
MooseSystem::reinitDGKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const Elem * neighbor, DenseVector<Number> * Re, bool reinitKe)
{
  // current element
  _face_data[tid]->_current_side = side;
  _face_data[tid]->_current_side_elem = elem->build_side(side).release();

  // loop over variables and reinit FE objects
  std::set<unsigned int>::iterator var_num_it = _face_data[tid]->_var_nums.begin();
  std::set<unsigned int>::iterator var_num_end = _face_data[tid]->_var_nums.end();
  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _dof_map->variable_type(var_num);
    _face_data[tid]->_fe[fe_type]->reinit(elem, side);
  }

  ((QuadraturePointData *) _face_data[tid])->reinit(soln, elem);

  _face_data[tid]->reinitMaterials(_materials[tid].getBoundaryMaterials(elem->subdomain_id()), side);

  // neighbor stuff

  _neighbor_dof_data[tid]._current_elem = neighbor;
  _dof_map->dof_indices(neighbor, _neighbor_dof_data[tid]._dof_indices);

  if(Re) Re->resize(_neighbor_dof_data[tid]._dof_indices.size());

  unsigned int position = 0;

  var_num_it = _face_data[tid]->_var_nums.begin();
  var_num_end = _face_data[tid]->_var_nums.end();
  for(;var_num_it != var_num_end; ++var_num_it)
  {
    unsigned int var_num = *var_num_it;

    FEType fe_type = _dof_map->variable_type(var_num);

    // Find locations of quad points on the neighbor
    std::vector<Point> qface_neighbor_point;
    libMesh::FEInterface::inverse_map (elem->dim(), fe_type, neighbor, (*_face_data[tid]->_q_point[fe_type]), qface_neighbor_point);
    // Calculate the neighbor element shape functions at those locations
    _neighbor_face_data[tid]->_fe[fe_type]->reinit(neighbor, &qface_neighbor_point);

    _dof_map->dof_indices(neighbor, _neighbor_dof_data[tid]._var_dof_indices[var_num], var_num);

    unsigned int num_e_dofs = _dof_data[tid]._var_dof_indices[var_num].size();
    unsigned int num_n_dofs = _neighbor_dof_data[tid]._var_dof_indices[var_num].size();

    if(Re) _neighbor_dof_data[tid].reinitRes(var_num, *Re, position, num_n_dofs);

    if(reinitKe)
    {
      _dof_data[tid].reinitKns(var_num, num_e_dofs, num_n_dofs);
      _neighbor_dof_data[tid].reinitKes(var_num, num_n_dofs);
      _neighbor_dof_data[tid].reinitKns(var_num, num_n_dofs, num_e_dofs);
    }

    position+=num_n_dofs;
  }
  ((QuadraturePointData *) _neighbor_face_data[tid])->reinit(soln, neighbor);

   // passing side here is WRONG, needs to be fixed somehow, should be the side from the other element, but that
   // is not going to work for adaptivity.
   _neighbor_face_data[tid]->reinitMaterials(_materials[tid].getNeighborMaterials(neighbor->subdomain_id()), side);
}

void
MooseSystem::reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node)
{
  _face_data[tid]->_current_node = &node;
  _aux_data[tid]->reinit(soln, node);
}

void
MooseSystem::reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem)
{
  _aux_data[tid]->reinit(soln, elem);
}

void
MooseSystem::reinitDampers(THREAD_ID tid, const NumericVector<Number>& increment)
{
  _damper_data[tid]->reinit(increment);
}  

void
MooseSystem::updateNewtonStep()
{
  if (_need_old_newton)
  {
    *_old_newton_soln = *_newton_soln;
    *_newton_soln = *_system->solution;
  }
}

void
MooseSystem::updateMaterials()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _materials[tid].updateMaterialDataState();
}

void
MooseSystem::subdomainSetup(THREAD_ID tid, unsigned int block_id)
{
  _element_data[tid]->_material = _materials[tid].getMaterials(block_id);
  _face_data[tid]->_material = _materials[tid].getBoundaryMaterials(block_id);

  // call subdomainSetup
  for (std::vector<Material *>::iterator it = _element_data[tid]->_material.begin(); it != _element_data[tid]->_material.end(); ++it)
    (*it)->subdomainSetup();
  for (std::vector<Material *>::iterator it = _face_data[tid]->_material.begin(); it != _face_data[tid]->_material.end(); ++it)
    (*it)->subdomainSetup();

  //Global Kernels
  KernelIterator kernel_begin = _kernels[tid].activeKernelsBegin();
  KernelIterator kernel_end = _kernels[tid].activeKernelsEnd();
  for(KernelIterator kernel_it=kernel_begin;kernel_it!=kernel_end;kernel_it++)
    (*kernel_it)->subdomainSetup();

  //Kernels on this block
  KernelIterator block_kernel_begin = _kernels[tid].blockKernelsBegin(block_id);
  KernelIterator block_kernel_end = _kernels[tid].blockKernelsEnd(block_id);
  for(KernelIterator block_kernel_it=block_kernel_begin;block_kernel_it!=block_kernel_end;block_kernel_it++)
    (*block_kernel_it)->subdomainSetup();

  //Stabilizers
  StabilizerIterator stabilizer_begin = _stabilizers[tid].activeStabilizersBegin();
  StabilizerIterator stabilizer_end = _stabilizers[tid].activeStabilizersEnd();
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
  for (MaterialIterator i = _materials[0].activeMaterialsBegin(); i != _materials[0].activeMaterialsEnd(); ++i)
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
    
    
    _kernels[0].updateActiveKernels(_t, _dt);
    global_kernels_exist = _kernels[0].activeKernelBlocks(input_subdomains);
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

    _bcs[0].activeBoundaries(input_bcs);  // get the boundaries from the simulation (input file)
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
  if(_es)
    _es->reinit();

  // Rebuild the boundary conditions
  _mesh->boundary_info->build_node_list_from_side_list();

  // Rebuild the active local element range
  delete _active_local_elem_range;
  _active_local_elem_range = NULL;

  // Rebuild the node range
  delete _active_node_range;
  _active_node_range = NULL;

  // Calling this function will rebuild the range.
  getActiveLocalElementRange();

  // Calling this function will rebuild the range.
  getActiveNodeRange();

  // Print out information about the adapated mesh if requested
  if (_print_mesh_changed)
  {
    std::cout << "\nMesh Changed:\n";
    _mesh->print_info();
  }

  //Update the node to elem map
  MeshTools::build_nodes_to_elem_map(*_mesh, node_to_elem_map);
  
  // Lets the output system know that the mesh has changed recently.
  _mesh_changed = true;
}

void
MooseSystem::updateDimension()
{
  _dim = _mesh->mesh_dimension();
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

NodeRange *
MooseSystem::getActiveNodeRange()
{
  if(!_active_node_range)
  {
    if(_has_displaced_mesh)
      _active_node_range = new NodeRange(_displaced_mesh->active_nodes_begin(),
                                         _displaced_mesh->active_nodes_end(), 1);
    else      
      _active_node_range = new NodeRange(_mesh->active_nodes_begin(),
                                         _mesh->active_nodes_end(), 1);
  }

  return _active_node_range;
}

/**
 * Outputs the system.
 */
void
MooseSystem::output_system(unsigned int t_step, Real time)
{
  OStringStream stream_file_base;
  OStringStream stream_file_base_displaced;

  stream_file_base << _file_base << "_";
  stream_file_base_displaced << _file_base << "_displaced_";
  OSSRealzeroright(stream_file_base,4,0,t_step);
  OSSRealzeroright(stream_file_base_displaced,4,0,t_step);

  std::string file_name = stream_file_base.str();
  std::string file_name_displaced = stream_file_base_displaced.str();

  if (_exodus_output)
  {
    std::string exodus_file_name;

    bool adaptivity = _es->parameters.have_parameter<bool>("adaptivity");

//    _mesh_changed = true;
//    adaptivity = true;

    //if the mesh changed we need to write to a new file
    if(_mesh_changed || !_ex_out)
    {
      _num_files++;

      if(_ex_out)
        delete _ex_out;

      _ex_out = new ExodusII_IO(_es->get_mesh());

      // We've captured this change... let's reset the changed bool and then see if it's changed again next time.
      _mesh_changed = false;

      // We're starting over
      _num_in_current_file = 0;
    }

    _num_in_current_file++;

    if(!adaptivity)
      exodus_file_name = _file_base;
    else
    {
      OStringStream exodus_stream_file_base;

      exodus_stream_file_base << _file_base << "_";

      // -1 is so that the first one that comes out is 000
      OSSRealzeroright(exodus_stream_file_base,4,0,_num_files-1);

      exodus_file_name = exodus_stream_file_base.str();
    }

    // The +1 is because Exodus starts timesteps at 1 and we start at 0
    _ex_out->write_timestep(exodus_file_name + ".e", *_es, _num_in_current_file, time);

    if(_has_displaced_mesh)
    {
      _num_files_displaced++;

      ExodusII_IO displaced_ex_out(_displaced_es->get_mesh());

      OStringStream exodus_stream_file_base;

      exodus_stream_file_base << _file_base << "_displaced_";

      // -1 is so that the first one that comes out is 000
      OSSRealzeroright(exodus_stream_file_base,4,0,_num_files_displaced-1);

      exodus_file_name = exodus_stream_file_base.str();

      displaced_ex_out.write_timestep(exodus_file_name + ".e", *_displaced_es, _num_in_current_file_displaced, time);      
    }
  }

  if(_gmv_output)
  {
    GMVIO(*_mesh).write_equation_systems(file_name + ".gmv", *_es);
    
    if(_has_displaced_mesh)
      GMVIO(*_displaced_mesh).write_equation_systems(file_name_displaced + ".gmv", *_displaced_es);
  }
  
      
  if(_tecplot_output)
  {
    TecplotIO(*_mesh).write_equation_systems(file_name + ".plt", *_es);

    if(_has_displaced_mesh)
      TecplotIO(*_displaced_mesh).write_equation_systems(file_name_displaced + ".plt", *_displaced_es);
  }

  if(_xda_output)
  {
    _mesh->write(file_name+"_mesh.xda");
    _es->write (file_name+".xda", libMeshEnums::WRITE);
  }
  
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
