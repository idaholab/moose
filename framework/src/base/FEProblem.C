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

#include "FEProblem.h"
#include "Factory.h"
#include "MooseUtils.h"
#include "DisplacedProblem.h"
#include "MaterialData.h"
#include "ComputeUserObjectsThread.h"
#include "ComputeNodalUserObjectsThread.h"
#include "ComputeMaterialsObjectThread.h"
#include "ProjectMaterialProperties.h"
#include "ComputeIndicatorThread.h"
#include "ComputeMarkerThread.h"
#include "ComputeInitialConditionThread.h"
#include "ComputeBoundaryInitialConditionThread.h"
#include "MaxQpsThread.h"
#include "ActionWarehouse.h"
#include "Conversion.h"
#include "Material.h"
#include "ConstantIC.h"
#include "Parser.h"
#include "ElementH1Error.h"
#include "Function.h"
#include "Material.h"
#include "PetscSupport.h"
#include "RandomInterface.h"
#include "RandomData.h"
#include "EigenSystem.h"
#include "MooseParsedFunction.h"
#include "MeshChangedInterface.h"
#include "ComputeJacobianBlocksThread.h"

#include "ScalarInitialCondition.h"
#include "ElementPostprocessor.h"
#include "NodalPostprocessor.h"
#include "SidePostprocessor.h"
#include "InternalSidePostprocessor.h"
#include "GeneralPostprocessor.h"
#include "ElementVectorPostprocessor.h"
#include "NodalVectorPostprocessor.h"
#include "SideVectorPostprocessor.h"
#include "InternalSideVectorPostprocessor.h"
#include "GeneralVectorPostprocessor.h"
#include "Indicator.h"
#include "Marker.h"

#include "MultiApp.h"
#include "TransientMultiApp.h"

#include "ElementUserObject.h"
#include "NodalUserObject.h"
#include "SideUserObject.h"
#include "InternalSideUserObject.h"
#include "GeneralUserObject.h"

#include "InternalSideIndicator.h"

#include "Transfer.h"
#include "MultiAppTransfer.h"
#include "MultiMooseEnum.h"

//libmesh Includes
#include "libmesh/exodusII_io.h"

unsigned int FEProblem::_n = 0;

static
std::string name_sys(const std::string & name, unsigned int n)
{
  std::ostringstream os;
  os << name << n;
  return os.str();
}

Threads::spin_mutex get_function_mutex;

template<>
InputParameters validParams<FEProblem>()
{
  InputParameters params = validParams<SubProblem>();
  params.addPrivateParam<MooseMesh *>("mesh");
  params.addParam<unsigned int>("dimNullSpace", 0, "The dimension of the nullspace");
  params.addParam<unsigned int>("dimNearNullSpace", 0, "The dimension of the near nullspace");
  params.addParam<bool>("solve", true, "Whether or not to actually solve the Nonlinear system.  This is handy in the case that all you want to do is execute AuxKernels, Transfers, etc. without actually solving anything");
  params.addParam<bool>("use_nonlinear", true, "Determines whether to use a Nonlinear vs a Eigenvalue system (Automatically determined based on executioner)");
  params.addParam<bool>("error_on_jacobian_nonzero_reallocation", false, "This causes PETSc to error if it had to reallocate memory in the Jacobian matrix due to not having enough nonzeros");
  return params;
}

/// Inject this object's self into it's own parameters
InputParameters & injectFEProblem(FEProblem * fe_problem, InputParameters & parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = fe_problem;
  return parameters;
}

FEProblem::FEProblem(const std::string & name, InputParameters parameters) :
    SubProblem(name, parameters),
    Restartable(injectFEProblem(this, parameters), "FEProblem"),
    _mesh(*parameters.get<MooseMesh *>("mesh")),
    _eq(_mesh),
    _initialized(false),
    _kernel_type(Moose::KT_ALL),
    _current_boundary_id(Moose::INVALID_BOUNDARY_ID),
    _solve(getParam<bool>("solve")),

    _transient(false),
    _time(declareRestartableData<Real>("time")),
    _time_old(declareRestartableData<Real>("time_old")),
    _t_step(declareRecoverableData<int>("t_step")),
    _dt(declareRestartableData<Real>("dt")),
    _dt_old(declareRestartableData<Real>("dt_old")),

    _nl(getParam<bool>("use_nonlinear") ? *(new NonlinearSystem(*this, name_sys("nl", _n))) : *(new EigenSystem(*this, name_sys("nl", _n)))),
    _aux(*this, name_sys("aux", _n)),
    _coupling(Moose::COUPLING_DIAG),
    _cm(NULL),
#ifdef LIBMESH_ENABLE_AMR
    _adaptivity(*this),
#endif
    _displaced_mesh(NULL),
    _displaced_problem(NULL),
    _geometric_search_data(*this, _mesh),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _input_file_saved(false),
    _has_dampers(false),
    _has_constraints(false),
    _has_multiapps(false),
    _has_initialized_stateful(false),
    _resurrector(NULL),
    _const_jacobian(false),
    _has_jacobian(false),
    _kernel_coverage_check(false),
    _max_qps(std::numeric_limits<unsigned int>::max()),
    _use_legacy_uo_aux_computation(_app.legacyUoAuxComputationDefault()),
    _use_legacy_uo_initialization(_app.legacyUoInitializationDefault()),
    _error_on_jacobian_nonzero_reallocation(getParam<bool>("error_on_jacobian_nonzero_reallocation"))
{

#ifdef LIBMESH_HAVE_PETSC
  // put in empty arrays for PETSc options
  this->parameters().set<MultiMooseEnum>("petsc_options") = MultiMooseEnum("", "", true);
  this->parameters().set<std::vector<std::string> >("petsc_inames") = std::vector<std::string>();
  this->parameters().set<std::vector<std::string> >("petsc_values") = std::vector<std::string>();
#endif

  _n++;

  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _dt = 0;
  _dt_old = _dt;

  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i] = new Assembly(_nl, couplingMatrix(), i);

  unsigned int dimNullSpace      = parameters.get<unsigned int>("dimNullSpace");
  unsigned int dimNearNullSpace  = parameters.get<unsigned int>("dimNearNullSpace");
  for (unsigned int i = 0; i < dimNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace builder might march over all nodes
    _nl.addVector("NullSpace"+oss.str(),false,GHOSTED,false);
  }
  _subspace_dim["NullSpace"] = dimNullSpace;
  for (unsigned int i = 0; i < dimNearNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near-nullspace builder might march over all semilocal nodes
    _nl.addVector("NearNullSpace"+oss.str(),false,GHOSTED,false);
  }
  _subspace_dim["NearNullSpace"] = dimNearNullSpace;

  _functions.resize(n_threads);
  _ics.resize(n_threads);
  _materials.resize(n_threads);

  _material_data.resize(n_threads);
  _bnd_material_data.resize(n_threads);
  _neighbor_material_data.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _material_data[i] = new MaterialData(_material_props);
    _bnd_material_data[i] = new MaterialData(_bnd_material_props);
    _neighbor_material_data[i] = new MaterialData(_bnd_material_props);
  }

  _pps_data.resize(n_threads);

  for (unsigned int i=0; i<n_threads; i++)
    _pps_data[i] = new PostprocessorData(*this, i);

  _vpps_data.resize(n_threads);

  for (unsigned int i=0; i<n_threads; i++)
    _vpps_data[i] = new VectorPostprocessorData(*this, i);

  _objects_by_name.resize(n_threads);

  _indicators.resize(n_threads);
  _markers.resize(n_threads);

  _active_elemental_moose_variables.resize(n_threads);

  _block_mat_side_cache.resize(n_threads);
  _bnd_mat_side_cache.resize(n_threads);

  _resurrector = new Resurrector(*this);

  _eq.parameters.set<FEProblem *>("_fe_problem") = this;
}

FEProblem::~FEProblem()
{
  // Flush the Console stream, the underlying call to Console::mooseConsole
  // relies on a call to Output::checkInterval that has references to
  // _time, etc. If it is not flush here memory problems arise if you have
  // an unflushed stream and start destructing things.
  _console << std::flush;

  delete _cm;
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _assembly[i];

    delete _material_data[i];
    delete _bnd_material_data[i];
    delete _neighbor_material_data[i];
  }

  delete _displaced_problem;
  delete &_nl;

  for (unsigned int i=0; i<n_threads; i++)
    delete _pps_data[i];

  for (unsigned int i=0; i<n_threads; i++)
    delete _vpps_data[i];

  delete _resurrector;

  // Random data objects
  for (std::map<std::string, RandomData *>::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end(); ++it)
    delete it->second;

}

Moose::CoordinateSystemType
FEProblem::getCoordSystem(SubdomainID sid)
{
  std::map<SubdomainID, Moose::CoordinateSystemType>::iterator it = _coord_sys.find(sid);
  if (it != _coord_sys.end())
    return (*it).second;
  else
  {
    std::stringstream err;
    err << "Requested subdomain "
        << sid
        << " does not exist.";
    mooseError(err.str());
  }
}

void
FEProblem::setCoordSystem(const std::vector<SubdomainName> & blocks, const MultiMooseEnum & coord_sys)
{
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();
  if (blocks.size() == 0)
  {
    // no blocks specified -> assume the whole domain
    Moose::CoordinateSystemType coord_type = Moose::COORD_XYZ;                          // all is going to be XYZ by default
    if (coord_sys.size() == 0)
      ; // relax, do nothing
    else if (coord_sys.size() == 1)
      coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[0]);      // one system specified, the whole domain is going to have that system
    else
      mooseError("Multiple coordinate systems specified, but no blocks given.");

    for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
      _coord_sys[*it] = coord_type;
  }
  else
  {
    if (blocks.size() != coord_sys.size())
      mooseError("Number of blocks and coordinate systems does not match.");

    for (unsigned int i = 0; i < blocks.size(); i++)
    {
      SubdomainID sid = _mesh.getSubdomainID(blocks[i]);
      Moose::CoordinateSystemType coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[i]);
      _coord_sys[sid] = coord_type;
    }

    for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
    {
      SubdomainID sid = *it;
      if (_coord_sys.find(sid) == _coord_sys.end())
        mooseError("Subdomain '" + Moose::stringify(sid) + "' does not have a coordinate system specified.");
    }
  }
}

void FEProblem::setAxisymmetricCoordAxis(const MooseEnum & rz_coord_axis)
{
  _rz_coord_axis = rz_coord_axis;
}

void FEProblem::initialSetup()
{
  // Flush all output to _console that occured during construction of objects
  _app.getOutputWarehouse().mooseConsole();

  // Perform output related setups
  _app.getOutputWarehouse().initialSetup();

  if (_app.isRecovering())
    _resurrector->setRestartFile(_app.getRecoverFileBase());

  if (_app.isRestarting() || _app.isRecovering())
    _resurrector->restartFromFile();
  else
  {
    ExodusII_IO * reader = _mesh.exReader();

    if (reader != NULL)
    {
      _nl.copyVars(*reader);
      _aux.copyVars(*reader);
    }
  }

  // Build Refinement and Coarsening maps for stateful material projections if necessary
  if (_adaptivity.isOn() && (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    Moose::setup_perf_log.push("mesh.buildRefinementAndCoarseningMaps()", "Setup");
    _mesh.buildRefinementAndCoarseningMaps(_assembly[0]);
    Moose::setup_perf_log.pop("mesh.buildRefinementAndCoarseningMaps()", "Setup");
  }

  if (!_app.isRecovering())
  {
    /**
     * If we are not recovering but we are doing restart (_app_setFileRestart() == true) with
     * additional uniform refinements. We have to delay the refinement until this point
     * in time so that the equation systems are initialized and projections can be performed.
     */
    if (_mesh.uniformRefineLevel() > 0 && _app.setFileRestart())
    {
      Moose::setup_perf_log.push("Uniformly Refine Mesh","Setup");
      adaptivity().uniformRefineWithProjection();
      Moose::setup_perf_log.pop("Uniformly Refine Mesh","Setup");
    }
  }

  // Do this just in case things have been done to the mesh
  ghostGhostedBoundaries();
  _mesh.meshChanged();
  if (_displaced_problem)
    _displaced_mesh->meshChanged();

  unsigned int n_threads = libMesh::n_threads();

  // UserObject initialSetup
  for (unsigned int i=0; i<n_threads; i++)
  {
    _user_objects(EXEC_LINEAR)[i].updateDependObjects(_aux.getDependObjects(EXEC_RESIDUAL));
    _user_objects(EXEC_NONLINEAR)[i].updateDependObjects(_aux.getDependObjects(EXEC_JACOBIAN));
    _user_objects(EXEC_TIMESTEP_END)[i].updateDependObjects(_aux.getDependObjects(EXEC_TIMESTEP_END));
    _user_objects(EXEC_TIMESTEP_BEGIN)[i].updateDependObjects(_aux.getDependObjects(EXEC_TIMESTEP_BEGIN));
    _user_objects(EXEC_INITIAL)[i].updateDependObjects(_aux.getDependObjects(EXEC_INITIAL));
    _user_objects(EXEC_CUSTOM)[i].updateDependObjects(_aux.getDependObjects(EXEC_CUSTOM));

    _user_objects(EXEC_LINEAR)[i].initialSetup();
    _user_objects(EXEC_NONLINEAR)[i].initialSetup();
    _user_objects(EXEC_TIMESTEP_END)[i].initialSetup();
    _user_objects(EXEC_TIMESTEP_BEGIN)[i].initialSetup();
    _user_objects(EXEC_INITIAL)[i].initialSetup();
    _user_objects(EXEC_CUSTOM)[i].initialSetup();
  }

  // Call the initialSetup methods for functions
  for (unsigned int i=0; i<n_threads; i++)
    for (std::map<std::string, MooseSharedPointer<Function> >::iterator vit = _functions[i].begin(); vit != _functions[i].end(); ++vit)
      vit->second->initialSetup();

  if (!_app.isRecovering())
  {
    for (unsigned int i = 0; i < n_threads; i++)
      _ics[i].initialSetup();
    projectSolution();
  }

  for (unsigned int i=0; i<n_threads; i++)
  {
    _indicators[i].initialSetup();
    _markers[i].initialSetup();
  }

#ifdef LIBMESH_ENABLE_AMR

  if (!_app.isRecovering())
  {
    Moose::setup_perf_log.push("initial adaptivity", "Setup");
    unsigned int n = adaptivity().getInitialSteps();
    for (unsigned int i = 0; i < n; i++)
    {
      _console << "Initial adaptivity step " << i+1 << " of " << n << std::endl;
      computeIndicatorsAndMarkers();

      _adaptivity.initialAdaptMesh();
      meshChanged();

      //reproject the initial condition
      projectSolution();
    }
    Moose::setup_perf_log.pop("initial adaptivity","Setup");
  }

#endif //LIBMESH_ENABLE_AMR

  if (!_app.isRecovering() && !_app.isRestarting())
  {
    // During initial setup the solution is copied to solution_old and solution_older
    Moose::setup_perf_log.push("copySolutionsBackwards()","Setup");
    copySolutionsBackwards();
    Moose::setup_perf_log.pop("copySolutionsBackwards()","Setup");
  }

  for (unsigned int i=0; i<n_threads; i++)
    _materials[i].initialSetup();

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeMaterialsObjectThread cmt(*this, _nl, _material_data, _bnd_material_data, _neighbor_material_data,
                                   _material_props, _bnd_material_props, _materials, _assembly);
  /**
   * The ComputeMaterialObjectThread object now allocates memory as needed for the material storage system.
   * This cannot be done with threads. The first call to this object bypasses threading by calling the object
   * directly. The subsequent call can be called with threads.
   */
  cmt(elem_range, true);

  if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
    _has_initialized_stateful = true;

  // Auxilary variable initialSetup calls
  _aux.initialSetup();

  if (_app.isRestarting() || _app.isRecovering())
  {
    // now if restarting and we have stateful material properties, go overwrite the values with the ones
    // from the restart file.  We need to do it this way, since we have no idea about sizes of user-defined material
    // properties (i.e. things like std:vector<std::vector<SymmTensor> >)
    if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
    {
      // load the stateful material props from a file
      _resurrector->restartStatefulMaterialProps();
    }

    // TODO: Reenable restarting for UserObjects!
//    if (_user_objects[0].size() > 0)
//      _resurrector->restartUserData();
  }

//  // RUN initial postprocessors
//  computePostprocessors(EXEC_INITIAL);

  _nl.setSolution(*(_nl.sys().current_local_solution.get()));

  Moose::setup_perf_log.push("Initial updateGeomSearch()","Setup");
  // Update the nearest node searches (has to be called after the problem is all set up)
  // We do this here because this sets up the Element's DoFs to ghost
  updateGeomSearch(GeometricSearchData::NEAREST_NODE);
  Moose::setup_perf_log.pop("Initial updateGeomSearch()","Setup");

  Moose::setup_perf_log.push("Initial updateActiveSemiLocalNodeRange()","Setup");
  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);
  if (_displaced_mesh)
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  Moose::setup_perf_log.pop("Initial updateActiveSemiLocalNodeRange()","Setup");

  Moose::setup_perf_log.push("reinit() after updateGeomSearch()","Setup");
  // Possibly reinit one more time to get ghosting correct
  reinitBecauseOfGhosting();
  Moose::setup_perf_log.pop("reinit() after updateGeomSearch()","Setup");


  if (_displaced_mesh)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

  Moose::setup_perf_log.push("Initial updateGeomSearch()","Setup");
  updateGeomSearch(); // Call all of the rest of the geometric searches
  Moose::setup_perf_log.pop("Initial updateGeomSearch()","Setup");

  // Random interface objects
  for (std::map<std::string, RandomData *>::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end();
       ++it)
    it->second->updateSeeds(EXEC_INITIAL);

  // Call init on the MultiApps
  _multi_apps(EXEC_LINEAR)[0].initialSetup();
  _multi_apps(EXEC_NONLINEAR)[0].initialSetup();
  _multi_apps(EXEC_TIMESTEP_END)[0].initialSetup();
  _multi_apps(EXEC_TIMESTEP_BEGIN)[0].initialSetup();
  _multi_apps(EXEC_INITIAL)[0].initialSetup();
  _multi_apps(EXEC_CUSTOM)[0].initialSetup();

  // Call initial setup on the transfers
  _transfers(EXEC_LINEAR)[0].initialSetup();
  _transfers(EXEC_NONLINEAR)[0].initialSetup();
  _transfers(EXEC_TIMESTEP_END)[0].initialSetup();
  _transfers(EXEC_TIMESTEP_BEGIN)[0].initialSetup();
  _transfers(EXEC_INITIAL)[0].initialSetup();
  _transfers(EXEC_CUSTOM)[0].initialSetup();

  _to_multi_app_transfers(EXEC_LINEAR)[0].initialSetup();
  _to_multi_app_transfers(EXEC_NONLINEAR)[0].initialSetup();
  _to_multi_app_transfers(EXEC_TIMESTEP_END)[0].initialSetup();
  _to_multi_app_transfers(EXEC_TIMESTEP_BEGIN)[0].initialSetup();
  _to_multi_app_transfers(EXEC_INITIAL)[0].initialSetup();
  _to_multi_app_transfers(EXEC_CUSTOM)[0].initialSetup();

  _from_multi_app_transfers(EXEC_LINEAR)[0].initialSetup();
  _from_multi_app_transfers(EXEC_NONLINEAR)[0].initialSetup();
  _from_multi_app_transfers(EXEC_TIMESTEP_END)[0].initialSetup();
  _from_multi_app_transfers(EXEC_TIMESTEP_BEGIN)[0].initialSetup();
  _from_multi_app_transfers(EXEC_INITIAL)[0].initialSetup();
  _from_multi_app_transfers(EXEC_CUSTOM)[0].initialSetup();

  if (!_app.isRecovering())
  {
    Moose::setup_perf_log.push("Initial execTransfers()","Setup");
    execTransfers(EXEC_INITIAL);
    Moose::setup_perf_log.pop("Initial execTransfers()","Setup");

    Moose::setup_perf_log.push("Initial execMultiApps()","Setup");
    execMultiApps(EXEC_INITIAL);
    Moose::setup_perf_log.pop("Initial execMultiApps()","Setup");

    Moose::setup_perf_log.push("Initial computeUserObjects()","Setup");

    computeUserObjects(EXEC_INITIAL, UserObjectWarehouse::PRE_AUX);
    _aux.compute(EXEC_INITIAL);

    if (_use_legacy_uo_initialization)
    {
      _aux.compute(EXEC_TIMESTEP_BEGIN);
      computeUserObjects(EXEC_TIMESTEP_END);
    }

    // The only user objects that should be computed here are the initial UOs
    computeUserObjects(EXEC_INITIAL, UserObjectWarehouse::POST_AUX);

    if (_use_legacy_uo_initialization)
    {
      computeUserObjects(EXEC_TIMESTEP_BEGIN);
      computeUserObjects(EXEC_LINEAR);
    }
    Moose::setup_perf_log.pop("Initial computeUserObjects()","Setup");
  }

  _nl.initialSetupBCs();
  _nl.initialSetupKernels();

  _nl.initialSetup();

  // Here we will initialize the stateful properties once more since they may have been updated
  // during initialSetup by calls to computeProperties.
  if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
  {
    if (_app.isRestarting() || _app.isRecovering())
      _resurrector->restartStatefulMaterialProps();
    else
    {
      ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
      ComputeMaterialsObjectThread cmt(*this, _nl, _material_data, _bnd_material_data, _neighbor_material_data,
                                       _material_props, _bnd_material_props, _materials, _assembly);
      Threads::parallel_reduce(elem_range, cmt);
    }
  }

  if (_app.isRestarting() || _app.isRecovering())
    _resurrector->restartRestartableData();

  // Scalar variables need to reinited for the initial conditions to be available for output
  for (unsigned int tid = 0; tid < n_threads; tid++)
    reinitScalars(tid);

  if (_displaced_mesh)
    _displaced_problem->syncSolutions(*_nl.currentSolution(), *_aux.currentSolution());

  // Writes all calls to _console from initialSetup() methods
  _app.getOutputWarehouse().mooseConsole();
}

void FEProblem::timestepSetup()
{
  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].timestepSetup();

    for (std::map<std::string, MooseSharedPointer<Function> >::iterator vit = _functions[i].begin();
        vit != _functions[i].end();
        ++vit)
      vit->second->timestepSetup();
  }

  _aux.timestepSetup();
  _nl.timestepSetup();

  for (unsigned int i=0; i<n_threads; i++)
  {
    _indicators[i].timestepSetup();
    _markers[i].timestepSetup();

    // Random interface objects
    for (std::map<std::string, RandomData *>::iterator it = _random_data_objects.begin();
         it != _random_data_objects.end();
         ++it)
      it->second->updateSeeds(EXEC_TIMESTEP_BEGIN);

    // Timestep setup of all UserObjects
    for (unsigned int j = 0; j < Moose::exec_types.size(); j++)
      _user_objects(Moose::exec_types[j])[i].timestepSetup();
  }

   // Timestep setup of output objects
  _app.getOutputWarehouse().timestepSetup();
}

unsigned int
FEProblem::getMaxQps() const
{
  if (_max_qps == std::numeric_limits<unsigned int>::max())
    mooseError("Max QPS uninitialized");
  return _max_qps;
}

void
FEProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), tid);
}

void
FEProblem::prepareFace(const Elem * elem, THREAD_ID tid)
{
  _nl.prepareFace(tid, true);
  _aux.prepareFace(tid, false);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepareFace(_displaced_mesh->elem(elem->id()), tid);
}

void
FEProblem::prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<dof_id_type> & dof_indices, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _nl.prepare(tid);
  _aux.prepare(tid);
  _assembly[tid]->prepareBlock(ivar, jvar, dof_indices);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepare(_displaced_mesh->elem(elem->id()), ivar, jvar, dof_indices, tid);
}

void
FEProblem::prepareAssembly(THREAD_ID tid)
{
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepareAssembly(tid);
}

NumericVector<Number> &
FEProblem::residualVector(Moose::KernelType type)
{
  return _nl.residualVector(type);
}

void
FEProblem::addResidual(THREAD_ID tid)
{
  _assembly[tid]->addResidual(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addResidual(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);

  if (_displaced_problem)
    _displaced_problem->addResidual(tid);
}

void
FEProblem::addResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->addResidualNeighbor(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addResidualNeighbor(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
  if (_displaced_problem)
    _displaced_problem->addResidualNeighbor(tid);
}

void
FEProblem::addResidualScalar(THREAD_ID tid/* = 0*/)
{
  _assembly[tid]->addResidualScalar(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addResidualScalar(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
}

void
FEProblem::cacheResidual(THREAD_ID tid)
{
  _assembly[tid]->cacheResidual();
  if (_displaced_problem)
    _displaced_problem->cacheResidual(tid);
}

void
FEProblem::cacheResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheResidualNeighbor();
  if (_displaced_problem)
    _displaced_problem->cacheResidualNeighbor(tid);
}

void
FEProblem::addCachedResidual(THREAD_ID tid)
{
  _assembly[tid]->addCachedResidual(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  _assembly[tid]->addCachedResidual(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);

  if (_displaced_problem)
    _displaced_problem->addCachedResidual(tid);
}

void
FEProblem::addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->addCachedResidual(residual, Moose::KT_TIME);
  _assembly[tid]->addCachedResidual(residual, Moose::KT_NONTIME);

  if (_displaced_problem)
    _displaced_problem->addCachedResidualDirectly(residual, tid);
}

void
FEProblem::setResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidual(residual);
  if (_displaced_problem)
    _displaced_problem->setResidual(residual, tid);
}

void
FEProblem::setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidualNeighbor(residual);
  if (_displaced_problem)
    _displaced_problem->setResidualNeighbor(residual, tid);
}

void
FEProblem::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobian(jacobian);
  if (_displaced_problem)
    _displaced_problem->addJacobian(jacobian, tid);
}

void
FEProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian);
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighbor(jacobian, tid);
}

void
FEProblem::addJacobianScalar(SparseMatrix<Number> & jacobian, THREAD_ID tid/* = 0*/)
{
  _assembly[tid]->addJacobianScalar(jacobian);
}

void
FEProblem::addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian, unsigned int ivar, THREAD_ID tid/* = 0*/)
{
  _assembly[tid]->addJacobianOffDiagScalar(jacobian, ivar);
}

void
FEProblem::cacheJacobian(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobian();
  if (_displaced_problem)
    _displaced_problem->cacheJacobian(tid);
}

void
FEProblem::cacheJacobianNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobianNeighbor();
  if (_displaced_problem)
    _displaced_problem->cacheJacobianNeighbor(tid);
}

void
FEProblem::addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addCachedJacobian(jacobian);
  if (_displaced_problem)
    _displaced_problem->addCachedJacobian(jacobian, tid);
}

void
FEProblem::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
  if (_displaced_problem)
    _displaced_problem->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, tid);
}

void
FEProblem::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, std::vector<dof_id_type> & neighbor_dof_indices, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighbor(jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices, tid);
}

void
FEProblem::prepareShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyShapes(var);
}

void
FEProblem::prepareFaceShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyFaceShapes(var);
}

void
FEProblem::prepareNeighborShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyNeighborShapes(var);
}

void
FEProblem::addGhostedElem(dof_id_type elem_id)
{
  if (_mesh.elem(elem_id)->processor_id() != processor_id())
    _ghosted_elems.insert(elem_id);
}

void
FEProblem::addGhostedBoundary(BoundaryID boundary_id)
{
  _mesh.addGhostedBoundary(boundary_id);
  if (_displaced_problem)
    _displaced_mesh->addGhostedBoundary(boundary_id);
}

void
FEProblem::ghostGhostedBoundaries()
{
  _mesh.ghostGhostedBoundaries();

  if (_displaced_problem)
    _displaced_mesh->ghostGhostedBoundaries();
}

void
FEProblem::sizeZeroes(unsigned int size, THREAD_ID tid)
{
  _zero[tid].resize(size, 0);
  _grad_zero[tid].resize(size, 0);
  _second_zero[tid].resize(size, RealTensor(0.));
}

bool
FEProblem::reinitDirac(const Elem * elem, THREAD_ID tid)
{
  std::vector<Point> & points = _dirac_kernel_info.getPoints()[elem];

  bool have_points = points.size();

  if (have_points)
  {
    _assembly[tid]->reinitAtPhysical(elem, points);

    _nl.prepare(tid);
    _aux.prepare(tid);

    reinitElem(elem, tid);
  }
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem))
    have_points = have_points || _displaced_problem->reinitDirac(_displaced_mesh->elem(elem->id()), tid);

  return have_points;
}

void
FEProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  sizeZeroes(_assembly[tid]->qRule()->n_points(), tid);

  _nl.reinitElem(elem, tid);
  _aux.reinitElem(elem, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elem(elem->id()), tid);
}

void
FEProblem::reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid)
{
  std::vector<Point> points(phys_points_in_elem.size());
  std::copy(phys_points_in_elem.begin(), phys_points_in_elem.end(), points.begin());

  _assembly[tid]->reinitAtPhysical(elem, points);

  _nl.prepare(tid);
  _aux.prepare(tid);

  reinitElem(elem, tid);
  _assembly[tid]->prepare();

  if (_displaced_problem != NULL && (_reinit_displaced_elem))
    _displaced_problem->reinitElemPhys(_displaced_mesh->elem(elem->id()), phys_points_in_elem, tid);
}

void
FEProblem::reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem, side);

  sizeZeroes(_assembly[tid]->qRule()->n_points(), tid);

  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elem(elem->id()), side, bnd_id, tid);
}

void
FEProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);

  sizeZeroes(1, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNode(&_displaced_mesh->node(node->id()), tid);

  _nl.reinitNode(node, tid);
  _aux.reinitNode(node, tid);
}

void
FEProblem::reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);

  sizeZeroes(1, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitNodeFace(&_displaced_mesh->node(node->id()), bnd_id, tid);

  _nl.reinitNodeFace(node, bnd_id, tid);
  _aux.reinitNodeFace(node, bnd_id, tid);

}

void
FEProblem::reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  sizeZeroes(nodes.size(), tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodes(nodes, tid);

  _nl.reinitNodes(nodes, tid);
  _aux.reinitNodes(nodes, tid);
}

void
FEProblem::reinitNodeNeighbor(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinitNodeNeighbor(node);

  sizeZeroes(1, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodeNeighbor(&_displaced_mesh->node(node->id()), tid);

  _nl.reinitNodeNeighbor(node, tid);
  _aux.reinitNodeNeighbor(node, tid);
}

void
FEProblem::reinitScalars(THREAD_ID tid)
{
  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitScalars(tid);

  _nl.reinitScalars(tid);
  _aux.reinitScalars(tid);

  _assembly[tid]->prepareScalar();
}

void
FEProblem::reinitOffDiagScalars(THREAD_ID tid)
{
  _assembly[tid]->prepareOffDiagScalar();
}


void
FEProblem::reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  const Elem * neighbor = elem->neighbor(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  _assembly[tid]->reinitElemAndNeighbor(elem, side, neighbor, neighbor_side);

  _nl.prepareNeighbor(tid);
  _aux.prepareNeighbor(tid);

  _assembly[tid]->prepareNeighbor();

  BoundaryID bnd_id = 0;              // some dummy number (it is not really used for anything, right now)
  _nl.reinitElemFace(elem, side, bnd_id, tid);
  _aux.reinitElemFace(elem, side, bnd_id, tid);

  _nl.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
  _aux.reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitNeighbor(elem, side, tid);
}

void
FEProblem::reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid)
{
  // Reinits shape the functions at the physical points
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, neighbor_side, physical_points);

  // Sets the neighbor dof indices
  _nl.prepareNeighbor(tid);
  _aux.prepareNeighbor(tid);

  // Resizes Re and Ke
  _assembly[tid]->prepareNeighbor();

  // Compute the values of each variable at the points
  _nl.reinitNeighbor(neighbor, tid);
  _aux.reinitNeighbor(neighbor, tid);

  // Do the same for the displaced problem
  if (_displaced_problem != NULL)
    _displaced_problem->reinitNeighborPhys(_displaced_mesh->elem(neighbor->id()), neighbor_side, physical_points, tid);
}

void
FEProblem::getDiracElements(std::set<const Elem *> & elems)
{
  // First add in the undisplaced elements
  elems = _dirac_kernel_info.getElements();

  if (_displaced_problem)
  {
    std::set<const Elem *> displaced_elements;
    _displaced_problem->getDiracElements(displaced_elements);

    { // Use the ids from the displaced elements to get the undisplaced elements
      // and add them to the list
      std::set<const Elem *>::iterator it = displaced_elements.begin();
      std::set<const Elem *>::iterator end = displaced_elements.end();

      for (;it != end; ++it)
        elems.insert(_mesh.elem((*it)->id()));
    }
  }
}

void
FEProblem::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();

  if (_displaced_problem)
    _displaced_problem->clearDiracInfo();
}


void
FEProblem::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(subdomain))
  {
    // call subdomainSetup
    for (std::vector<Material *>::const_iterator it = _materials[tid].getMaterials(subdomain).begin(); it != _materials[tid].getMaterials(subdomain).end(); ++it)
      (*it)->subdomainSetup();

    // Need to reinitialize the material properties in case subdomain setup for a Kernel needs it
    // TODO: This means we are doing one more materialReinit than is necessary.  Need to refactor this to
    // keep that from happening

    // FIXME: cannot do this b/c variables are not computed => potential NaNs will pop-up
//    reinitMaterials(subdomain, tid);
  }

  // Call the subdomain methods of the output system, these are not threaded so only call it once
  if (tid == 0)
    _app.getOutputWarehouse().subdomainSetup();


  _nl.subdomainSetup(subdomain, tid);

  // FIXME: call displaced_problem->subdomainSetup() ?
  //        When adding possibility with materials being evaluated on displaced mesh
}

void
FEProblem::subdomainSetupSide(SubdomainID subdomain, THREAD_ID tid)
{
  if (_materials[tid].hasBoundaryMaterials(subdomain))
  {
    // call subdomainSetup
    for (std::vector<Material *>::const_iterator it = _materials[tid].getBoundaryMaterials(subdomain).begin(); it != _materials[tid].getBoundaryMaterials(subdomain).end(); ++it)
      (*it)->subdomainSetup();

  }
}

const std::vector<MooseObject *> &
FEProblem::getObjectsByName(const std::string & name, THREAD_ID tid)
{
  std::map<std::string, std::vector<MooseObject *> >::iterator it = _objects_by_name[tid].find(name);
  if (it != _objects_by_name[tid].end())
    return (*it).second;
  else
    mooseError("Unable to find objects with a given name.");
}

void
FEProblem::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    MooseSharedPointer<Function> func = MooseSharedNamespace::static_pointer_cast<Function>(_factory.create(type, name, parameters));
    if (_functions[tid].find(name) != _functions[tid].end())
      mooseError("Duplicate function name added to FEProblem: " << name);
    _functions[tid][name] = func;
    _objects_by_name[tid][name].push_back(func.get());
  }
}

bool
FEProblem::hasFunction(const std::string & name, THREAD_ID tid)
{
  return (_functions[tid].find(name) != _functions[tid].end());
}

Function &
FEProblem::getFunction(const std::string & name, THREAD_ID tid)
{
  // This thread lock is necessary since this method will create functions
  // for all threads if one is missing.
  Threads::spin_mutex::scoped_lock lock(get_function_mutex);

  if (!hasFunction(name, tid))
  {
    // If we didn't find a function, it might be a default function, attempt to construct one now
    std::istringstream ss(name);
    Real real_value;

    // First see if it's just a constant. If it is, build a ConstantFunction
    if (ss >> real_value && ss.eof())
    {
      InputParameters params = _factory.getValidParams("ConstantFunction");
      params.set<Real>("value") = real_value;
      addFunction("ConstantFunction", ss.str(), params);
    }
    else
    {
      FunctionParserBase<Real> fp;
      std::string vars = "x,y,z,t";
      if (fp.Parse(name, vars) == -1) // -1 for success
      {
        // It parsed ok, so build a MooseParsedFunction
        InputParameters params = _factory.getValidParams("ParsedFunction");
        params.set<std::string>("value") = name;
        addFunction("ParsedFunction", name, params);
      }
    }

    // Try once more
    if (!hasFunction(name, tid))
      mooseError("Unable to find function " + name);
  }

  return *(_functions[tid][name]);
}

void
FEProblem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains/* = NULL*/)
{
  if (_aux.hasVariable(var_name))
    mooseError("Cannot have an auxiliary variable and a nonlinear variable with the same name: " << var_name);

  if (_nl.hasVariable(var_name))
  {
    const Variable & var = _nl.sys().variable(_nl.sys().variable_number(var_name));
    if (var.type() != type)
      mooseError("Variable with name '" << var_name << "' already exists but is of a differing type!");

    return;
  }

  _nl.addVariable(var_name, type, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addVariable(var_name, type, scale_factor, active_subdomains);
}

void
FEProblem::addScalarVariable(const std::string & var_name, Order order, Real scale_factor, const std::set< SubdomainID > * const active_subdomains)
{
  _nl.addScalarVariable(var_name, order, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
FEProblem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addKernel(kernel_name, name, parameters);
}

void
FEProblem::addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addScalarKernel(kernel_name, name, parameters);
}

void
FEProblem::addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addBoundaryCondition(bc_name, name, parameters);
}

void
FEProblem::addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters)
{
  _has_constraints = true;

  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    // It might _want_ to use a displaced mesh... but we're not so set it to false
    if (parameters.have_parameter<bool>("use_displaced_mesh"))
      parameters.set<bool>("use_displaced_mesh") = false;

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addConstraint(c_name, name, parameters);
}

void
FEProblem::addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains/* = NULL*/)
{
  if (_nl.hasVariable(var_name))
    mooseError("Cannot have an auxiliary variable and a nonlinear variable with the same name!");

  if (_aux.hasVariable(var_name))
  {
    const Variable & var = _aux.sys().variable(_aux.sys().variable_number(var_name));
    if (var.type() != type)
      mooseError("AuxVariable with name '" << var_name << "' already exists but is of a differing type!");

    return;
  }

  _aux.addVariable(var_name, type, 1.0, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable(var_name, type, active_subdomains);
}

void
FEProblem::addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor, const std::set< SubdomainID > * const active_subdomains)
{
  _aux.addScalarVariable(var_name, order, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
FEProblem::addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->nlSys();
    if (!parameters.get<std::vector<BoundaryName> >("boundary").empty())
      _reinit_displaced_face = true;
    else
      _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
    parameters.set<SystemBase *>("_nl_sys") = &_nl;
  }
  _aux.addKernel(kernel_name, name, parameters);
}

void
FEProblem::addAuxScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }
  _aux.addScalarKernel(kernel_name, name, parameters);
}

void
FEProblem::addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDiracKernel(kernel_name, name, parameters);
}

// DGKernels ////

void
FEProblem::addDGKernel(const std::string & dg_kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_nl;
  }
  _nl.addDGKernel(dg_kernel_name, name, parameters);
}

void
FEProblem::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{
  // before we start to mess with the initial condition, we need to check parameters for errors.
  parameters.checkParams(name);

  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");
  // field IC
  if (hasVariable(var_name))
  {
    MooseVariable & var = getVariable(0, var_name);
    parameters.set<SystemBase *>("_sys") = &var.sys();

    const std::vector<SubdomainName> & blocks = parameters.get<std::vector<SubdomainName> >("block");
    const std::vector<BoundaryName> & boundaries = parameters.get<std::vector<BoundaryName> >("boundary");

    if (blocks.size() > 0 && boundaries.size() > 0)
    {
      mooseError("Both 'block' and 'boundary' parameters were specified in initial condition '" << name << "'.  You can only you either of them.");
    }
    // boundary-restricted IC
    else if (boundaries.size() > 0 && blocks.size() == 0)
    {
      if (var.isNodal())
      {
        for (unsigned int tid=0; tid < libMesh::n_threads(); tid++)
        {
          parameters.set<THREAD_ID>("_tid") = tid;
          for (unsigned int i = 0; i < boundaries.size(); i++)
          {
            BoundaryID bnd_id = _mesh.getBoundaryID(boundaries[i]);
            _ics[tid].addBoundaryInitialCondition(var_name, bnd_id,
                                                  MooseSharedNamespace::static_pointer_cast<InitialCondition>(_factory.create(ic_name, name, parameters)));
          }
        }
      }
      else
        mooseError("You are trying to set a boundary restricted variable on non-nodal variable.  That is not allowed.");
    }
    // block-restricted IC
    else
    {
      // this means: either no block and no boundary parameters specified or just block specified
      for (unsigned int tid=0; tid < libMesh::n_threads(); tid++)
      {
        parameters.set<THREAD_ID>("_tid") = tid;
        if (blocks.size() > 0)
          for (unsigned int i = 0; i < blocks.size(); i++)
          {
            SubdomainID blk_id = _mesh.getSubdomainID(blocks[i]);
            _ics[tid].addInitialCondition(var_name, blk_id,
                                          MooseSharedNamespace::static_pointer_cast<InitialCondition>(_factory.create(ic_name, name, parameters)));
          }
        else
          _ics[tid].addInitialCondition(var_name, Moose::ANY_BLOCK_ID,
                                        MooseSharedNamespace::static_pointer_cast<InitialCondition>(_factory.create(ic_name, name, parameters)));
      }
    }

  }
  // scalar IC
  else if (hasScalarVariable(var_name))
  {
    MooseVariableScalar & var = getScalarVariable(0, var_name);
    parameters.set<SystemBase *>("_sys") = &var.sys();

    for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    {
      parameters.set<THREAD_ID>("_tid") = tid;
      _ics[tid].addScalarInitialCondition(var_name,
                                          MooseSharedNamespace::static_pointer_cast<ScalarInitialCondition>(_factory.create(ic_name, name, parameters)));
    }
  }
  else
    mooseError("Variable '" << var_name << "' requested in initial condition '" << name << "' does not exist.");
}

void
FEProblem::projectSolution()
{
  Moose::enableFPE();

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeInitialConditionThread cic(*this);
  Threads::parallel_reduce(elem_range, cic);

  // Need to close the solution vector here so that boundary ICs take precendence
  _nl.solution().close();
  _aux.solution().close();

  // now run boundary-restricted initial conditions
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  ComputeBoundaryInitialConditionThread cbic(*this);
  Threads::parallel_reduce(bnd_nodes, cbic);

  _nl.solution().close();
  _aux.solution().close();

  // Also, load values into the SCALAR dofs
  // Note: We assume that all SCALAR dofs are on the
  // processor with highest ID
  if (processor_id() == (n_processors()-1))
  {
    THREAD_ID tid = 0;

    const std::vector<ScalarInitialCondition *> & ics = _ics[tid].activeScalar();
    for (std::vector<ScalarInitialCondition *>::const_iterator it = ics.begin(); it != ics.end(); ++it)
    {
      ScalarInitialCondition * ic = (*it);

      MooseVariableScalar & var = ic->variable();
      var.reinit();

      DenseVector<Number> vals(var.order());
      ic->compute(vals);

      const unsigned int n_SCALAR_dofs = var.dofIndices().size();
      for (unsigned int i = 0; i < n_SCALAR_dofs; i++)
      {
        const dof_id_type global_index = var.dofIndices()[i];
        var.sys().solution().set(global_index, vals(i));
        var.setValue(i, vals(i));
      }
    }
  }

  Moose::enableFPE(false);

  _nl.solution().close();
  _nl.solution().localize(*_nl.sys().current_local_solution, _nl.dofMap().get_send_list());

  _aux.solution().close();
  _aux.solution().localize(*_aux.sys().current_local_solution, _aux.dofMap().get_send_list());
}

void
FEProblem::addMaterial(const std::string & mat_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    _reinit_displaced_elem = true;
  }
  else
    parameters.set<SubProblem *>("_subproblem") = this;

  // Get user-defined list of block names
  std::vector<SubdomainName> blocks = parameters.get<std::vector<SubdomainName> >("block");
  std::vector<SubdomainID> block_ids = _mesh.getSubdomainIDs(blocks);

  // Get user-defined list of boundary names
  std::vector<BoundaryName> boundaries = parameters.get<std::vector<BoundaryName> >("boundary");
  std::vector<BoundaryID> boundary_ids = _mesh.getBoundaryIDs(boundaries);

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    if (block_ids.size() > 0)
    {
      // volume material
      parameters.set<bool>("_bnd") = false;
      parameters.set<bool>("_neighbor") = false;
      parameters.set<MaterialData *>("_material_data") = _material_data[tid];
      MooseSharedPointer<Material> volume_material = MooseSharedNamespace::static_pointer_cast<Material>(_factory.create(mat_name, name, parameters));
      _materials[tid].addMaterial(block_ids, volume_material);
      _objects_by_name[tid][name].push_back(volume_material.get());

      // face material
      parameters.set<bool>("_bnd") = true;
      parameters.set<bool>("_neighbor") = false;
      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
      MooseSharedPointer<Material> face_material = MooseSharedNamespace::static_pointer_cast<Material>(_factory.create(mat_name, name, parameters));
      _materials[tid].addFaceMaterial(block_ids, face_material);
      _objects_by_name[tid][name].push_back(face_material.get());

      // neighbor material
      parameters.set<bool>("_bnd") = true;
      parameters.set<bool>("_neighbor") = true;
      parameters.set<MaterialData *>("_material_data") = _neighbor_material_data[tid];
      MooseSharedPointer<Material> neighbor_material = MooseSharedNamespace::static_pointer_cast<Material>(_factory.create(mat_name, name, parameters));
      _materials[tid].addNeighborMaterial(block_ids, neighbor_material);
      _objects_by_name[tid][name].push_back(neighbor_material.get());
    }
    else if (boundary_ids.size() > 0)
    {
      parameters.set<bool>("_bnd") = true;
      parameters.set<bool>("_neighbor") = false;
      parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
      MooseSharedPointer<Material> bnd_material = MooseSharedNamespace::static_pointer_cast<Material>(_factory.create(mat_name, name, parameters));
      _materials[tid].addBoundaryMaterial(boundary_ids, bnd_material);
      _objects_by_name[tid][name].push_back(bnd_material.get());
    }
    else
      mooseError("Material '" + name + "' did not specify either block or boundary parameter");
  }
}

const std::vector<Material *> &
FEProblem::getMaterialsByName(const std::string & name, THREAD_ID tid)
{
  return _materials[tid].getMaterialsByName(name);
}

const std::vector<Material*> &
FEProblem::getMaterials(SubdomainID block_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getMaterials(block_id);
}

const std::vector<Material*> &
FEProblem::getFaceMaterials(SubdomainID block_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getFaceMaterials(block_id);
}

const std::vector<Material*> &
FEProblem::getBndMaterials(BoundaryID boundary_id, THREAD_ID tid)
{
  mooseAssert( tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getBoundaryMaterials(boundary_id);
}

const std::vector<Material*> &
FEProblem::getNeighborMaterials(SubdomainID block_id, THREAD_ID tid)
{
  mooseAssert(tid < _materials.size(), "Requesting a material warehouse that does not exist");
  return _materials[tid].getNeighborMaterials(block_id);
}

void
FEProblem::prepareMaterials(SubdomainID blk_id, THREAD_ID tid)
{
  if (_materials[tid].hasMaterials(blk_id))
  {
    std::set<MooseVariable *> needed_moose_vars;

    const std::vector<Material *> & materials = _materials[tid].getMaterials(blk_id);

    for (std::vector<Material *>::const_iterator it = materials.begin();
        it != materials.end();
        ++it)
    {
      const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }

    const std::set<unsigned int> & subdomain_boundary_ids = _mesh.getSubdomainBoundaryIds(blk_id);
    for (std::set<unsigned int>::const_iterator id_it = subdomain_boundary_ids.begin();
        id_it != subdomain_boundary_ids.end();
        ++id_it)
    {
      if (_materials[tid].hasBoundaryMaterials(*id_it))
      {
        const std::vector<Material *> & materials = _materials[tid].getBoundaryMaterials(*id_it);

        for (std::vector<Material *>::const_iterator it = materials.begin();
            it != materials.end();
            ++it)
        {
          const std::set<MooseVariable *> & mv_deps = (*it)->getMooseVariableDependencies();
          needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
        }
      }
    }

    const std::set<MooseVariable *> & current_active_elemental_moose_variables = getActiveElementalMooseVariables(tid);

    needed_moose_vars.insert(current_active_elemental_moose_variables.begin(), current_active_elemental_moose_variables.end());

    setActiveElementalMooseVariables(needed_moose_vars, tid);
  }
}

void
FEProblem::reinitMaterials(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (_materials[tid].hasMaterials(blk_id))
  {
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int n_points = _assembly[tid]->qRule()->n_points();
    if (_material_data[tid]->nQPoints() != n_points)
      _material_data[tid]->size(n_points);

    // Only swap if requested
    if (swap_stateful)
      _material_data[tid]->swap(*elem);

    _material_data[tid]->reinit(_materials[tid].getMaterials(blk_id));
  }
}

void
FEProblem::reinitMaterialsFace(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (_materials[tid].hasFaceMaterials(blk_id))
  {
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();

    if (_bnd_material_data[tid]->nQPoints() != n_points)
      _bnd_material_data[tid]->size(n_points);

    if (swap_stateful && !_bnd_material_data[tid]->isSwapped())
      _bnd_material_data[tid]->swap(*elem, side);

    _bnd_material_data[tid]->reinit(_materials[tid].getFaceMaterials(blk_id));
  }
}

void
FEProblem::reinitMaterialsNeighbor(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (_materials[tid].hasNeighborMaterials(blk_id)/* && _nl.doingDG()*/)
  {
    // NOTE: this will not work with h-adaptivity
    const Elem * & neighbor = _assembly[tid]->neighbor();
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();
    if (_neighbor_material_data[tid]->nQPoints() != n_points)
      _neighbor_material_data[tid]->size(n_points);

    // Only swap if requested
    if (swap_stateful)
      _neighbor_material_data[tid]->swap(*neighbor, neighbor_side);

    _neighbor_material_data[tid]->reinit(_materials[tid].getNeighborMaterials(blk_id));
  }
}

void
FEProblem::reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid, bool swap_stateful)
{
  if (_materials[tid].hasBoundaryMaterials(boundary_id)/* && _nl.hasActiveIntegratedBCs(boundary_id, tid)*/)
  {
    const Elem * & elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();
    if (_bnd_material_data[tid]->nQPoints() != n_points)
      _bnd_material_data[tid]->size(n_points);

    if (swap_stateful && !_bnd_material_data[tid]->isSwapped())
      _bnd_material_data[tid]->swap(*elem, side);

    _bnd_material_data[tid]->reinit(_materials[tid].getBoundaryMaterials(boundary_id));
  }
}

void
FEProblem::swapBackMaterials(THREAD_ID tid)
{
  const Elem * & elem = _assembly[tid]->elem();
  _material_data[tid]->swapBack(*elem);
}

void
FEProblem::swapBackMaterialsFace(THREAD_ID tid)
{
  const Elem * & elem = _assembly[tid]->elem();
  unsigned int side = _assembly[tid]->side();
  _bnd_material_data[tid]->swapBack(*elem, side);
}

void
FEProblem::swapBackMaterialsNeighbor(THREAD_ID tid)
{
  // NOTE: this will not work with h-adaptivity
  const Elem * & neighbor = _assembly[tid]->neighbor();
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
  _neighbor_material_data[tid]->swapBack(*neighbor, neighbor_side);
}

/**
 * Small helper function used by addPostprocessor to try to get a Postprocessor pointer from a MooseObject
 */
MooseSharedPointer<Postprocessor>
getPostprocessorPointer(MooseSharedPointer<MooseObject> mo)
{
  {
    MooseSharedPointer<ElementPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<ElementPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<NodalPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<NodalPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<InternalSidePostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<InternalSidePostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<SidePostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<SidePostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    MooseSharedPointer<GeneralPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<GeneralPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<Postprocessor>(intermediate);
  }

  return MooseSharedPointer<Postprocessor>();
}

template <typename UO_TYPE, typename PP_TYPE>
Postprocessor *
getPostprocessorPointer(UO_TYPE * uo)
{
  PP_TYPE * intermediate = dynamic_cast<PP_TYPE *>(uo);
  if (intermediate)
    return static_cast<Postprocessor *>(intermediate);

  return NULL;
}

void
FEProblem::addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
  else
    parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // Set a pointer to the correct material data; assume that it is a non-boundary material unless proven
    // to be otherwise
    MaterialData * mat_data = _material_data[tid];
    if (parameters.have_parameter<std::vector<BoundaryName> >("boundary") && !parameters.have_parameter<bool>("block_restricted_nodal"))
       mat_data = _bnd_material_data[tid];

    if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
      _reinit_displaced_face = true;

    parameters.set<MaterialData *>("_material_data") = mat_data;

    MooseSharedPointer<MooseObject> mo = _factory.create(pp_name, name, parameters);
    if (!mo)
      mooseError("Unable to determine type for Postprocessor: " + mo->name());

    MooseSharedPointer<Postprocessor> pp = getPostprocessorPointer(mo);

    // Postprocessor does not inherit from SetupInterface so we need to retrieve the exec_flags from the parameters directory
    const std::vector<ExecFlagType> exec_flags = Moose::vectorStringsToEnum<ExecFlagType>(parameters.get<MultiMooseEnum>("execute_on"));
    for (unsigned int i=0; i<exec_flags.size(); ++i)
    {
      // Check for name collision
      if (_user_objects(exec_flags[i])[tid].getUserObjectByName(name))
        mooseError(std::string("A UserObject with the name \"") + name + "\" already exists.  You may not add a Postprocessor by the same name.");
      _pps(exec_flags[i])[tid].addPostprocessor(pp);

      // Add it to the user object warehouse as well...
      MooseSharedPointer<UserObject> user_object = MooseSharedNamespace::dynamic_pointer_cast<UserObject>(mo);
      if (!user_object.get())
        mooseError("Unknown user object type: " + pp_name);

      _user_objects(exec_flags[i])[tid].addUserObject(user_object);
    }

    _objects_by_name[tid][name].push_back(mo.get());
    _pps_data[tid]->init(name);
  }
}

void
FEProblem::initPostprocessorData(const std::string & name)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _pps_data[tid]->init(name);

}

ExecStore<PostprocessorWarehouse> &
FEProblem::getPostprocessorWarehouse()
{
  return _pps;
}

ExecStore<UserObjectWarehouse> &
FEProblem::getUserObjectWarehouse()
{
  return _user_objects;
}

/**
 * Small helper function used by addVectorPostprocessor to try to get a VectorPostprocessor pointer from a MooseObject
 */
MooseSharedPointer<VectorPostprocessor>
getVectorPostprocessorPointer(MooseSharedPointer<MooseObject> mo)
{
  {
    MooseSharedPointer<ElementVectorPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<ElementVectorPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<VectorPostprocessor>(intermediate);
  }

  {
    MooseSharedPointer<NodalVectorPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<NodalVectorPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<VectorPostprocessor>(intermediate);
  }

  {
    MooseSharedPointer<InternalSideVectorPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<InternalSideVectorPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<VectorPostprocessor>(intermediate);
  }

  {
    MooseSharedPointer<SideVectorPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<SideVectorPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<VectorPostprocessor>(intermediate);
  }

  {
    MooseSharedPointer<GeneralVectorPostprocessor> intermediate = MooseSharedNamespace::dynamic_pointer_cast<GeneralVectorPostprocessor>(mo);
    if (intermediate.get())
      return MooseSharedNamespace::static_pointer_cast<VectorPostprocessor>(intermediate);
  }

  return MooseSharedPointer<VectorPostprocessor>();
}

void
FEProblem::addVectorPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
  else
    parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<VectorPostprocessorData *>("_vector_postprocessor_data") = _vpps_data[tid];

    // Set a pointer to the correct material data; assume that it is a non-boundary material unless proven
    // to be otherwise
    MaterialData * mat_data = _material_data[tid];
    if (parameters.have_parameter<std::vector<BoundaryName> >("boundary") && !parameters.have_parameter<bool>("block_restricted_nodal"))
       mat_data = _bnd_material_data[tid];

    if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
      _reinit_displaced_face = true;

    parameters.set<MaterialData *>("_material_data") = mat_data;

    MooseSharedPointer<MooseObject> mo = _factory.create(pp_name, name, parameters);
    if (!mo)
      mooseError("Unable to determine type for VectorPostprocessor: " + mo->name());

    MooseSharedPointer<VectorPostprocessor> pp = getVectorPostprocessorPointer(mo);

    // VectorPostprocessor does not inherit from SetupInterface so we need to retrieve the exec_flags from the parameters directory
    const std::vector<ExecFlagType> exec_flags = Moose::vectorStringsToEnum<ExecFlagType>(parameters.get<MultiMooseEnum>("execute_on"));
    for (unsigned int i=0; i<exec_flags.size(); ++i)
    {
      // Check for name collision
      if (_user_objects(exec_flags[i])[tid].getUserObjectByName(name))
        mooseError(std::string("A UserObject with the name \"") + name + "\" already exists.  You may not add a VectorPostprocessor by the same name.");
      _vpps(exec_flags[i])[tid].addVectorPostprocessor(pp);

      // Add it to the user object warehouse as well...
      MooseSharedPointer<UserObject> user_object = MooseSharedNamespace::dynamic_pointer_cast<UserObject>(mo);
      if (!user_object.get())
        mooseError("Unknown user object type: " + pp_name);

      _user_objects(exec_flags[i])[tid].addUserObject(user_object);
    }

    _objects_by_name[tid][name].push_back(mo.get());
  }
}

ExecStore<VectorPostprocessorWarehouse> &
FEProblem::getVectorPostprocessorWarehouse()
{
  return _vpps;
}

void
FEProblem::addUserObject(std::string user_object_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
  else
    parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    // Set a pointer to the correct material data; assume that it is a non-boundary material unless proven
    // to be otherwise
    MaterialData * mat_data = _material_data[tid];
    if ((parameters.isParamValid("use_bnd_material") && parameters.get<bool>("use_bnd_material")) ||
        (parameters.have_parameter<std::vector<BoundaryName> >("boundary") && !parameters.have_parameter<bool>("block_restricted_nodal")))
       mat_data = _bnd_material_data[tid];

    if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
      _reinit_displaced_face = true;

    parameters.set<MaterialData *>("_material_data") = mat_data;

    MooseSharedPointer<UserObject> user_object = MooseSharedNamespace::static_pointer_cast<UserObject>(_factory.create(user_object_name, name, parameters));

    const std::vector<ExecFlagType> & exec_flags = user_object->execFlags();
    for (unsigned int i=0; i<exec_flags.size(); ++i)
      _user_objects(exec_flags[i])[tid].addUserObject(user_object);

    _objects_by_name[tid][name].push_back(user_object.get());
  }
}

const UserObject &
FEProblem::getUserObjectBase(const std::string & name)
{
  for (unsigned int i = 0; i < Moose::exec_types.size(); ++i)
    if (_user_objects(Moose::exec_types[i])[0].hasUserObject(name))
      return *_user_objects(Moose::exec_types[i])[0].getUserObjectByName(name);

  mooseError("Unable to find user object with name '" + name + "'");
}

bool
FEProblem::hasUserObject(const std::string & name)
{
  for (unsigned int i = 0; i < Moose::exec_types.size(); i++)
    if (_user_objects(Moose::exec_types[i])[0].hasUserObject(name))
      return true;
  return false;
}

bool
FEProblem::hasPostprocessor(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid]->hasPostprocessor(name);
}

PostprocessorValue &
FEProblem::getPostprocessorValue(const PostprocessorName & name, THREAD_ID tid)
{
  return _pps_data[tid]->getPostprocessorValue(name);
}

PostprocessorValue &
FEProblem::getPostprocessorValueOld(const std::string & name, THREAD_ID tid)
{
  return _pps_data[tid]->getPostprocessorValueOld(name);
}

bool
FEProblem::hasVectorPostprocessor(const std::string & name)
{
  return _vpps_data[0]->hasVectorPostprocessor(name);
}

VectorPostprocessorValue &
FEProblem::getVectorPostprocessorValue(const VectorPostprocessorName & name, const std::string & vector_name)
{
  return _vpps_data[0]->getVectorPostprocessorValue(name, vector_name);
}

VectorPostprocessorValue &
FEProblem::getVectorPostprocessorValueOld(const std::string & name, const std::string & vector_name)
{
  return _vpps_data[0]->getVectorPostprocessorValueOld(name, vector_name);
}

const std::map<std::string, VectorPostprocessorValue*> &
FEProblem::getVectorPostprocessorVectors(const std::string & vpp_name)
{
  return _vpps_data[0]->vectors(vpp_name);
}

void
FEProblem::parentOutputPositionChanged()
{
  for (unsigned int i = 0; i < Moose::exec_types.size(); i++)
    _multi_apps(Moose::exec_types[i])[0].parentOutputPositionChanged();
}

void
FEProblem::computeIndicatorsAndMarkers()
{
  // Zero them out first
  if (_indicators[0].all().size() || _markers[0].all().size())
  {
    std::vector<std::string> fields;

    // Add Indicator Fields
    {
      const std::vector<Indicator *> & all_indicators = _indicators[0].all();

      for (std::vector<Indicator *>::const_iterator i=all_indicators.begin();
          i != all_indicators.end();
          ++i)
        fields.push_back((*i)->name());
    }

    // Add Marker Fields
    {
      const std::vector<Marker *> & all_markers = _markers[0].all();

      for (std::vector<Marker *>::const_iterator i=all_markers.begin();
          i != all_markers.end();
          ++i)
        fields.push_back((*i)->name());
    }

    _aux.zeroVariables(fields);
  }

  // compute Indicators
  if (_indicators[0].all().size())
  {
    ComputeIndicatorThread cit(*this, getAuxiliarySystem(), _indicators);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux.solution().close();
    _aux.update();
  }

  // finalize Indicators
  if (_indicators[0].all().size())
  {
    ComputeIndicatorThread cit(*this, getAuxiliarySystem(), _indicators, true);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux.solution().close();
    _aux.update();
  }

  // compute Markers
  if (_markers[0].all().size())
  {
    _adaptivity.updateErrorVectors();

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
      _markers[tid].markerSetup();

    ComputeMarkerThread cmt(*this, getAuxiliarySystem(), _markers);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cmt);

    _aux.solution().close();
    _aux.update();
  }
}

void
FEProblem::computeUserObjectsInternal(ExecFlagType type, UserObjectWarehouse::GROUP group)
{
  std::vector<UserObjectWarehouse> & pps = _user_objects(type);
  if (pps[0].blockIds().size() || pps[0].boundaryIds().size() || pps[0].nodesetIds().size() || pps[0].blockNodalIds().size() || pps[0].internalSideUserObjects(group).size())
  {
    if (!pps[0].nodesetIds().size())
    {
      serializeSolution();

      if (_displaced_problem != NULL)
        _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

      /**
       * Legacy behavior requires that we compute the RESIDUAL set of AuxKernels when
       * we compute user objects.
       */
      if (_use_legacy_uo_aux_computation)
        _aux.compute(EXEC_LINEAR);
    }

    // init
    bool have_elemental_uo = false;
    bool have_side_uo = false;
    bool have_internal_uo = false;
    bool have_nodal_uo = false;
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      for (std::set<SubdomainID>::const_iterator block_it = pps[tid].blockIds().begin();
          block_it != pps[tid].blockIds().end();
          ++block_it)
      {
        SubdomainID block_id = *block_it;

        for (std::vector<ElementUserObject *>::const_iterator user_object_it = pps[tid].elementUserObjects(block_id, group).begin();
             user_object_it != pps[tid].elementUserObjects(block_id, group).end();
            ++user_object_it)
        {
          (*user_object_it)->initialize();
          have_elemental_uo = true;
        }
      }

      for (std::set<BoundaryID>::const_iterator boundary_it = pps[tid].boundaryIds().begin();
          boundary_it != pps[tid].boundaryIds().end();
          ++boundary_it)
      {
        //note: for threaded applications where the elements get broken up it
        //may be more efficient to initialize these on demand inside the loop
        for (std::vector<SideUserObject *>::const_iterator side_user_object_it = pps[tid].sideUserObjects(*boundary_it, group).begin();
             side_user_object_it != pps[tid].sideUserObjects(*boundary_it, group).end();
             ++side_user_object_it)
        {
          (*side_user_object_it)->initialize();
          have_side_uo = true;
        }
      }

      for (std::set<SubdomainID>::const_iterator block_ids_it = pps[tid].blockIds().begin();
           block_ids_it != pps[tid].blockIds().end();
           ++block_ids_it)
      {
        SubdomainID block_id = *block_ids_it;

        const std::vector<InternalSideUserObject *> & isuos = pps[tid].internalSideUserObjects(block_id, group);
        for (std::vector<InternalSideUserObject *>::const_iterator it = isuos.begin(); it != isuos.end(); ++it)
        {
          (*it)->initialize();
          have_internal_uo = true;
        }
      }

      for (std::set<BoundaryID>::const_iterator boundary_it = pps[tid].nodesetIds().begin();
          boundary_it != pps[tid].nodesetIds().end();
          ++boundary_it)
      {
        for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it = pps[tid].nodalUserObjects(*boundary_it, group).begin();
             nodal_user_object_it != pps[tid].nodalUserObjects(*boundary_it, group).end();
             ++nodal_user_object_it)
        {
          (*nodal_user_object_it)->initialize();
          have_nodal_uo = true;
        }
      }

      // Block restricted nodal user objects
      for (std::set<SubdomainID>::const_iterator block_it = pps[tid].blockNodalIds().begin();
          block_it != pps[tid].blockNodalIds().end();
          ++block_it)
      {
        for (std::vector<NodalUserObject *>::const_iterator nodal_user_object_it = pps[tid].blockNodalUserObjects(*block_it, group).begin();
             nodal_user_object_it != pps[tid].blockNodalUserObjects(*block_it, group).end();
             ++nodal_user_object_it)
        {
          (*nodal_user_object_it)->initialize();
          have_nodal_uo = true;
        }
      }
    }

    // Store element user_objects values
    std::set<UserObject *> already_gathered;

    // compute
    if (have_elemental_uo || have_side_uo || have_internal_uo)
    {
      ComputeUserObjectsThread cppt(*this, getNonlinearSystem(), *getNonlinearSystem().currentSolution(), pps, group);
      Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);

      for (std::set<SubdomainID>::const_iterator block_ids_it = pps[0].blockIds().begin();
           block_ids_it != pps[0].blockIds().end();
           ++block_ids_it)
      {
        SubdomainID block_id = *block_ids_it;

        const std::vector<ElementUserObject *> & element_user_objects = pps[0].elementUserObjects(block_id, group);
        // Store element user_objects values
        for (unsigned int i = 0; i < element_user_objects.size(); ++i)
        {
          ElementUserObject *ps = element_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].elementUserObjects(block_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer<ElementUserObject, ElementPostprocessor>(ps);

            if (pp)
            {
              Real value = pp->getValue();
              // store the value in each thread

              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid]->storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }

      // Store side user_objects values
      already_gathered.clear();
      for (std::set<BoundaryID>::const_iterator boundary_ids_it = pps[0].boundaryIds().begin();
           boundary_ids_it != pps[0].boundaryIds().end();
           ++boundary_ids_it)
      {
        BoundaryID boundary_id = *boundary_ids_it;

        const std::vector<SideUserObject *> & side_user_objects = pps[0].sideUserObjects(boundary_id, group);
        for (unsigned int i = 0; i < side_user_objects.size(); ++i)
        {
          SideUserObject *ps = side_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].sideUserObjects(boundary_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer<SideUserObject, SidePostprocessor>(ps);

            if (pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid]->storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }

      // Internal side user objects
      already_gathered.clear();
      for (std::set<SubdomainID>::const_iterator block_ids_it = pps[0].blockIds().begin();
           block_ids_it != pps[0].blockIds().end();
           ++block_ids_it)
      {
        SubdomainID block_id = *block_ids_it;

        const std::vector<InternalSideUserObject *> & internal_side_user_objects = pps[0].internalSideUserObjects(block_id, group);
        for (unsigned int i = 0; i < internal_side_user_objects.size(); ++i)
        {
          // Pointer to current warehouse
          InternalSideUserObject * it = internal_side_user_objects[i];

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(it) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              it->threadJoin(*pps[tid].internalSideUserObjects(block_id, group)[i]);

            it->finalize();

            Postprocessor * pp = getPostprocessorPointer<InternalSideUserObject, InternalSidePostprocessor>(it);

            if (pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid]->storeValue(pp->PPName(), value);
            }

            already_gathered.insert(it);
          }
        }
      }

      /*
       const std::vector<InternalSideUserObject *> & isuos = pps[0].internalSideUserObjects(group);
      for (unsigned int i = 0; i < isuos.size(); ++i)
      {
        InternalSideUserObject *ps = isuos[i];
        // join across the threads (gather the value in thread #0)
        if (already_gathered.find(ps) == already_gathered.end())
        {
          for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
            ps->threadJoin(*pps[tid].internalSideUserObjects(group)[i]);
          ps->finalize();
        }
        already_gathered.insert(ps);
      }
      */
    }

    // Don't waste time looping over nodes if there aren't any nodal user_objects to calculate
    if (have_nodal_uo)
    {
      ComputeNodalUserObjectsThread cnppt(*this, pps, group);
      Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cnppt);

      // Store nodal user_objects values
      already_gathered.clear();
      for (std::set<BoundaryID>::const_iterator boundary_ids_it = pps[0].nodesetIds().begin();
           boundary_ids_it != pps[0].nodesetIds().end();
           ++boundary_ids_it)
      {
        BoundaryID boundary_id = *boundary_ids_it;

        const std::vector<NodalUserObject *> & nodal_user_objects = pps[0].nodalUserObjects(boundary_id, group);
        for (unsigned int i = 0; i < nodal_user_objects.size(); ++i)
        {
          NodalUserObject *ps = nodal_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].nodalUserObjects(boundary_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer<NodalUserObject, NodalPostprocessor>(ps);

            if (pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid]->storeValue(name, value);
            }

            already_gathered.insert(ps);
          }
        }
      }

      // Block restricted nodal user_objects
      for (std::set<SubdomainID>::const_iterator block_ids_it = pps[0].blockNodalIds().begin();
           block_ids_it != pps[0].blockNodalIds().end();
           ++block_ids_it)
      {
        SubdomainID block_id = *block_ids_it;

        const std::vector<NodalUserObject *> & nodal_user_objects = pps[0].blockNodalUserObjects(block_id, group);
        for (unsigned int i = 0; i < nodal_user_objects.size(); ++i)
        {
          NodalUserObject *ps = nodal_user_objects[i];
          std::string name = ps->name();

          // join across the threads (gather the value in thread #0)
          if (already_gathered.find(ps) == already_gathered.end())
          {
            for (THREAD_ID tid = 1; tid < libMesh::n_threads(); ++tid)
              ps->threadJoin(*pps[tid].blockNodalUserObjects(block_id, group)[i]);

            ps->finalize();

            Postprocessor * pp = getPostprocessorPointer<NodalUserObject, NodalPostprocessor>(ps);

            if (pp)
            {
              Real value = pp->getValue();

              // store the value in each thread
              for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
                _pps_data[tid]->storeValue(name, value);

              already_gathered.insert(ps);
            }
          }
        }
      }
    }
  }

  // Compute and store generic user_objects values
  for (std::vector<GeneralUserObject *>::const_iterator generic_user_object_it = pps[0].genericUserObjects(group).begin();
      generic_user_object_it != pps[0].genericUserObjects(group).end();
      ++generic_user_object_it)
  {
    std::string name = (*generic_user_object_it)->name();
    (*generic_user_object_it)->initialize();
    (*generic_user_object_it)->execute();

    (*generic_user_object_it)->finalize();

    Postprocessor * pp = getPostprocessorPointer<GeneralUserObject, GeneralPostprocessor>(*generic_user_object_it);

    if (pp)
    {
      Real value = pp->getValue();

      // store the value in each thread
      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
        _pps_data[tid]->storeValue(name, value);
    }
  }
}

void
FEProblem::computeUserObjects(ExecFlagType type/* = EXEC_TIMESTEP_END*/, UserObjectWarehouse::GROUP group)
{
  Moose::perf_log.push("compute_user_objects()","Solve");

  switch (type)
  {
  case EXEC_LINEAR:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _user_objects(type)[tid].residualSetup();
    break;

  case EXEC_NONLINEAR:
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      _user_objects(type)[tid].jacobianSetup();
    break;

  default:
    break;
  }
  computeUserObjectsInternal(type, group);

  Moose::perf_log.pop("compute_user_objects()","Solve");
}

void
FEProblem::reinitBecauseOfGhosting()
{
  // Need to see if _any_ processor has ghosted elems
  dof_id_type ghosted = _ghosted_elems.size();
  _communicator.sum(ghosted);

  if (ghosted)
  {
    // Call reinit to get the ghosted vectors correct now that some geometric search has been done
    _eq.reinit();

    if (_displaced_mesh)
      _displaced_problem->es().reinit();
  }
}

void
FEProblem::addDamper(std::string damper_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = &_nl;

  _has_dampers = true;
  _nl.addDamper(damper_name, name, parameters);
}

void
FEProblem::setupDampers()
{
  _nl.setupDampers();
}

void
FEProblem::addIndicator(std::string indicator_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    parameters.set<MaterialData *>("_material_data") = _material_data[tid];

    parameters.set<MaterialData *>("_material_data") = _bnd_material_data[tid];
    parameters.set<MaterialData *>("_neighbor_material_data") = _neighbor_material_data[tid];

    MooseSharedPointer<Indicator> indicator = MooseSharedNamespace::static_pointer_cast<Indicator>(_factory.create(indicator_name, name, parameters));

    std::vector<SubdomainID> block_ids;
    _indicators[tid].addIndicator(indicator, block_ids);

    _objects_by_name[tid][name].push_back(indicator.get());
  }
}

void
FEProblem::addMarker(std::string marker_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    MooseSharedPointer<Marker> marker = MooseSharedNamespace::static_pointer_cast<Marker>(_factory.create(marker_name, name, parameters));

    std::vector<SubdomainID> block_ids;
    _markers[tid].addMarker(marker, block_ids);

    _objects_by_name[tid][name].push_back(marker.get());
  }
}

void
FEProblem::addMultiApp(const std::string & multi_app_name, const std::string & name, InputParameters parameters)
{
  _has_multiapps = true;

  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<THREAD_ID>("_tid") = 0;
  parameters.set<MPI_Comm>("_mpi_comm") = _communicator.get();
  parameters.set<MooseSharedPointer<CommandLine> >("_command_line") = _app.commandLine();

  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  MooseSharedPointer<MooseObject> mo = _factory.create(multi_app_name, name, parameters);

  MooseSharedPointer<MultiApp> multi_app = MooseSharedNamespace::dynamic_pointer_cast<MultiApp>(mo);
  if (multi_app.get() == NULL)
    mooseError("Unknown MultiApp type: " << multi_app_name);

  const std::vector<ExecFlagType> & exec_flags = multi_app->execFlags();
  for (unsigned int i=0; i<exec_flags.size(); ++i)
    _multi_apps(exec_flags[i])[0].addMultiApp(multi_app);

  // TODO: Is this really the right spot to init a multiapp?
//  multi_app->init();
}

MultiApp *
FEProblem::getMultiApp(const std::string & multi_app_name)
{
  for (unsigned int i = 0; i < Moose::exec_types.size(); i++)
    if (_multi_apps(Moose::exec_types[i])[0].hasMultiApp(multi_app_name))
      return _multi_apps(Moose::exec_types[i])[0].getMultiApp(multi_app_name);

  mooseError("MultiApp "<<multi_app_name<<" not found!");
}

void
FEProblem::execMultiApps(ExecFlagType type, bool auto_advance)
{
 std::vector<MultiApp *> multi_apps = _multi_apps(type)[0].all();

  // Do anything that needs to be done to Apps before transfers
  for (unsigned int i=0; i<multi_apps.size(); i++)
    multi_apps[i]->preTransfer(_dt, _time);

  // Execute Transfers _to_ MultiApps
  {
    std::vector<Transfer *> transfers = _to_multi_app_transfers(type)[0].all();
    if (transfers.size())
      for (unsigned int i=0; i<transfers.size(); i++)
        transfers[i]->execute();
  }

  if (multi_apps.size())
  {
    _console << "Executing MultiApps" << std::endl;

    for (unsigned int i=0; i<multi_apps.size(); i++)
      multi_apps[i]->solveStep(_dt, _time, auto_advance);

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << "Finished Executing MultiApps" << std::endl;
  }

  // Execute Transfers _from_ MultiApps
  {
    std::vector<Transfer *> transfers = _from_multi_app_transfers(type)[0].all();
    if (transfers.size())
    {
      _console << "Starting Transfers From MultiApps" << std::endl;
      for (unsigned int i=0; i<transfers.size(); i++)
        transfers[i]->execute();

      _console << "Waiting For Transfers To Finish" << std::endl;
      MooseUtils::parallelBarrierNotify(_communicator);

      _console << "Transfers To Finished" << std::endl;
    }
  }
}

void
FEProblem::advanceMultiApps(ExecFlagType type)
{
  std::vector<MultiApp *> multi_apps = _multi_apps(type)[0].all();

  if (multi_apps.size())
  {
    _console << "Advancing MultiApps" << std::endl;

    for (unsigned int i=0; i<multi_apps.size(); i++)
      multi_apps[i]->advanceStep();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << "Finished Advancing MultiApps" << std::endl;
  }
}

Real
FEProblem::computeMultiAppsDT(ExecFlagType type)
{
  std::vector<TransientMultiApp *> multi_apps = _multi_apps(type)[0].transient();

  Real smallest_dt = std::numeric_limits<Real>::max();

  for (unsigned int i=0; i<multi_apps.size(); i++)
    smallest_dt = std::min(smallest_dt, multi_apps[i]->computeDT());

  return smallest_dt;
}


void
FEProblem::execTransfers(ExecFlagType type)
{
  std::vector<Transfer *> transfers = _transfers(type)[0].all();

  if (transfers.size())
    for (unsigned int i=0; i<transfers.size(); i++)
      transfers[i]->execute();
}

void
FEProblem::addTransfer(const std::string & transfer_name, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem;
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = &_aux;
  }

  parameters.set<THREAD_ID>("_tid") = 0;

  MooseSharedPointer<MooseObject> mo = _factory.create(transfer_name, name, parameters);

  MooseSharedPointer<Transfer> transfer = MooseSharedNamespace::dynamic_pointer_cast<Transfer>(mo);
  if (transfer.get() == NULL)
    mooseError("Unknown Transfer type: " << transfer_name);

  MooseSharedPointer<MultiAppTransfer> multi_app_transfer = MooseSharedNamespace::dynamic_pointer_cast<MultiAppTransfer>(transfer);

  if (multi_app_transfer.get())
  {
    std::vector<ExecFlagType> transfer_exec_flags = multi_app_transfer->execFlags();
    const std::vector<ExecFlagType> & multiapp_exec_flags = multi_app_transfer->getMultiApp()->execFlags();

    if (transfer_exec_flags.empty())
      transfer_exec_flags.assign(multiapp_exec_flags.begin(), multiapp_exec_flags.end());
    else if (transfer_exec_flags != multiapp_exec_flags)
      mooseDoOnce(mooseWarning("MultiAppTransfer execute_on flags do not match associated Multiapp execute_on flags"));

    for (unsigned int i=0; i<transfer_exec_flags.size(); ++i)
      if (multi_app_transfer->direction() == MultiAppTransfer::TO_MULTIAPP)
        _to_multi_app_transfers(transfer_exec_flags[i])[0].addTransfer(multi_app_transfer);
      else
        _from_multi_app_transfers(transfer_exec_flags[i])[0].addTransfer(multi_app_transfer);
  }
  else
  {
    const std::vector<ExecFlagType> & exec_flags = transfer->execFlags();
    for (unsigned int i=0; i<exec_flags.size(); ++i)
      _transfers(exec_flags[i])[0].addTransfer(transfer);
  }
}

bool
FEProblem::hasVariable(const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return true;
  else if (_aux.hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
FEProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasVariable(var_name))
    return _nl.getVariable(tid, var_name);
  else if (!_aux.hasVariable(var_name))
    mooseError("Unknown variable " + var_name);

  return _aux.getVariable(tid, var_name);
}

bool
FEProblem::hasScalarVariable(const std::string & var_name)
{
  if (_nl.hasScalarVariable(var_name))
    return true;
  else if (_aux.hasScalarVariable(var_name))
    return true;
  else
    return false;
}

MooseVariableScalar &
FEProblem::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl.hasScalarVariable(var_name))
    return _nl.getScalarVariable(tid, var_name);
  else if (_aux.hasScalarVariable(var_name))
    return _aux.getScalarVariable(tid, var_name);
  else
    mooseError("Unknown variable " + var_name);
}

void
FEProblem::setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars, THREAD_ID tid)
{
  SubProblem::setActiveElementalMooseVariables(moose_vars, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveElementalMooseVariables(moose_vars, tid);
}

const std::set<MooseVariable *> &
FEProblem::getActiveElementalMooseVariables(THREAD_ID tid)
{
  return SubProblem::getActiveElementalMooseVariables(tid);
}

bool
FEProblem::hasActiveElementalMooseVariables(THREAD_ID tid)
{
  return SubProblem::hasActiveElementalMooseVariables(tid);
}

void
FEProblem::clearActiveElementalMooseVariables(THREAD_ID tid)
{
  SubProblem::clearActiveElementalMooseVariables(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveElementalMooseVariables(tid);
}

void
FEProblem::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  if (order == INVALID_ORDER)
  {
    // automatically determine the integration order
    Moose::setup_perf_log.push("getMinQuadratureOrder()","Setup");
    order = _nl.getMinQuadratureOrder();
    if (order<_aux.getMinQuadratureOrder()) order = _aux.getMinQuadratureOrder();
    Moose::setup_perf_log.pop("getMinQuadratureOrder()","Setup");
  }

  if (volume_order == INVALID_ORDER)
    volume_order = order;

  if (face_order == INVALID_ORDER)
    face_order = order;

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->createQRules(type, order, volume_order, face_order);

  if (_displaced_problem)
    _displaced_problem->createQRules(type, order, volume_order, face_order);

  // Find the maximum number of quadrature points
  {
    Moose::setup_perf_log.push("maxQps()","Setup");
    MaxQpsThread mqt(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), mqt);
    _max_qps = mqt.max();

    // Set all of the current volume quadrature rules back to NULL as if we never did this...
    for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      for (unsigned int dim=0; dim <= LIBMESH_DIM; dim++)
        _assembly[tid]->setVolumeQRule(NULL,dim);
    }

    Moose::setup_perf_log.pop("maxQps()","Setup");
  }
}

void
FEProblem::setCoupling(Moose::CouplingType type)
{
  _coupling = type;
}

void
FEProblem::setCouplingMatrix(CouplingMatrix * cm)
{
  _coupling = Moose::COUPLING_CUSTOM;
  delete _cm;
  _cm = cm;
}

void
FEProblem::useFECache(bool fe_cache)
{
  if (fe_cache)
    _console << "\nUtilizing FE Shape Function Caching\n" << std::endl;

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->useFECache(fe_cache); //fe_cache);
}

void
FEProblem::init()
{
  if (_initialized)
    return;

  unsigned int n_vars = _nl.nVariables();
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

  _nl.dofMap()._dof_coupling = _cm;
  _nl.dofMap().attach_extra_sparsity_function(&extraSparsity, &_nl);
  _nl.dofMap().attach_extra_send_list_function(&extraSendList, &_nl);
  _aux.dofMap().attach_extra_send_list_function(&extraSendList, &_aux);


  if (_solve && n_vars == 0)
    mooseError("No variables specified in the FEProblem '" << name() << "'.");

  Moose::setup_perf_log.push("eq.init()","ghostGhostedBoundaries");
  ghostGhostedBoundaries(); // We do this again right here in case new boundaries have been added
  Moose::setup_perf_log.pop("eq.init()","ghostGhostedBoundaries");

  Moose::setup_perf_log.push("eq.init()","Setup");
  _eq.init();
  Moose::setup_perf_log.pop("eq.init()","Setup");

  Moose::setup_perf_log.push("FEProblem::init::meshChanged()","Setup");
  _mesh.meshChanged();
  if (_displaced_problem)
    _displaced_mesh->meshChanged();
  Moose::setup_perf_log.pop("FEProblem::init::meshChanged()","Setup");

  Moose::setup_perf_log.push("NonlinearSystem::update()","Setup");
  _nl.update();
  Moose::setup_perf_log.pop("NonlinearSystem::update()","Setup");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->init();

  _nl.init();

  if (_displaced_problem)
    _displaced_problem->init();

  _aux.init();

  _initialized = true;
}

void
FEProblem::solve()
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(*this); // Make sure the PETSc options are setup for this app
#endif

  Moose::setSolverDefaults(*this);

  // Setup the output system for printing linear/nonlinear iteration information
  initPetscOutput();

  Moose::perf_log.push("solve()","Solve");

  possiblyRebuildGeomSearchPatches();

//  _solve_only_perf_log.push("solve");

  if (_solve)
    _nl.solve();

//  _solve_only_perf_log.pop("solve");
  Moose::perf_log.pop("solve()","Solve");

  if (_solve)
    _nl.update();

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions(*_nl.currentSolution(), *_aux.currentSolution());
}

bool
FEProblem::converged()
{
  if (_solve)
    return _nl.converged();
  else
    return true;
}

void
FEProblem::copySolutionsBackwards()
{
  _nl.copySolutionsBackwards();
  _aux.copySolutionsBackwards();
}

void
FEProblem::advanceState()
{
  _nl.copyOldSolutions();
  _aux.copyOldSolutions();

  if ( _displaced_problem != NULL )
  {
    _displaced_problem->nlSys().copyOldSolutions();
    _displaced_problem->auxSys().copyOldSolutions();
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _pps_data[tid]->copyValuesBack();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _materials[tid].timestepSetup();

  if (_material_props.hasStatefulProperties())
    _material_props.shift();

  if (_bnd_material_props.hasStatefulProperties())
    _bnd_material_props.shift();
}

void
FEProblem::restoreSolutions()
{
  _nl.restoreSolutions();
  _aux.restoreSolutions();

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());
}

void
FEProblem::outputStep(ExecFlagType type)
{
  _nl.update();
  _aux.update();
  _app.getOutputWarehouse().outputStep(type);
}

void
FEProblem::allowOutput(bool state)
{
  _app.getOutputWarehouse().allowOutput(state);
}

void
FEProblem::forceOutput()
{
  _app.getOutputWarehouse().forceOutput();
}

void
FEProblem::initPetscOutput()
{
  _app.getOutputWarehouse().solveSetup();
  Moose::PetscSupport::petscSetDefaults(*this);
}


Real
FEProblem::solutionChangeNorm()
{
  if (!_solve)
    return 0;

  NumericVector<Number> & current_solution  = (*_nl.sys().current_local_solution);
  NumericVector<Number> & old_solution = (*_nl.sys().old_local_solution);

  NumericVector<Number> & difference = *NumericVector<Number>::build(_communicator).release();
  difference.init(current_solution, true);

  difference = current_solution;

  difference -= old_solution;

  Real abs_change = difference.l2_norm();

  delete &difference;

  return (abs_change / current_solution.l2_norm()) / _dt;
}

void
FEProblem::onTimestepBegin()
{
  _nl.onTimestepBegin();
}

void
FEProblem::onTimestepEnd()
{
}

void
FEProblem::computeAuxiliaryKernels(ExecFlagType type)
{
  _aux.compute(type);
}

void
FEProblem::addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  _aux.addTimeIntegrator(type, name + ":aux", parameters);
  _nl.addTimeIntegrator(type, name, parameters);
}

void
FEProblem::addPredictor(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<FEProblem *>("_fe_problem") = this;
  parameters.set<SubProblem *>("_subproblem") = this;
  MooseSharedPointer<Predictor> predictor = MooseSharedNamespace::static_pointer_cast<Predictor>(_factory.create(type, name, parameters));
  _nl.setPredictor(predictor);
}

Real
FEProblem::computeResidualL2Norm()
{
  computeResidualType(*_nl.currentSolution(), *_nl.sys().rhs);

  return _nl.sys().rhs->l2_norm();
}

void
FEProblem::computeResidual(NonlinearImplicitSystem &/*sys*/, const NumericVector<Number> & soln, NumericVector<Number> & residual)
{
  computeResidualType(soln, residual, _kernel_type);
}

void
FEProblem::computeTransientImplicitResidual(Real time, const NumericVector<Number> & u, const NumericVector<Number> & udot, NumericVector<Number> & residual)
{
  _nl.setSolutionUDot(udot);
  NonlinearImplicitSystem &sys = _nl.sys();
  _time = time;
  computeResidual(sys, u, residual);
}

void
FEProblem::computeResidualType(const NumericVector<Number>& soln, NumericVector<Number>& residual, Moose::KernelType type)
{
  _nl.setSolution(soln);

  _nl.zeroVariablesForResidual();
  _aux.zeroVariablesForResidual();

  unsigned int n_threads = libMesh::n_threads();

  // Random interface objects
  for (std::map<std::string, RandomData *>::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end();
       ++it)
    it->second->updateSeeds(EXEC_LINEAR);

  execTransfers(EXEC_LINEAR);

  execMultiApps(EXEC_LINEAR);

  computeUserObjects(EXEC_LINEAR, UserObjectWarehouse::PRE_AUX);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(soln, *_aux.currentSolution());



  for (unsigned int i=0; i<n_threads; i++)
  {
    _materials[i].residualSetup();

    for (std::map<std::string, MooseSharedPointer<Function> >::iterator vit = _functions[i].begin();
        vit != _functions[i].end();
        ++vit)
      vit->second->residualSetup();
  }
  _aux.residualSetup();

  _aux.compute(EXEC_LINEAR);

  computeUserObjects(EXEC_LINEAR, UserObjectWarehouse::POST_AUX);

  _app.getOutputWarehouse().residualSetup();

  _nl.computeResidual(residual, type);

  // Need to close and update the aux system in case residuals were saved to it.
  _aux.solution().close();
  _aux.update();
}

void
FEProblem::computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian)
{
  if (!_has_jacobian || !_const_jacobian)
  {
    _nl.setSolution(soln);

    _nl.zeroVariablesForJacobian();
    _aux.zeroVariablesForJacobian();

    unsigned int n_threads = libMesh::n_threads();

    // Random interface objects
    for (std::map<std::string, RandomData *>::iterator it = _random_data_objects.begin();
         it != _random_data_objects.end();
         ++it)
      it->second->updateSeeds(EXEC_NONLINEAR);

    execTransfers(EXEC_NONLINEAR);
    execMultiApps(EXEC_NONLINEAR);

    computeUserObjects(EXEC_NONLINEAR, UserObjectWarehouse::PRE_AUX);

    if (_displaced_problem != NULL)
      _displaced_problem->updateMesh(soln, *_aux.currentSolution());

    for (unsigned int i=0; i<n_threads; i++)
    {
      _materials[i].jacobianSetup();

      for (std::map<std::string, MooseSharedPointer<Function> >::iterator vit = _functions[i].begin();
          vit != _functions[i].end();
          ++vit)
        vit->second->jacobianSetup();
    }

    _aux.jacobianSetup();

    _aux.compute(EXEC_NONLINEAR);

    computeUserObjects(EXEC_NONLINEAR, UserObjectWarehouse::POST_AUX);

    _app.getOutputWarehouse().jacobianSetup();

    _nl.computeJacobian(jacobian);

    _has_jacobian = true;
  }

  if (_solver_params._type == Moose::ST_JFNK || _solver_params._type == Moose::ST_PJFNK)
  {
    // This call is here to make sure the residual vector is up to date with any decisions that have been made in
    // the Jacobian evaluation.  That is important in JFNK because that residual is used for finite differencing
    computeResidual(sys, soln, *sys.rhs);
    sys.rhs->close();
  }
}

void
FEProblem::computeTransientImplicitJacobian(Real time, const NumericVector<Number> & u, const NumericVector<Number> & udot, Real shift, SparseMatrix<Number> & jacobian)
{
  if (0)
  { // The current interface guarantees that the residual is called before Jacobian, thus udot has already been set
    _nl.setSolutionUDot(udot);
  }
  _nl.duDotDu() = shift;
  NonlinearImplicitSystem &sys = _nl.sys();
  _time = time;
  computeJacobian(sys,u,jacobian);
}


void
FEProblem::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh(*_nl.currentSolution(), *_aux.currentSolution());

  _aux.compute(EXEC_NONLINEAR);

  _nl.computeJacobianBlocks(blocks);
}

void
FEProblem::computeJacobianBlock(SparseMatrix<Number> & jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar)
{
  std::vector<JacobianBlock *> blocks;
  JacobianBlock * block = new JacobianBlock(precond_system, jacobian, ivar, jvar);
  blocks.push_back(block);
  computeJacobianBlocks(blocks);
  delete block;
}

void
FEProblem::computeBounds(NonlinearImplicitSystem & /*sys*/, NumericVector<Number>& lower, NumericVector<Number>& upper)
{
  if (!_nl.hasVector("lower_bound") || !_nl.hasVector("upper_bound")) return;
  try
  {
    NumericVector<Number> & _lower = _nl.getVector("lower_bound");
    NumericVector<Number> & _upper = _nl.getVector("upper_bound");
    _lower.swap(lower);
    _upper.swap(upper);
    unsigned int n_threads = libMesh::n_threads();
    for (unsigned int i=0; i<n_threads; i++)
    {
      _materials[i].residualSetup();
    }
    _aux.residualSetup();
    _aux.compute(EXEC_LINEAR);
    _lower.swap(lower);
    _upper.swap(upper);
  }
  catch (MooseException & e)
  {
    // tell solver to stop
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,0,0)
#else
    PetscNonlinearSolver<Real> & solver = static_cast<PetscNonlinearSolver<Real> &>(*_nl.sys().nonlinear_solver);
    SNESSetFunctionDomainError(solver.snes());
#endif
#endif
  }
}

void
FEProblem::computeNearNullSpace(NonlinearImplicitSystem & /*sys*/, std::vector<NumericVector<Number>*>& sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NearNullSpace"); ++i) {
    std::stringstream postfix;
    postfix << "_" << i;
    std::string modename = "NearNullSpace" + postfix.str();
    sp.push_back(&_nl.getVector(modename));
  }
}

void
FEProblem::computeNullSpace(NonlinearImplicitSystem & /*sys*/, std::vector<NumericVector<Number>*>& sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NullSpace"); ++i) {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_nl.getVector("NullSpace"+postfix.str()));
  }
}

Real
FEProblem::computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update)
{
  Moose::perf_log.push("compute_dampers()","Solve");

  // Default to no damping
  Real damping = 1.0;

  if (_has_dampers)
  {
    // Save pointer to the current solution
    const NumericVector<Number>* _saved_current_solution = _nl.currentSolution();

    _nl.setSolution(soln);
    // For now, do not re-compute auxiliary variables.  Doing so allows a wild solution increment
    //   to get to the material models, which may not be able to cope with drastically different
    //   values.  Once more complete dependency checking is in place, auxiliary variables (and
    //   material properties) will be computed as needed by dampers.
//    _aux.compute();
    damping = _nl.computeDamping(update);

    // restore saved solution
    _nl.setSolution(*_saved_current_solution);
  }

  Moose::perf_log.pop("compute_dampers()","Solve");

  return damping;
}

bool
FEProblem::shouldUpdateSolution()
{
  return false;
}

bool
FEProblem::updateSolution(NumericVector<Number>& /*vec_solution*/, NumericVector<Number>& /*ghosted_solution*/)
{
  return false;
}

void
FEProblem::predictorCleanup(NumericVector<Number>& /*ghosted_solution*/)
{
}

void
FEProblem::initDisplacedProblem(MooseMesh * displaced_mesh, InputParameters params)
{
  if (displaced_mesh == NULL)
    mooseError("Trying to set displaced mesh to NULL");
  _displaced_mesh = displaced_mesh;

  Moose::setup_perf_log.push("Create DisplacedProblem","Setup");
  params += parameters();
  _displaced_problem = new DisplacedProblem(*this, *_displaced_mesh, params);
  Moose::setup_perf_log.pop("Create DisplacedProblem","Setup");
}

void
FEProblem::updateGeomSearch(GeometricSearchData::GeometricSearchType type)
{
  _geometric_search_data.update(type);

  if (_displaced_problem)
    _displaced_problem->updateGeomSearch(type);
}

void
FEProblem::possiblyRebuildGeomSearchPatches()
{
  if (_displaced_problem) // Only need to do this if things are moving...
  {
    switch (_mesh.getPatchUpdateStrategy())
    {
      case 0: // Never
        break;
      case 2: // Auto
      {
        Real max = _displaced_problem->geomSearchData().maxPatchPercentage();
        _communicator.max(max);

        // If we haven't moved very far through the patch
        if (max < 0.4)
          break;
      }

      // Let this fall through if things do need to be updated...

      case 1: // Always
        // Flush output here to see the message before the reinitialization, which could take a while
        _console << "\n\nUpdating geometric search patches\n"<<std::endl;

        _geometric_search_data.clearNearestNodeLocators();
        _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

        _displaced_problem->geomSearchData().clearNearestNodeLocators();
        _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);

        reinitBecauseOfGhosting();

        // This is needed to reinitialize PETSc output
        initPetscOutput();
    }
  }
}

#ifdef LIBMESH_ENABLE_AMR
void
FEProblem::adaptMesh()
{
  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();
  for (unsigned int i = 0; i < cycles_per_step; ++i)
  {
    _console << "Adaptivity step " << i+1 << " of " << cycles_per_step << '\n';
    if (_adaptivity.adaptMesh())
      meshChanged();
  }
}
#endif //LIBMESH_ENABLE_AMR

void
FEProblem::meshChanged()
{
  if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
    _mesh.cacheChangedLists(); // Currently only used with adaptivity and stateful material properties

  // Clear these out because they corresponded to the old mesh
  _ghosted_elems.clear();

  ghostGhostedBoundaries();

  // mesh changed
  _eq.reinit();
  _mesh.meshChanged();

  // Since the Mesh changed, update the PointLocator object used by DiracKernels.
  _dirac_kernel_info.updatePointLocator(_mesh);

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->invalidateCache();

  // Need to redo ghosting
  _geometric_search_data.reinit();

  if (_displaced_problem != NULL)
  {
    _displaced_problem->meshChanged();
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  }

  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

  reinitBecauseOfGhosting();

  // We need to create new storage for the new elements and copy stateful properties from the old elements.
  if (_has_initialized_stateful && (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    {
      ProjectMaterialProperties pmp(true, *this, _nl, _material_data, _bnd_material_data, _material_props, _bnd_material_props, _materials, _assembly);
      Threads::parallel_reduce(*_mesh.refinedElementRange(), pmp);
    }

    {
      ProjectMaterialProperties pmp(false, *this, _nl, _material_data, _bnd_material_data, _material_props, _bnd_material_props, _materials, _assembly);
      Threads::parallel_reduce(*_mesh.coarsenedElementRange(), pmp);
    }

  }

  _has_jacobian = false;                    // we have to recompute jacobian when mesh changed

  for (std::vector<MeshChangedInterface *>::iterator it = _notify_when_mesh_changes.begin();
       it != _notify_when_mesh_changes.end();
       ++it)
    (*it)->meshChanged();
}

void
FEProblem::notifyWhenMeshChanges(MeshChangedInterface * mci)
{
  _notify_when_mesh_changes.push_back(mci);
}

void
FEProblem::checkProblemIntegrity()
{
  // Check for unsatisfied actions
  const std::set<SubdomainID> & mesh_subdomains = _mesh.meshSubdomains();

  // Check kernel coverage of subdomains (blocks) in the mesh
  if (_solve)
    _nl.checkKernelCoverage(mesh_subdomains, _kernel_coverage_check);

  // Check materials
  {
#ifdef LIBMESH_ENABLE_AMR
    if (_adaptivity.isOn() && (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
    {
      _console << "Using EXPERIMENTAL Stateful Material Property projection with Adaptivity!\n";

      if (n_processors() > 1)
      {
        if (_mesh.uniformRefineLevel() > 0 && _mesh.getMesh().skip_partitioning() == false)
          mooseError("This simulation is using uniform refinement on the mesh, with stateful properties and adaptivity. "
                     "You must skip partitioning to run this case:\nMesh/skip_partitioning=true");

        _console << "\nWarning! Mesh re-partitioning is disabled while using stateful material properties!  This can lead to large load imbalances and degraded performance!!\n\n";
        _mesh.getMesh().skip_partitioning(true);
        if (_displaced_problem)
          _displaced_problem->mesh().getMesh().skip_partitioning(true);
      }
    }
#endif

    std::set<SubdomainID> local_mesh_subs(mesh_subdomains);

    /**
     * If a material is specified for any block in the simulation, then all blocks must
     * have a material specified.
     */
    bool check_material_coverage = false;
    for (std::set<SubdomainID>::const_iterator i = _materials[0].blocks().begin(); i != _materials[0].blocks().end(); ++i)
    {
      local_mesh_subs.erase(*i);
      check_material_coverage = true;
    }
    // also exclude mortar spaces from the material check
    std::vector<MooseMesh::MortarInterface *> & mortar_ifaces = _mesh.getMortarInterfaces();
    for (std::vector<MooseMesh::MortarInterface *>::iterator it = mortar_ifaces.begin(); it != mortar_ifaces.end(); ++it)
      local_mesh_subs.erase((*it)->_id);

    // Check Material Coverage
    if (check_material_coverage && !local_mesh_subs.empty())
    {
      std::stringstream extra_subdomain_ids;
      /// unsigned int is necessary to print SubdomainIDs in the statement below
      std::copy (local_mesh_subs.begin(), local_mesh_subs.end(), std::ostream_iterator<unsigned int>(extra_subdomain_ids, " "));

      mooseError("The following blocks from your input mesh do not contain an active material: " + extra_subdomain_ids.str() + "\nWhen ANY mesh block contains a Material object, all blocks must contain a Material object.\n");
    }

    // Check material properties on blocks and boundaries
    checkBlockMatProps();
    //checkBoundaryMatProps();

    // Check that material properties exist when requested by other properties on a given block
    _materials[0].checkMaterialDependSanity();

    _materials[0].checkStatefulSanity();
  }

  // Check UserObjects and Postprocessors
  checkUserObjects();

  // Verify that we don't have any Element type/Coordinate Type conflicts
  checkCoordinateSystems();
}

void
FEProblem::checkUserObjects()
{
  // Check user_objects block coverage
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID> user_objects_blocks;

  // gather names of all user_objects that were defined in the input file
  // and the blocks that they are defined on
  std::set<std::string> names;
  for (unsigned int i = 0; i < Moose::exec_types.size(); i++)
  {
    for (std::vector<UserObject *>::const_iterator it = _user_objects(Moose::exec_types[i])[0].all().begin(); it != _user_objects(Moose::exec_types[i])[0].all().end(); ++it)
      names.insert((*it)->name());

    user_objects_blocks.insert(_user_objects(Moose::exec_types[i])[0].blockIds().begin(), _user_objects(Moose::exec_types[i])[0].blockIds().end());
  }

  // See if all referenced blocks are covered
  mesh_subdomains.insert(Moose::ANY_BLOCK_ID);
  std::set<SubdomainID> difference;
  std::set_difference(user_objects_blocks.begin(), user_objects_blocks.end(), mesh_subdomains.begin(), mesh_subdomains.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more UserObjects is referencing a nonexistent block:\n";
    for (std::set<SubdomainID>::iterator i = difference.begin(); i != difference.end();  ++i)
      oss << *i << "\n";
    mooseError(oss.str());
  }

  // check that all requested UserObjects were defined in the input file
  for (std::map<std::string, PostprocessorValue*>::const_iterator it = _pps_data[0]->values().begin(); it != _pps_data[0]->values().end(); ++it)
  {
    if (names.find(it->first) == names.end())
      mooseError("Postprocessor '" + it->first + "' requested but not specified in the input file.");
  }
}

void
FEProblem::checkCoordinateSystems()
{
  MeshBase::const_element_iterator it = _mesh.getMesh().elements_begin();
  MeshBase::const_element_iterator it_end = _mesh.getMesh().elements_end();

  for ( ; it != it_end; ++it)
  {
    SubdomainID sid = (*it)->subdomain_id();
    if (_coord_sys[sid] == Moose::COORD_RZ && (*it)->dim() == 3)
      mooseError("An RZ coordinate system was requested for subdomain " + Moose::stringify(sid) + " which contains 3D elements.");
    if (_coord_sys[sid] == Moose::COORD_RSPHERICAL && (*it)->dim() > 1)
      mooseError("An RSPHERICAL coordinate system was requested for subdomain " + Moose::stringify(sid) + " which contains 2D or 3D elements.");
  }
}

void
FEProblem::serializeSolution()
{
  _nl.serializeSolution();
  _aux.serializeSolution();
}

void
FEProblem::setRestartFile(const std::string & file_name)
{
  _app.setRestart(true);
  _resurrector->setRestartFile(file_name);
}

void
FEProblem::registerRestartableData(std::string name, RestartableDataValue * data, THREAD_ID tid)
{
  std::map<std::string, RestartableDataValue *> & restartable_data = _restartable_data[tid];

  if (restartable_data.find(name) != restartable_data.end())
    mooseError("Attempted to declare restartable twice with the same name: " << name);

  restartable_data[name] = data;
}

void
FEProblem::registerRecoverableData(std::string name)
{
  _recoverable_data.insert(name);
}

std::vector<VariableName>
FEProblem::getVariableNames()
{
  std::vector<VariableName> names;

  const std::vector<VariableName> & nl_var_names = _nl.getVariableNames();
  names.insert(names.end(), nl_var_names.begin(), nl_var_names.end());

  const std::vector<VariableName> & aux_var_names = _aux.getVariableNames();
  names.insert(names.end(), aux_var_names.begin(), aux_var_names.end());

  return names;
}

MooseNonlinearConvergenceReason
FEProblem::checkNonlinearConvergence(std::string &msg,
                                     const PetscInt it,
                                     const Real xnorm,
                                     const Real snorm,
                                     const Real fnorm,
                                     const Real rtol,
                                     const Real stol,
                                     const Real abstol,
                                     const PetscInt nfuncs,
                                     const PetscInt max_funcs,
                                     const Real ref_resid,
                                     const Real div_threshold)
{
  NonlinearSystem & system = getNonlinearSystem();
  MooseNonlinearConvergenceReason reason = MOOSE_NONLINEAR_ITERATING;

  std::ostringstream oss;
  if (fnorm != fnorm)
  {
    oss << "Failed to converge, function norm is NaN\n";
    reason = MOOSE_DIVERGED_FNORM_NAN;
  }
  else if (fnorm < abstol)
  {
    oss << "Converged due to function norm " << fnorm << " < " << abstol << '\n';
    reason = MOOSE_CONVERGED_FNORM_ABS;
  }
  else if (nfuncs >= max_funcs)
  {
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs << '\n';
    reason = MOOSE_DIVERGED_FUNCTION_COUNT;
  }
  else if (it &&
          fnorm > system._last_nl_rnorm &&
          fnorm >= div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    reason = MOOSE_DIVERGED_LINE_SEARCH;
  }

  if (it && !reason)
  {
    if (fnorm <= ref_resid*rtol)
    {
      oss << "Converged due to function norm " << fnorm << " < " << " (relative tolerance)\n";
      reason = MOOSE_CONVERGED_FNORM_RELATIVE;
    }
    else if (snorm < stol*xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm << '\n';
      reason = MOOSE_CONVERGED_SNORM_RELATIVE;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  msg = oss.str();

  return(reason);
}



MooseLinearConvergenceReason
FEProblem::checkLinearConvergence(std::string & /*msg*/,
                                  const PetscInt n,
                                  const Real rnorm,
                                  const Real /*rtol*/,
                                  const Real /*atol*/,
                                  const Real /*dtol*/,
                                  const PetscInt maxits)
{
  // We initialize the reason to something that basically means MOOSE
  // has not made a decision on convergence yet.
  MooseLinearConvergenceReason reason = MOOSE_LINEAR_ITERATING;

  // Get a reference to our Nonlinear System
  NonlinearSystem & system = getNonlinearSystem();

  // If it's the beginning of a new set of iterations, reset
  // last_rnorm, otherwise record the most recent linear residual norm
  // in the NonlinearSystem.
  if (n == 0)
    system._last_rnorm = 1e99;
  else
    system._last_rnorm = rnorm;

  // If the linear residual norm is less than the System's linear absolute
  // step tolerance, we consider it to be converged and set the reason as
  // MOOSE_CONVERGED_RTOL.
  if (std::abs(rnorm - system._last_rnorm) < system._l_abs_step_tol)
    reason = MOOSE_CONVERGED_RTOL;

  // If we hit max its, then we consider that converged (rather than
  // KSP_DIVERGED_ITS).
  if (n >= maxits)
    reason = MOOSE_CONVERGED_ITS;

  // If either of our convergence criteria is met, store the number of linear
  // iterations in the System.
  if (reason == MOOSE_CONVERGED_ITS || reason == MOOSE_CONVERGED_RTOL)
    system._current_l_its.push_back(static_cast<unsigned int>(n));

  return reason;
}

#ifdef LIBMESH_HAVE_PETSC
void
FEProblem::storePetscOptions(const MultiMooseEnum & petsc_options,
                             const std::vector<std::string> & petsc_options_inames,
                             const std::vector<std::string> & petsc_options_values)
{
  MultiMooseEnum & po = parameters().set<MultiMooseEnum>("petsc_options");         // set because we need a writable reference

  for (MooseEnumIterator it = petsc_options.begin(); it != petsc_options.end(); ++it)
  {
    /**
     * "-log_summary" cannot be used in the input file. This option needs to be set when PETSc is initialized
     * which happens before the parser is even created.  We'll throw an error if somebody attempts to add this option later.
     */
    if (*it == "-log_summary")
      mooseError("The PETSc option \"-log_summary\" can only be used on the command line.  Please remove it from the input file");

    // Warn about superseded PETSc options (Note: -snes is not a REAL option, but people used it in their input files)
    else
    {
      std::string help_string;
      if (*it == "-snes" || *it == "-snes_mf" || *it == "-snes_mf_operator")
        help_string = "Please set the solver type through \"solve_type\".";
      else if (*it == "-ksp_monitor")
        help_string = "Please use \"Outputs/console/type=Console Outputs/console/linear_residuals=true\"";

      if (help_string != "")
        mooseWarning("The PETSc option " << *it << " should not be used directly in a MOOSE input file. " << help_string);
    }

    if (find(po.begin(), po.end(), *it) == po.end())
      po.push_back(*it);
  }

  std::vector<std::string> & pn = parameters().set<std::vector<std::string> >("petsc_inames");         // set because we need a writable reference
  std::vector<std::string> & pv = parameters().set<std::vector<std::string> >("petsc_values");         // set because we need a writable reference

  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("PETSc names and options are not the same length");

  bool boomeramg_found = false;
  bool strong_threshold_found = false;
  _pc_description = "";
  for (unsigned int i = 0; i < petsc_options_inames.size(); i++)
  {
    if (find(pn.begin(), pn.end(), petsc_options_inames[i]) == pn.end())
    {
      pn.push_back(petsc_options_inames[i]);
      pv.push_back(petsc_options_values[i]);

      // Look for a pc description
      if (petsc_options_inames[i] == "-pc_type" || petsc_options_inames[i] == "-pc_sub_type" || petsc_options_inames[i] == "-pc_hypre_type")
        _pc_description += petsc_options_values[i] + ' ';

      // This special case is common enough that we'd like to handle it for the user.
      if (petsc_options_inames[i] == "-pc_hypre_type" && petsc_options_values[i] == "boomeramg")
        boomeramg_found = true;
      if (petsc_options_inames[i] == "-pc_hypre_boomeramg_strong_threshold")
        strong_threshold_found = true;
    }
    else
    {
      for (unsigned int j = 0; j < pn.size(); j++)
        if (pn[j] == petsc_options_inames[i])
          pv[j] = petsc_options_values[i];
    }
  }

  // When running a 3D mesh with boomeramg, it is almost always best to supply a strong threshold value
  // We will provide that for the user here if they haven't supplied it themselves.
  if (boomeramg_found && !strong_threshold_found && _mesh.dimension() == 3)
  {
    pn.push_back("-pc_hypre_boomeramg_strong_threshold");
    pv.push_back("0.7");
    _pc_description += "strong_threshold: 0.7 (auto)";
  }
}
#endif

SolverParams &
FEProblem::solverParams()
{
  return _solver_params;
}

void
FEProblem::registerRandomInterface(RandomInterface & random_interface, const std::string & name)
{
  RandomData *random_data;
  if (_random_data_objects.find(name) == _random_data_objects.end())
  {
    random_data = new RandomData(*this, random_interface);
    random_interface.setRandomDataPointer(random_data);

    _random_data_objects[name] = random_data;
  }
  else
    random_interface.setRandomDataPointer(_random_data_objects[name]);
}

bool
FEProblem::needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid)
{
  if (_bnd_mat_side_cache[tid].find(bnd_id) == _bnd_mat_side_cache[tid].end())
  {
    _bnd_mat_side_cache[tid][bnd_id] = false;

    if (_nl.needMaterialOnSide(bnd_id, tid) || _aux.needMaterialOnSide(bnd_id))
      _bnd_mat_side_cache[tid][bnd_id] = true;
    else
    {
      for (unsigned int i=0; i < Moose::exec_types.size(); ++i)
        if (!_user_objects(Moose::exec_types[i])[tid].sideUserObjects(bnd_id).empty() ||
            !_user_objects(Moose::exec_types[i])[tid].sideUserObjects(Moose::ANY_BOUNDARY_ID).empty())
        {
          _bnd_mat_side_cache[tid][bnd_id] = true;
          break;
        }
    }
  }

  return _bnd_mat_side_cache[tid][bnd_id];
}

bool
FEProblem::needMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid)
{
  if (_block_mat_side_cache[tid].find(subdomain_id) == _block_mat_side_cache[tid].end())
  {
    _block_mat_side_cache[tid][subdomain_id] = false;

    if (_nl.needMaterialOnSide(subdomain_id, tid))
      _block_mat_side_cache[tid][subdomain_id] = true;
    else
    {
      for (unsigned int i=0; i < Moose::exec_types.size(); ++i)
        if (!_user_objects(Moose::exec_types[i])[tid].internalSideUserObjects(subdomain_id).empty() ||
            !_user_objects(Moose::exec_types[i])[tid].internalSideUserObjects(Moose::ANY_BLOCK_ID).empty())
        {
          _block_mat_side_cache[tid][subdomain_id] = true;
          break;
        }
    }
  }

  return _block_mat_side_cache[tid][subdomain_id];
}
