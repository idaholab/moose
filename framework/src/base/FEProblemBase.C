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

#include "FEProblemBase.h"
#include "AuxiliarySystem.h"
#include "MaterialPropertyStorage.h"
#include "MooseEnum.h"
#include "Resurrector.h"
#include "Factory.h"
#include "MooseUtils.h"
#include "DisplacedProblem.h"
#include "SystemBase.h"
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
#include "NonlinearSystem.h"
#include "Distribution.h"
#include "Sampler.h"
#include "PetscSupport.h"
#include "RandomInterface.h"
#include "RandomData.h"
#include "MooseEigenSystem.h"
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
#include "MultiAppTransfer.h"
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
#include "Predictor.h"
#include "Assembly.h"
#include "Control.h"
#include "XFEMInterface.h"
#include "ConsoleUtils.h"
#include "NonlocalKernel.h"
#include "NonlocalIntegratedBC.h"
#include "ShapeElementUserObject.h"
#include "ShapeSideUserObject.h"
#include "MooseVariableScalar.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/quadrature.h"
#include "libmesh/coupling_matrix.h"
#include "libmesh/nonlinear_solver.h"

// Anonymous namespace for helper function
namespace
{
/**
 * Method for sorting the MooseVariables based on variable numbers
 */
bool
sortMooseVariables(MooseVariable * a, MooseVariable * b)
{
  return a->number() < b->number();
}
}

Threads::spin_mutex get_function_mutex;

template <>
InputParameters
validParams<FEProblemBase>()
{
  InputParameters params = validParams<SubProblem>();
  params.addPrivateParam<MooseMesh *>("mesh");
  params.addParam<unsigned int>("null_space_dimension", 0, "The dimension of the nullspace");
  params.addParam<unsigned int>(
      "transpose_null_space_dimension", 0, "The dimension of the transpose nullspace");
  params.addParam<unsigned int>(
      "near_null_space_dimension", 0, "The dimension of the near nullspace");
  params.addParam<bool>("solve",
                        true,
                        "Whether or not to actually solve the Nonlinear system.  "
                        "This is handy in the case that all you want to do is "
                        "execute AuxKernels, Transfers, etc. without actually "
                        "solving anything");
  params.addParam<bool>("use_nonlinear",
                        true,
                        "Determines whether to use a Nonlinear vs a "
                        "Eigenvalue system (Automatically determined based "
                        "on executioner)");
  params.addParam<bool>("error_on_jacobian_nonzero_reallocation",
                        false,
                        "This causes PETSc to error if it had to reallocate memory in the Jacobian "
                        "matrix due to not having enough nonzeros");
  params.addParam<bool>("ignore_zeros_in_jacobian",
                        false,
                        "Do not explicitly store zero values in "
                        "the Jacobian matrix if true");
  params.addParam<bool>("force_restart",
                        false,
                        "EXPERIMENTAL: If true, a sub_app may use a "
                        "restart file instead of using of using the master "
                        "backup file");
  params.addParam<bool>("skip_additional_restart_data",
                        false,
                        "True to skip additional data in equation system for restart. It is useful "
                        "for starting a transient calculation with a steady-state solution");

  return params;
}

FEProblemBase::FEProblemBase(const InputParameters & parameters)
  : SubProblem(parameters),
    Restartable(parameters, "FEProblemBase", this),
    _mesh(*parameters.get<MooseMesh *>("mesh")),
    _eq(_mesh),
    _initialized(false),
    _kernel_type(Moose::KT_ALL),
    _solve(getParam<bool>("solve")),

    _transient(false),
    _time(declareRestartableData<Real>("time")),
    _time_old(declareRestartableData<Real>("time_old")),
    _t_step(declareRecoverableData<int>("t_step")),
    _dt(declareRestartableData<Real>("dt")),
    _dt_old(declareRestartableData<Real>("dt_old")),

    _nl(NULL),
    _aux(NULL),
    _coupling(Moose::COUPLING_DIAG),
    _distributions(/*threaded=*/false),
    _scalar_ics(/*threaded=*/false),
    _material_props(
        declareRestartableDataWithContext<MaterialPropertyStorage>("material_props", &_mesh)),
    _bnd_material_props(
        declareRestartableDataWithContext<MaterialPropertyStorage>("bnd_material_props", &_mesh)),
    _pps_data(*this),
    _vpps_data(*this),
    _general_user_objects(/*threaded=*/false),
    _transfers(/*threaded=*/false),
    _to_multi_app_transfers(/*threaded=*/false),
    _from_multi_app_transfers(/*threaded=*/false),
#ifdef LIBMESH_ENABLE_AMR
    _adaptivity(*this),
    _cycles_completed(0),
#endif
    _displaced_mesh(NULL),
    _geometric_search_data(*this, _mesh),
    _reinit_displaced_elem(false),
    _reinit_displaced_face(false),
    _input_file_saved(false),
    _has_dampers(false),
    _has_constraints(false),
    _has_initialized_stateful(false),
    _const_jacobian(false),
    _has_jacobian(false),
    _needs_old_newton_iter(false),
    _has_nonlocal_coupling(false),
    _calculate_jacobian_in_uo(false),
    _kernel_coverage_check(false),
    _material_coverage_check(false),
    _max_qps(std::numeric_limits<unsigned int>::max()),
    _max_shape_funcs(std::numeric_limits<unsigned int>::max()),
    _max_scalar_order(INVALID_ORDER),
    _has_time_integrator(false),
    _has_exception(false),
    _current_execute_on_flag(EXEC_NONE),
    _error_on_jacobian_nonzero_reallocation(
        getParam<bool>("error_on_jacobian_nonzero_reallocation")),
    _ignore_zeros_in_jacobian(getParam<bool>("ignore_zeros_in_jacobian")),
    _force_restart(getParam<bool>("force_restart")),
    _skip_additional_restart_data(getParam<bool>("skip_additional_restart_data")),
    _fail_next_linear_convergence_check(false),
    _currently_computing_jacobian(false),
    _started_initial_setup(false)
{

  _time = 0.0;
  _time_old = 0.0;
  _t_step = 0;
  _dt = 0;
  _dt_old = _dt;

  unsigned int n_threads = libMesh::n_threads();

  _real_zero.resize(n_threads, 0.);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);
  _second_phi_zero.resize(n_threads);
  _uo_jacobian_moose_vars.resize(n_threads);

  _material_data.resize(n_threads);
  _bnd_material_data.resize(n_threads);
  _neighbor_material_data.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _material_data[i] = std::make_shared<MaterialData>(_material_props);
    _bnd_material_data[i] = std::make_shared<MaterialData>(_bnd_material_props);
    _neighbor_material_data[i] = std::make_shared<MaterialData>(_bnd_material_props);
  }

  _active_elemental_moose_variables.resize(n_threads);

  _block_mat_side_cache.resize(n_threads);
  _bnd_mat_side_cache.resize(n_threads);

  _resurrector = libmesh_make_unique<Resurrector>(*this);

  _eq.parameters.set<FEProblemBase *>("_fe_problem_base") = this;
}

void
FEProblemBase::newAssemblyArray(NonlinearSystemBase & nl)
{
  unsigned int n_threads = libMesh::n_threads();

  _assembly.resize(n_threads);
  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i] = new Assembly(nl, i);
}

void
FEProblemBase::deleteAssemblyArray()
{
  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    delete _assembly[i];
  }
}

void
FEProblemBase::initNullSpaceVectors(const InputParameters & parameters, NonlinearSystemBase & nl)
{
  unsigned int dimNullSpace = parameters.get<unsigned int>("null_space_dimension");
  unsigned int dimTransposeNullSpace =
      parameters.get<unsigned int>("transpose_null_space_dimension");
  unsigned int dimNearNullSpace = parameters.get<unsigned int>("near_null_space_dimension");
  for (unsigned int i = 0; i < dimNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace
    // builder might march over all nodes
    nl.addVector("NullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["NullSpace"] = dimNullSpace;
  for (unsigned int i = 0; i < dimTransposeNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near nullspace
    // builder might march over all nodes
    nl.addVector("TransposeNullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["TransposeNullSpace"] = dimTransposeNullSpace;
  for (unsigned int i = 0; i < dimNearNullSpace; ++i)
  {
    std::ostringstream oss;
    oss << "_" << i;
    // do not project, since this will be recomputed, but make it ghosted, since the near-nullspace
    // builder might march over all semilocal nodes
    nl.addVector("NearNullSpace" + oss.str(), false, GHOSTED);
  }
  _subspace_dim["NearNullSpace"] = dimNearNullSpace;
}

FEProblemBase::~FEProblemBase()
{
  // Flush the Console stream, the underlying call to Console::mooseConsole
  // relies on a call to Output::checkInterval that has references to
  // _time, etc. If it is not flushed here memory problems arise if you have
  // an unflushed stream and start destructing things.
  _console << std::flush;

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; i++)
  {
    _zero[i].release();
    _grad_zero[i].release();
    _second_zero[i].release();
    _second_phi_zero[i].release();
  }
}

Moose::CoordinateSystemType
FEProblemBase::getCoordSystem(SubdomainID sid)
{
  std::map<SubdomainID, Moose::CoordinateSystemType>::iterator it = _coord_sys.find(sid);
  if (it != _coord_sys.end())
    return (*it).second;
  else
    mooseError("Requested subdomain ", sid, " does not exist.");
}

void
FEProblemBase::setCoordSystem(const std::vector<SubdomainName> & blocks,
                              const MultiMooseEnum & coord_sys)
{
  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();
  if (blocks.size() == 0)
  {
    // no blocks specified -> assume the whole domain
    Moose::CoordinateSystemType coord_type = Moose::COORD_XYZ; // all is going to be XYZ by default
    if (coord_sys.size() == 0)
      ; // relax, do nothing
    else if (coord_sys.size() == 1)
      coord_type = Moose::stringToEnum<Moose::CoordinateSystemType>(
          coord_sys[0]); // one system specified, the whole domain is going to have that system
    else
      mooseError("Multiple coordinate systems specified, but no blocks given.");

    for (const auto & sbd : subdomains)
      _coord_sys[sbd] = coord_type;
  }
  else
  {
    // user specified 'blocks' but not coordinate systems
    if (coord_sys.size() == 0)
    {
      // set all blocks to cartesian coordinate system
      for (const auto & block : blocks)
      {
        SubdomainID sid = _mesh.getSubdomainID(block);
        _coord_sys[sid] = Moose::COORD_XYZ;
      }
    }
    else if (coord_sys.size() == 1)
    {
      // set all blocks to the coordinate system specified by `coord_sys[0]`
      Moose::CoordinateSystemType coord_type =
          Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[0]);
      for (const auto & block : blocks)
      {
        SubdomainID sid = _mesh.getSubdomainID(block);
        _coord_sys[sid] = coord_type;
      }
    }
    else
    {
      if (blocks.size() != coord_sys.size())
        mooseError("Number of blocks and coordinate systems does not match.");

      for (unsigned int i = 0; i < blocks.size(); i++)
      {
        SubdomainID sid = _mesh.getSubdomainID(blocks[i]);
        Moose::CoordinateSystemType coord_type =
            Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[i]);
        _coord_sys[sid] = coord_type;
      }

      for (const auto & sid : subdomains)
        if (_coord_sys.find(sid) == _coord_sys.end())
          mooseError("Subdomain '" + Moose::stringify(sid) +
                     "' does not have a coordinate system specified.");
    }
  }
}

void
FEProblemBase::setAxisymmetricCoordAxis(const MooseEnum & rz_coord_axis)
{
  _rz_coord_axis = rz_coord_axis;
}

void
FEProblemBase::addExtraVectors()
{
  _nl->addExtraVectors();
  _aux->addExtraVectors();
}

void
FEProblemBase::initialSetup()
{
  Moose::perf_log.push("initialSetup()", "Setup");

  // set state flag indicating that we are in or beyond initialSetup.
  // This can be used to throw errors in methods that _must_ be called at construction time.
  _started_initial_setup = true;

  addExtraVectors();

  // Perform output related setups
  _app.getOutputWarehouse().initialSetup();

  // Flush all output to _console that occur during construction and initialization of objects
  _app.getOutputWarehouse().mooseConsole();

  if (_app.isRecovering() && (_app.isUltimateMaster() || _force_restart))
  {
    _resurrector->setRestartFile(_app.getRecoverFileBase());
    if (_app.getRecoverFileSuffix() == "cpa")
      _resurrector->setRestartSuffix("xda");
  }

  if ((_app.isRestarting() || _app.isRecovering()) && (_app.isUltimateMaster() || _force_restart))
    _resurrector->restartFromFile();
  else
  {
    ExodusII_IO * reader = _mesh.exReader();

    if (reader != NULL)
    {
      _nl->copyVars(*reader);
      _aux->copyVars(*reader);
    }
  }

  // Build Refinement and Coarsening maps for stateful material projections if necessary
  if (_adaptivity.isOn() &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    Moose::perf_log.push("mesh.buildRefinementAndCoarseningMaps()", "Setup");
    _mesh.buildRefinementAndCoarseningMaps(_assembly[0]);
    Moose::perf_log.pop("mesh.buildRefinementAndCoarseningMaps()", "Setup");
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
      if (!_app.isUltimateMaster())
        mooseError(
            "Doing extra refinements when restarting is NOT supported for sub-apps of a MultiApp");

      Moose::perf_log.push("Uniformly Refine Mesh", "Setup");
      adaptivity().uniformRefineWithProjection();
      Moose::perf_log.pop("Uniformly Refine Mesh", "Setup");
    }
  }

  // Do this just in case things have been done to the mesh
  ghostGhostedBoundaries();
  _mesh.meshChanged();
  if (_displaced_problem)
    _displaced_mesh->meshChanged();

  unsigned int n_threads = libMesh::n_threads();

  // UserObject initialSetup
  std::set<std::string> depend_objects_ic = _ics.getDependObjects();
  std::set<std::string> depend_objects_aux = _aux->getDependObjects();

  _general_user_objects.updateDependObjects(depend_objects_ic, depend_objects_aux);
  _general_user_objects.initialSetup();
  _general_user_objects.sort();

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _nodal_user_objects.updateDependObjects(depend_objects_ic, depend_objects_aux, tid);
    _nodal_user_objects.initialSetup(tid);

    _elemental_user_objects.updateDependObjects(depend_objects_ic, depend_objects_aux, tid);
    _elemental_user_objects.initialSetup(tid);

    _side_user_objects.updateDependObjects(depend_objects_ic, depend_objects_aux, tid);
    _side_user_objects.initialSetup(tid);

    _internal_side_user_objects.updateDependObjects(depend_objects_ic, depend_objects_aux, tid);
    _internal_side_user_objects.initialSetup(tid);
  }

  // check if jacobian calculation is done in userobject
  for (THREAD_ID tid = 0; tid < n_threads; ++tid)
    checkUserObjectJacobianRequirement(tid);
  // Check whether nonlocal couling is required or not
  checkNonlocalCoupling();
  if (_requires_nonlocal_coupling)
    setVariableAllDoFMap(_uo_jacobian_moose_vars[0]);

  // Call the initialSetup methods for functions
  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    reinitScalars(
        tid); // initialize scalars so they are properly sized for use as input into ParsedFunctions
    _functions.initialSetup(tid);
  }

  if (!_app.isRecovering())
  {
    computeUserObjects(EXEC_INITIAL, Moose::PRE_IC);

    for (THREAD_ID tid = 0; tid < n_threads; tid++)
      _ics.initialSetup(tid);
    _scalar_ics.sort();
    projectSolution();
  }

  // Materials
  if (_all_materials.hasActiveObjects(0))
  {
    for (THREAD_ID tid = 0; tid < n_threads; tid++)
    {
      // Sort the Material objects, these will be actually computed by MOOSE in reinit methods.
      _materials.sort(tid);

      // Call initialSetup on both Material and Material objects
      _all_materials.initialSetup(tid);
    }

    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeMaterialsObjectThread cmt(*this,
                                     _material_data,
                                     _bnd_material_data,
                                     _neighbor_material_data,
                                     _material_props,
                                     _bnd_material_props,
                                     _assembly);
    /**
     * The ComputeMaterialObjectThread object now allocates memory as needed for the material
     * storage system.
     * This cannot be done with threads. The first call to this object bypasses threading by calling
     * the object
     * directly. The subsequent call can be called with threads.
     */
    cmt(elem_range, true);

    if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
      _has_initialized_stateful = true;
  }

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _internal_side_indicators.initialSetup(tid);
    _indicators.initialSetup(tid);
    _markers.sort(tid);
    _markers.initialSetup(tid);
  }

#ifdef LIBMESH_ENABLE_AMR

  if (!_app.isRecovering())
  {
    Moose::perf_log.push("initial adaptivity", "Setup");

    unsigned int n = adaptivity().getInitialSteps();
    if (n && !_app.isUltimateMaster() && _app.isRestarting())
      mooseError("Cannot perform initial adaptivity during restart on sub-apps of a MultiApp!");

    initialAdaptMesh();
    Moose::perf_log.pop("initial adaptivity", "Setup");
  }

#endif // LIBMESH_ENABLE_AMR

  if (!_app.isRecovering() && !_app.isRestarting())
  {
    // During initial setup the solution is copied to solution_old and solution_older
    copySolutionsBackwards();
  }

  if (!_app.isRecovering())
  {
    if (haveXFEM() && updateMeshXFEM())
      _console << "XFEM updated mesh on initializaton" << std::endl;
  }

  // Call initialSetup on the nonlinear system
  _nl->initialSetup();

  // Auxilary variable initialSetup calls
  _aux->initialSetup();

  _nl->setSolution(*(_nl->system().current_local_solution.get()));

  Moose::perf_log.push("Initial updateGeomSearch()", "Setup");
  // Update the nearest node searches (has to be called after the problem is all set up)
  // We do this here because this sets up the Element's DoFs to ghost
  updateGeomSearch(GeometricSearchData::NEAREST_NODE);
  Moose::perf_log.pop("Initial updateGeomSearch()", "Setup");

  Moose::perf_log.push("Initial updateActiveSemiLocalNodeRange()", "Setup");
  _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);
  if (_displaced_mesh)
    _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);
  Moose::perf_log.pop("Initial updateActiveSemiLocalNodeRange()", "Setup");

  Moose::perf_log.push("reinit() after updateGeomSearch()", "Setup");
  // Possibly reinit one more time to get ghosting correct
  reinitBecauseOfGhostingOrNewGeomObjects();
  Moose::perf_log.pop("reinit() after updateGeomSearch()", "Setup");

  if (_displaced_mesh)
    _displaced_problem->updateMesh();

  Moose::perf_log.push("Initial updateGeomSearch()", "Setup");
  updateGeomSearch(); // Call all of the rest of the geometric searches
  Moose::perf_log.pop("Initial updateGeomSearch()", "Setup");

  // Random interface objects
  for (const auto & it : _random_data_objects)
    it.second->updateSeeds(EXEC_INITIAL);

  if (_app.isRestarting() || _app.isRecovering())
  {
    if (_app.hasCachedBackup()) // This happens when this app is a sub-app and has been given a
                                // Backup
      _app.restoreCachedBackup();
    else
      _resurrector->restartRestartableData();

    // We may have just clobbered initial conditions that were explicitly set
    // In a _restart_ scenario it is completely valid to specify new initial conditions
    // for some of the variables which should override what's coming from the restart file
    if (!_app.isRecovering())
    {
      for (THREAD_ID tid = 0; tid < n_threads; tid++)
        _ics.initialSetup(tid);
      _scalar_ics.sort();
      projectSolution();
    }
  }

  // HUGE NOTE: MultiApp initialSetup() MUST... I repeat MUST be _after_ restartable data has been
  // restored

  // Call initialSetup on the MultiApps
  if (_multi_apps.hasActiveObjects())
  {
    _console << COLOR_CYAN << "Initializing MultiApps" << COLOR_DEFAULT << std::endl;
    _multi_apps.initialSetup();
    _console << COLOR_CYAN << "Finished Initializing MultiApps" << COLOR_DEFAULT << std::endl;
  }

  // Call initialSetup on the transfers
  _transfers.initialSetup();
  _to_multi_app_transfers.initialSetup();
  _from_multi_app_transfers.initialSetup();

  if (!_app.isRecovering())
  {
    _current_execute_on_flag = EXEC_INITIAL;

    Moose::perf_log.push("execTransfers()", "Setup");
    execTransfers(EXEC_INITIAL);
    Moose::perf_log.pop("execTransfers()", "Setup");

    Moose::perf_log.push("execMultiApps()", "Setup");
    bool converged = execMultiApps(EXEC_INITIAL);
    if (!converged)
      mooseError("failed to converge initial MultiApp");

    // We'll backup the Multiapp here
    backupMultiApps(EXEC_INITIAL);
    Moose::perf_log.pop("execMultiApps()", "Setup");

    // Yak is currently relying on doing this after initial Transfers
    Moose::setup_perf_log.push("computeUserObjects()", "Setup");

    // TODO: user object evaluation could fail.
    computeUserObjects(EXEC_INITIAL, Moose::PRE_AUX);

    Moose::setup_perf_log.push("computeAux()", "Setup");
    _aux->compute(EXEC_INITIAL);
    Moose::setup_perf_log.pop("computeAux()", "Setup");

    // The only user objects that should be computed here are the initial UOs
    computeUserObjects(EXEC_INITIAL, Moose::POST_AUX);

    Moose::setup_perf_log.pop("computeUserObjects()", "Setup");

    _current_execute_on_flag = EXEC_NONE;
  }

  // Here we will initialize the stateful properties once more since they may have been updated
  // during initialSetup by calls to computeProperties.
  //
  // It's really bad that we don't allow this during restart.  It means that we can't add new
  // stateful materials
  // during restart.  This is only happening because this _has_ to be below initial userobject
  // execution.
  // Otherwise this could be done up above... _before_ restoring restartable data... which would
  // allow you to have
  // this happen during restart.  I honestly have no idea why this has to happen after initial user
  // object computation.
  // THAT is something we should fix... so I've opened this ticket: #5804
  if (!_app.isRecovering() && !_app.isRestarting() &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeMaterialsObjectThread cmt(*this,
                                     _material_data,
                                     _bnd_material_data,
                                     _neighbor_material_data,
                                     _material_props,
                                     _bnd_material_props,
                                     _assembly);
    Threads::parallel_reduce(elem_range, cmt);
  }

  // Control Logic
  executeControls(EXEC_INITIAL);

  // Scalar variables need to reinited for the initial conditions to be available for output
  for (unsigned int tid = 0; tid < n_threads; tid++)
    reinitScalars(tid);

  if (_displaced_mesh)
    _displaced_problem->syncSolutions();

  // Writes all calls to _console from initialSetup() methods
  _app.getOutputWarehouse().mooseConsole();

  Moose::perf_log.pop("initialSetup()", "Setup");

  if (_requires_nonlocal_coupling)
  {
    setNonlocalCouplingMatrix();
    for (THREAD_ID tid = 0; tid < n_threads; ++tid)
      _assembly[tid]->initNonlocalCoupling();
  }
}

void
FEProblemBase::timestepSetup()
{
  unsigned int n_threads = libMesh::n_threads();

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _all_materials.timestepSetup(tid);
    _functions.timestepSetup(tid);
  }

  _aux->timestepSetup();
  _nl->timestepSetup();

  // Random interface objects
  for (const auto & it : _random_data_objects)
    it.second->updateSeeds(EXEC_TIMESTEP_BEGIN);

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _internal_side_indicators.timestepSetup(tid);
    _indicators.timestepSetup(tid);
    _markers.timestepSetup(tid);

    // Timestep setup of all UserObjects
    _nodal_user_objects.timestepSetup(tid);
    _elemental_user_objects.timestepSetup(tid);
    _side_user_objects.timestepSetup(tid);
    _internal_side_user_objects.timestepSetup(tid);
  }
  _general_user_objects.timestepSetup();

  // Timestep setup of output objects
  _app.getOutputWarehouse().timestepSetup();

  if (_requires_nonlocal_coupling)
    if (_nonlocal_kernels.hasActiveObjects() || _nonlocal_integrated_bcs.hasActiveObjects())
      _has_nonlocal_coupling = true;
}

unsigned int
FEProblemBase::getMaxQps() const
{
  if (_max_qps == std::numeric_limits<unsigned int>::max())
    mooseError("Max QPS uninitialized");
  return _max_qps;
}

unsigned int
FEProblemBase::getMaxShapeFunctions() const
{
  if (_max_shape_funcs == std::numeric_limits<unsigned int>::max())
    mooseError("Max shape functions uninitialized");
  return _max_shape_funcs;
}

Order
FEProblemBase::getMaxScalarOrder() const
{
  return _max_scalar_order;
}

void
FEProblemBase::checkNonlocalCoupling()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    const KernelWarehouse & all_kernels = _nl->getKernelWarehouse();
    const auto & kernels = all_kernels.getObjects(tid);
    for (const auto & kernel : kernels)
    {
      std::shared_ptr<NonlocalKernel> nonlocal_kernel =
          std::dynamic_pointer_cast<NonlocalKernel>(kernel);
      if (nonlocal_kernel)
      {
        if (_calculate_jacobian_in_uo)
          _requires_nonlocal_coupling = true;
        _nonlocal_kernels.addObject(kernel, tid);
      }
    }
    const MooseObjectWarehouse<IntegratedBC> & all_integrated_bcs = _nl->getIntegratedBCWarehouse();
    const auto & integrated_bcs = all_integrated_bcs.getObjects(tid);
    for (const auto & integrated_bc : integrated_bcs)
    {
      std::shared_ptr<NonlocalIntegratedBC> nonlocal_integrated_bc =
          std::dynamic_pointer_cast<NonlocalIntegratedBC>(integrated_bc);
      if (nonlocal_integrated_bc)
      {
        if (_calculate_jacobian_in_uo)
          _requires_nonlocal_coupling = true;
        _nonlocal_integrated_bcs.addObject(integrated_bc, tid);
      }
    }
  }
}

void
FEProblemBase::checkUserObjectJacobianRequirement(THREAD_ID tid)
{
  std::set<MooseVariable *> uo_jacobian_moose_vars;
  const auto & e_objects = _elemental_user_objects.getActiveObjects(tid);
  for (const auto & uo : e_objects)
  {
    std::shared_ptr<ShapeElementUserObject> shape_element_uo =
        std::dynamic_pointer_cast<ShapeElementUserObject>(uo);
    if (shape_element_uo)
    {
      _calculate_jacobian_in_uo = shape_element_uo->computeJacobianFlag();
      const std::set<MooseVariable *> & mv_deps = shape_element_uo->jacobianMooseVariables();
      uo_jacobian_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }
  const auto & s_objects = _side_user_objects.getActiveObjects(tid);
  for (const auto & uo : s_objects)
  {
    std::shared_ptr<ShapeSideUserObject> shape_side_uo =
        std::dynamic_pointer_cast<ShapeSideUserObject>(uo);
    if (shape_side_uo)
    {
      _calculate_jacobian_in_uo = shape_side_uo->computeJacobianFlag();
      const std::set<MooseVariable *> & mv_deps = shape_side_uo->jacobianMooseVariables();
      uo_jacobian_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }
  }
  _uo_jacobian_moose_vars[tid].assign(uo_jacobian_moose_vars.begin(), uo_jacobian_moose_vars.end());
  std::sort(
      _uo_jacobian_moose_vars[tid].begin(), _uo_jacobian_moose_vars[tid].end(), sortMooseVariables);
}

void
FEProblemBase::setVariableAllDoFMap(const std::vector<MooseVariable *> moose_vars)
{
  for (unsigned int i = 0; i < moose_vars.size(); ++i)
  {
    VariableName var_name = moose_vars[i]->name();
    _nl->setVariableGlobalDoFs(var_name);
    _var_dof_map[var_name] = _nl->getVariableGlobalDoFs();
  }
}

void
FEProblemBase::prepare(const Elem * elem, THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _nl->prepare(tid);
  _aux->prepare(tid);
  _assembly[tid]->prepare();
  if (_has_nonlocal_coupling)
    _assembly[tid]->prepareNonlocal();

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
  {
    _displaced_problem->prepare(_displaced_mesh->elemPtr(elem->id()), tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }
}

void
FEProblemBase::prepareFace(const Elem * elem, THREAD_ID tid)
{
  _nl->prepareFace(tid, true);
  _aux->prepareFace(tid, false);

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->prepareFace(_displaced_mesh->elemPtr(elem->id()), tid);
}

void
FEProblemBase::prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       THREAD_ID tid)
{
  _assembly[tid]->reinit(elem);

  _nl->prepare(tid);
  _aux->prepare(tid);
  _assembly[tid]->prepareBlock(ivar, jvar, dof_indices);
  if (_has_nonlocal_coupling)
    if (_nonlocal_cm(ivar, jvar) != 0)
    {
      MooseVariable & jv = _nl->getVariable(tid, jvar);
      _assembly[tid]->prepareBlockNonlocal(ivar, jvar, dof_indices, jv.allDofIndices());
    }

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
  {
    _displaced_problem->prepare(_displaced_mesh->elemPtr(elem->id()), ivar, jvar, dof_indices, tid);
    if (_has_nonlocal_coupling)
      if (_nonlocal_cm(ivar, jvar) != 0)
      {
        MooseVariable & jv = _nl->getVariable(tid, jvar);
        _displaced_problem->prepareBlockNonlocal(ivar, jvar, dof_indices, jv.allDofIndices(), tid);
      }
  }
}

void
FEProblemBase::setCurrentSubdomainID(const Elem * elem, THREAD_ID tid)
{
  SubdomainID did = elem->subdomain_id();
  _assembly[tid]->setCurrentSubdomainID(did);
  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->assembly(tid).setCurrentSubdomainID(did);
}

void
FEProblemBase::setNeighborSubdomainID(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  SubdomainID did = elem->neighbor_ptr(side)->subdomain_id();
  _assembly[tid]->setCurrentNeighborSubdomainID(did);
  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->assembly(tid).setCurrentNeighborSubdomainID(did);
}

void
FEProblemBase::setNeighborSubdomainID(const Elem * elem, THREAD_ID tid)
{
  SubdomainID did = elem->subdomain_id();
  _assembly[tid]->setCurrentNeighborSubdomainID(did);
  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
    _displaced_problem->assembly(tid).setCurrentNeighborSubdomainID(did);
}

void
FEProblemBase::prepareAssembly(THREAD_ID tid)
{
  _assembly[tid]->prepare();
  if (_has_nonlocal_coupling)
    _assembly[tid]->prepareNonlocal();

  if (_displaced_problem != NULL && (_reinit_displaced_elem || _reinit_displaced_face))
  {
    _displaced_problem->prepareAssembly(tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }
}

NumericVector<Number> &
FEProblemBase::residualVector(Moose::KernelType type)
{
  return _nl->residualVector(type);
}

void
FEProblemBase::addResidual(THREAD_ID tid)
{
  if (_nl->hasResidualVector(Moose::KT_TIME))
    _assembly[tid]->addResidual(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  if (_nl->hasResidualVector(Moose::KT_NONTIME))
    _assembly[tid]->addResidual(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);

  if (_displaced_problem)
    _displaced_problem->addResidual(tid);
}

void
FEProblemBase::addResidualNeighbor(THREAD_ID tid)
{
  if (_nl->hasResidualVector(Moose::KT_TIME))
    _assembly[tid]->addResidualNeighbor(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  if (_nl->hasResidualVector(Moose::KT_NONTIME))
    _assembly[tid]->addResidualNeighbor(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
  if (_displaced_problem)
    _displaced_problem->addResidualNeighbor(tid);
}

void
FEProblemBase::addResidualScalar(THREAD_ID tid /* = 0*/)
{
  if (_nl->hasResidualVector(Moose::KT_TIME))
    _assembly[tid]->addResidualScalar(residualVector(Moose::KT_TIME), Moose::KT_TIME);
  if (_nl->hasResidualVector(Moose::KT_NONTIME))
    _assembly[tid]->addResidualScalar(residualVector(Moose::KT_NONTIME), Moose::KT_NONTIME);
}

void
FEProblemBase::cacheResidual(THREAD_ID tid)
{
  _assembly[tid]->cacheResidual();
  if (_displaced_problem)
    _displaced_problem->cacheResidual(tid);
}

void
FEProblemBase::cacheResidualNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheResidualNeighbor();
  if (_displaced_problem)
    _displaced_problem->cacheResidualNeighbor(tid);
}

void
FEProblemBase::addCachedResidual(THREAD_ID tid)
{
  _assembly[tid]->addCachedResiduals();

  if (_displaced_problem)
    _displaced_problem->addCachedResidual(tid);
}

void
FEProblemBase::addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid)
{
  if (_nl->hasResidualVector(Moose::KT_TIME))
    _assembly[tid]->addCachedResidual(residual, Moose::KT_TIME);
  if (_nl->hasResidualVector(Moose::KT_NONTIME))
    _assembly[tid]->addCachedResidual(residual, Moose::KT_NONTIME);

  if (_displaced_problem)
    _displaced_problem->addCachedResidualDirectly(residual, tid);
}

void
FEProblemBase::setResidual(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidual(residual);
  if (_displaced_problem)
    _displaced_problem->setResidual(residual, tid);
}

void
FEProblemBase::setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid)
{
  _assembly[tid]->setResidualNeighbor(residual);
  if (_displaced_problem)
    _displaced_problem->setResidualNeighbor(residual, tid);
}

void
FEProblemBase::addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobian(jacobian);
  if (_has_nonlocal_coupling)
    _assembly[tid]->addJacobianNonlocal(jacobian);
  if (_displaced_problem)
  {
    _displaced_problem->addJacobian(jacobian, tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->addJacobianNonlocal(jacobian, tid);
  }
}

void
FEProblemBase::addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(jacobian);
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighbor(jacobian, tid);
}

void
FEProblemBase::addJacobianScalar(SparseMatrix<Number> & jacobian, THREAD_ID tid /* = 0*/)
{
  _assembly[tid]->addJacobianScalar(jacobian);
}

void
FEProblemBase::addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian,
                                        unsigned int ivar,
                                        THREAD_ID tid /* = 0*/)
{
  _assembly[tid]->addJacobianOffDiagScalar(jacobian, ivar);
}

void
FEProblemBase::cacheJacobian(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobian();
  if (_has_nonlocal_coupling)
    _assembly[tid]->cacheJacobianNonlocal();
  if (_displaced_problem)
  {
    _displaced_problem->cacheJacobian(tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->cacheJacobianNonlocal(tid);
  }
}

void
FEProblemBase::cacheJacobianNeighbor(THREAD_ID tid)
{
  _assembly[tid]->cacheJacobianNeighbor();
  if (_displaced_problem)
    _displaced_problem->cacheJacobianNeighbor(tid);
}

void
FEProblemBase::addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid)
{
  _assembly[tid]->addCachedJacobian(jacobian);
  if (_displaced_problem)
    _displaced_problem->addCachedJacobian(jacobian, tid);
}

void
FEProblemBase::addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid)
{
  _assembly[tid]->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices);
  if (_has_nonlocal_coupling)
    if (_nonlocal_cm(ivar, jvar) != 0)
    {
      MooseVariable & jv = _nl->getVariable(tid, jvar);
      _assembly[tid]->addJacobianBlockNonlocal(
          jacobian, ivar, jvar, dof_map, dof_indices, jv.allDofIndices());
    }

  if (_displaced_problem)
  {
    _displaced_problem->addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, tid);
    if (_has_nonlocal_coupling)
      if (_nonlocal_cm(ivar, jvar) != 0)
      {
        MooseVariable & jv = _nl->getVariable(tid, jvar);
        _displaced_problem->addJacobianBlockNonlocal(
            jacobian, ivar, jvar, dof_map, dof_indices, jv.allDofIndices(), tid);
      }
  }
}

void
FEProblemBase::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   THREAD_ID tid)
{
  _assembly[tid]->addJacobianNeighbor(
      jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices);
  if (_displaced_problem)
    _displaced_problem->addJacobianNeighbor(
        jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices, tid);
}

void
FEProblemBase::prepareShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyShapes(var);
}

void
FEProblemBase::prepareFaceShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyFaceShapes(var);
}

void
FEProblemBase::prepareNeighborShapes(unsigned int var, THREAD_ID tid)
{
  _assembly[tid]->copyNeighborShapes(var);
}

void
FEProblemBase::addGhostedElem(dof_id_type elem_id)
{
  if (_mesh.elemPtr(elem_id)->processor_id() != processor_id())
    _ghosted_elems.insert(elem_id);
}

void
FEProblemBase::addGhostedBoundary(BoundaryID boundary_id)
{
  _mesh.addGhostedBoundary(boundary_id);
  if (_displaced_problem)
    _displaced_mesh->addGhostedBoundary(boundary_id);
}

void
FEProblemBase::ghostGhostedBoundaries()
{
  _mesh.ghostGhostedBoundaries();

  if (_displaced_problem)
    _displaced_mesh->ghostGhostedBoundaries();
}

void
FEProblemBase::sizeZeroes(unsigned int /*size*/, THREAD_ID /*tid*/)
{
  mooseDoOnce(mooseWarning(
      "This function is deprecated and no longer performs any function. Please do not call it."));
}

bool
FEProblemBase::reinitDirac(const Elem * elem, THREAD_ID tid)
{
  std::vector<Point> & points = _dirac_kernel_info.getPoints()[elem].first;

  unsigned int n_points = points.size();

  if (n_points)
  {
    if (n_points > _max_qps)
    {
      _max_qps = n_points;

      /**
       * The maximum number of qps can rise if several Dirac points are added to a single element.
       * In that case we need to resize the zeros to compensate.
       */
      for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
      {
        _zero[tid].resize(getMaxQps(), 0);
        _grad_zero[tid].resize(getMaxQps(), RealGradient(0.));
        _second_zero[tid].resize(getMaxQps(), RealTensor(0.));
        _second_phi_zero[tid].resize(
            getMaxQps(), std::vector<RealTensor>(getMaxShapeFunctions(), RealTensor(0.)));
      }
    }

    _assembly[tid]->reinitAtPhysical(elem, points);

    _nl->prepare(tid);
    _aux->prepare(tid);

    reinitElem(elem, tid);
  }

  _assembly[tid]->prepare();
  if (_has_nonlocal_coupling)
    _assembly[tid]->prepareNonlocal();

  bool have_points = n_points > 0;
  if (_displaced_problem != NULL && (_reinit_displaced_elem))
  {
    have_points |= _displaced_problem->reinitDirac(_displaced_mesh->elemPtr(elem->id()), tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }

  return have_points;
}

void
FEProblemBase::reinitElem(const Elem * elem, THREAD_ID tid)
{
  _nl->reinitElem(elem, tid);
  _aux->reinitElem(elem, tid);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitElem(_displaced_mesh->elemPtr(elem->id()), tid);
}

void
FEProblemBase::reinitElemPhys(const Elem * elem,
                              std::vector<Point> phys_points_in_elem,
                              THREAD_ID tid)
{
  _assembly[tid]->reinitAtPhysical(elem, phys_points_in_elem);

  _nl->prepare(tid);
  _aux->prepare(tid);

  reinitElem(elem, tid);
  _assembly[tid]->prepare();
  if (_has_nonlocal_coupling)
    _assembly[tid]->prepareNonlocal();

  if (_displaced_problem != NULL && (_reinit_displaced_elem))
  {
    _displaced_problem->reinitElemPhys(
        _displaced_mesh->elemPtr(elem->id()), phys_points_in_elem, tid);
    if (_has_nonlocal_coupling)
      _displaced_problem->prepareNonlocal(tid);
  }
}

void
FEProblemBase::reinitElemFace(const Elem * elem,
                              unsigned int side,
                              BoundaryID bnd_id,
                              THREAD_ID tid)
{
  _assembly[tid]->reinit(elem, side);

  _nl->reinitElemFace(elem, side, bnd_id, tid);
  _aux->reinitElemFace(elem, side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitElemFace(_displaced_mesh->elemPtr(elem->id()), side, bnd_id, tid);
}

void
FEProblemBase::reinitNode(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNode(&_displaced_mesh->nodeRef(node->id()), tid);

  _nl->reinitNode(node, tid);
  _aux->reinitNode(node, tid);
}

void
FEProblemBase::reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid)
{
  _assembly[tid]->reinit(node);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitNodeFace(&_displaced_mesh->nodeRef(node->id()), bnd_id, tid);

  _nl->reinitNodeFace(node, bnd_id, tid);
  _aux->reinitNodeFace(node, bnd_id, tid);
}

void
FEProblemBase::reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodes(nodes, tid);

  _nl->reinitNodes(nodes, tid);
  _aux->reinitNodes(nodes, tid);
}

void
FEProblemBase::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid)
{
  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodesNeighbor(nodes, tid);

  _nl->reinitNodesNeighbor(nodes, tid);
  _aux->reinitNodesNeighbor(nodes, tid);
}

void
FEProblemBase::reinitNodeNeighbor(const Node * node, THREAD_ID tid)
{
  _assembly[tid]->reinitNodeNeighbor(node);

  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitNodeNeighbor(&_displaced_mesh->nodeRef(node->id()), tid);

  _nl->reinitNodeNeighbor(node, tid);
  _aux->reinitNodeNeighbor(node, tid);
}

void
FEProblemBase::reinitScalars(THREAD_ID tid)
{
  if (_displaced_problem != NULL && _reinit_displaced_elem)
    _displaced_problem->reinitScalars(tid);

  _nl->reinitScalars(tid);
  _aux->reinitScalars(tid);

  _assembly[tid]->prepareScalar();
}

void
FEProblemBase::reinitOffDiagScalars(THREAD_ID tid)
{
  _assembly[tid]->prepareOffDiagScalar();
  if (_displaced_problem != NULL)
    _displaced_problem->reinitOffDiagScalars(tid);
}

void
FEProblemBase::reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  const Elem * neighbor = elem->neighbor_ptr(side);
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);

  _assembly[tid]->reinitElemAndNeighbor(elem, side, neighbor, neighbor_side);

  _nl->prepareNeighbor(tid);
  _aux->prepareNeighbor(tid);

  _assembly[tid]->prepareNeighbor();

  BoundaryID bnd_id = 0; // some dummy number (it is not really used for anything, right now)
  _nl->reinitElemFace(elem, side, bnd_id, tid);
  _aux->reinitElemFace(elem, side, bnd_id, tid);

  _nl->reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);
  _aux->reinitNeighborFace(neighbor, neighbor_side, bnd_id, tid);

  if (_displaced_problem != NULL && _reinit_displaced_face)
    _displaced_problem->reinitNeighbor(elem, side, tid);
}

void
FEProblemBase::reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid)
{
  // Reinits shape the functions at the physical points
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, neighbor_side, physical_points);

  // Sets the neighbor dof indices
  _nl->prepareNeighbor(tid);
  _aux->prepareNeighbor(tid);

  // Resizes Re and Ke
  _assembly[tid]->prepareNeighbor();

  // Compute the values of each variable at the points
  _nl->reinitNeighborFace(neighbor, neighbor_side, 0, tid);
  _aux->reinitNeighborFace(neighbor, neighbor_side, 0, tid);

  // Do the same for the displaced problem
  if (_displaced_problem != NULL)
    _displaced_problem->reinitNeighborPhys(
        _displaced_mesh->elemPtr(neighbor->id()), neighbor_side, physical_points, tid);
}

void
FEProblemBase::reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid)
{
  // Reinits shape the functions at the physical points
  _assembly[tid]->reinitNeighborAtPhysical(neighbor, physical_points);

  // Sets the neighbor dof indices
  _nl->prepareNeighbor(tid);
  _aux->prepareNeighbor(tid);

  // Resizes Re and Ke
  _assembly[tid]->prepareNeighbor();

  // Compute the values of each variable at the points
  _nl->reinitNeighbor(neighbor, tid);
  _aux->reinitNeighbor(neighbor, tid);

  // Do the same for the displaced problem
  if (_displaced_problem != NULL)
    _displaced_problem->reinitNeighborPhys(
        _displaced_mesh->elemPtr(neighbor->id()), physical_points, tid);
}

void
FEProblemBase::getDiracElements(std::set<const Elem *> & elems)
{
  // First add in the undisplaced elements
  elems = _dirac_kernel_info.getElements();

  if (_displaced_problem)
  {
    std::set<const Elem *> displaced_elements;
    _displaced_problem->getDiracElements(displaced_elements);

    { // Use the ids from the displaced elements to get the undisplaced elements
      // and add them to the list
      for (const auto & elem : displaced_elements)
        elems.insert(_mesh.elemPtr(elem->id()));
    }
  }
}

void
FEProblemBase::clearDiracInfo()
{
  _dirac_kernel_info.clearPoints();

  if (_displaced_problem)
    _displaced_problem->clearDiracInfo();
}

void
FEProblemBase::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  _all_materials.subdomainSetup(subdomain, tid);

  // Call the subdomain methods of the output system, these are not threaded so only call it once
  if (tid == 0)
    _app.getOutputWarehouse().subdomainSetup();

  _nl->subdomainSetup(subdomain, tid);

  // FIXME: call displaced_problem->subdomainSetup() ?
  //        When adding possibility with materials being evaluated on displaced mesh
}

void
FEProblemBase::neighborSubdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  _all_materials.neighborSubdomainSetup(subdomain, tid);
}

void
FEProblemBase::addFunction(std::string type, const std::string & name, InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  parameters.set<SubProblem *>("_subproblem") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Function> func = _factory.create<Function>(type, name, parameters, tid);
    _functions.addObject(func, tid);
  }
}

bool
FEProblemBase::hasFunction(const std::string & name, THREAD_ID tid)
{
  return _functions.hasActiveObject(name, tid);
}

Function &
FEProblemBase::getFunction(const std::string & name, THREAD_ID tid)
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
      std::string vars = "x,y,z,t,NaN,pi,e";
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

  return *(_functions.getActiveObject(name, tid));
}

NonlinearSystem &
FEProblemBase::getNonlinearSystem()
{
  mooseDeprecated("FEProblemBase::getNonlinearSystem() is deprecated, please use "
                  "FEProblemBase::getNonlinearSystemBase() \n");

  auto nl_sys = std::dynamic_pointer_cast<NonlinearSystem>(_nl);

  if (!nl_sys)
    mooseError("This is not a NonlinearSystem");

  return *nl_sys;
}

void
FEProblemBase::addDistribution(std::string type,
                               const std::string & name,
                               InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  parameters.set<std::string>("type") = type;
  std::shared_ptr<Distribution> dist = _factory.create<Distribution>(type, name, parameters);
  _distributions.addObject(dist);
}

Distribution &
FEProblemBase::getDistribution(const std::string & name)
{
  if (!_distributions.hasActiveObject(name))
    mooseError("Unable to find distribution " + name);

  return *(_distributions.getActiveObject(name));
}

void
FEProblemBase::addSampler(std::string type, const std::string & name, InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Sampler> dist = _factory.create<Sampler>(type, name, parameters, tid);
    _samplers.addObject(dist, tid);
  }
}

Sampler &
FEProblemBase::getSampler(const std::string & name, THREAD_ID tid)
{
  if (!_samplers.hasActiveObject(name, tid))
    mooseError("Unable to find Sampler " + name);

  return *(_samplers.getActiveObject(name, tid));
}

bool
FEProblemBase::duplicateVariableCheck(const std::string & var_name,
                                      const FEType & type,
                                      bool is_aux)
{
  SystemBase * curr_sys_ptr = _nl.get();
  SystemBase * other_sys_ptr = _aux.get();
  std::string error_prefix = "";
  if (is_aux)
  {
    curr_sys_ptr = _aux.get();
    other_sys_ptr = _nl.get();
    error_prefix = "Aux";
  }

  if (other_sys_ptr->hasVariable(var_name))
    mooseError("Cannot have an auxiliary variable and a nonlinear variable with the same name: ",
               var_name);

  if (curr_sys_ptr->hasVariable(var_name))
  {
    const Variable & var =
        curr_sys_ptr->system().variable(curr_sys_ptr->system().variable_number(var_name));
    if (var.type() != type)
      mooseError(error_prefix,
                 "Variable with name '",
                 var_name,
                 "' already exists but is of a differing type!");

    return true;
  }

  return false;
}

void
FEProblemBase::addVariable(const std::string & var_name,
                           const FEType & type,
                           Real scale_factor,
                           const std::set<SubdomainID> * const active_subdomains)
{
  if (duplicateVariableCheck(var_name, type, /* is_aux = */ false))
    return;

  _nl->addVariable(var_name, type, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addVariable(var_name, type, scale_factor, active_subdomains);
}

void
FEProblemBase::addScalarVariable(const std::string & var_name,
                                 Order order,
                                 Real scale_factor,
                                 const std::set<SubdomainID> * const active_subdomains)
{
  if (order > _max_scalar_order)
    _max_scalar_order = order;

  FEType type(order, SCALAR);
  if (duplicateVariableCheck(var_name, type, /* is_aux = */ false))
    return;

  _nl->addScalarVariable(var_name, order, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
FEProblemBase::addKernel(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Kernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Kernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }

  _nl->addKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addNodalKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow NodalKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this NodalKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addNodalKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addScalarKernel(const std::string & kernel_name,
                               const std::string & name,
                               InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow ScalarKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this ScalarKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addScalarKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addBoundaryCondition(const std::string & bc_name,
                                    const std::string & name,
                                    InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Materials to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Material.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addBoundaryCondition(bc_name, name, parameters);
}

void
FEProblemBase::addConstraint(const std::string & c_name,
                             const std::string & name,
                             InputParameters parameters)
{
  _has_constraints = true;

  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    // It might _want_ to use a displaced mesh... but we're not so set it to false
    if (parameters.have_parameter<bool>("use_displaced_mesh"))
      parameters.set<bool>("use_displaced_mesh") = false;

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addConstraint(c_name, name, parameters);
}

void
FEProblemBase::addAuxVariable(const std::string & var_name,
                              const FEType & type,
                              const std::set<SubdomainID> * const active_subdomains)
{
  if (duplicateVariableCheck(var_name, type, /* is_aux = */ true))
    return;

  _aux->addVariable(var_name, type, 1.0, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxVariable(var_name, type, active_subdomains);
}

void
FEProblemBase::addAuxScalarVariable(const std::string & var_name,
                                    Order order,
                                    Real scale_factor,
                                    const std::set<SubdomainID> * const active_subdomains)
{
  if (order > _max_scalar_order)
    _max_scalar_order = order;

  FEType type(order, SCALAR);
  if (duplicateVariableCheck(var_name, type, /* is_aux = */ true))
    return;

  _aux->addScalarVariable(var_name, order, scale_factor, active_subdomains);
  if (_displaced_problem)
    _displaced_problem->addAuxScalarVariable(var_name, order, scale_factor, active_subdomains);
}

void
FEProblemBase::addAuxKernel(const std::string & kernel_name,
                            const std::string & name,
                            InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    parameters.set<SystemBase *>("_nl_sys") = &_displaced_problem->nlSys();
    if (!parameters.get<std::vector<BoundaryName>>("boundary").empty())
      _reinit_displaced_face = true;
    else
      _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow AuxKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this AuxKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
    parameters.set<SystemBase *>("_nl_sys") = _nl.get();
  }
  _aux->addKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addAuxScalarKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow AuxScalarKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this AuxScalarKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }
  _aux->addScalarKernel(kernel_name, name, parameters);
}

void
FEProblemBase::addDiracKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow DiracKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this DiracKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addDiracKernel(kernel_name, name, parameters);
}

// DGKernels ////

void
FEProblemBase::addDGKernel(const std::string & dg_kernel_name,
                           const std::string & name,
                           InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow DGKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this DGKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addDGKernel(dg_kernel_name, name, parameters);
}

// InterfaceKernels ////

void
FEProblemBase::addInterfaceKernel(const std::string & interface_kernel_name,
                                  const std::string & name,
                                  InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->nlSys();
    _reinit_displaced_face = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow InterfaceKernels to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this InterfaceKernel.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _nl.get();
  }
  _nl->addInterfaceKernel(interface_kernel_name, name, parameters);
}

void
FEProblemBase::addInitialCondition(const std::string & ic_name,
                                   const std::string & name,
                                   InputParameters parameters)
{

  // before we start to mess with the initial condition, we need to check parameters for errors.
  parameters.checkParams(name);

  setInputParametersFEProblem(parameters);
  parameters.set<SubProblem *>("_subproblem") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");
  // field IC
  if (hasVariable(var_name))
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      MooseVariable & var = getVariable(tid, var_name);
      parameters.set<SystemBase *>("_sys") = &var.sys();
      std::shared_ptr<InitialCondition> ic =
          _factory.create<InitialCondition>(ic_name, name, parameters, tid);
      _ics.addObject(ic, tid);
    }
  }

  // scalar IC
  else if (hasScalarVariable(var_name))
  {
    MooseVariableScalar & var = getScalarVariable(0, var_name);
    parameters.set<SystemBase *>("_sys") = &var.sys();
    std::shared_ptr<ScalarInitialCondition> ic =
        _factory.create<ScalarInitialCondition>(ic_name, name, parameters);
    _scalar_ics.addObject(ic);
  }

  else
    mooseError(
        "Variable '", var_name, "' requested in initial condition '", name, "' does not exist.");
}

void
FEProblemBase::projectSolution()
{
  Moose::perf_log.push("projectSolution()", "Utility");

  Moose::enableFPE();

  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  ComputeInitialConditionThread cic(*this);
  Threads::parallel_reduce(elem_range, cic);

  // Need to close the solution vector here so that boundary ICs take precendence
  _nl->solution().close();
  _aux->solution().close();

  // now run boundary-restricted initial conditions
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  ComputeBoundaryInitialConditionThread cbic(*this);
  Threads::parallel_reduce(bnd_nodes, cbic);

  _nl->solution().close();
  _aux->solution().close();

  // Also, load values into the SCALAR dofs
  // Note: We assume that all SCALAR dofs are on the
  // processor with highest ID
  if (processor_id() == (n_processors() - 1) && _scalar_ics.hasActiveObjects())
  {
    const auto & ics = _scalar_ics.getActiveObjects();
    for (const auto & ic : ics)
    {
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

  _nl->solution().close();
  _nl->solution().localize(*_nl->system().current_local_solution, _nl->dofMap().get_send_list());

  _aux->solution().close();
  _aux->solution().localize(*_aux->sys().current_local_solution, _aux->dofMap().get_send_list());

  Moose::perf_log.pop("projectSolution()", "Utility");
}

std::shared_ptr<Material>
FEProblemBase::getMaterial(std::string name,
                           Moose::MaterialDataType type,
                           THREAD_ID tid,
                           bool no_warn)
{
  switch (type)
  {
    case Moose::NEIGHBOR_MATERIAL_DATA:
      name += "_neighbor";
      break;
    case Moose::FACE_MATERIAL_DATA:
      name += "_face";
      break;
    default:
      break;
  }

  std::shared_ptr<Material> material = _all_materials[type].getActiveObject(name, tid);
  if (!no_warn && material->getParam<bool>("compute") && type == Moose::BLOCK_MATERIAL_DATA)
    mooseWarning("You are retrieving a Material object (",
                 material->name(),
                 "), but its compute flag is set to true. This indicates that MOOSE is "
                 "computing this property which may not be desired and produce un-expected "
                 "results.");

  return material;
}

std::shared_ptr<MaterialData>
FEProblemBase::getMaterialData(Moose::MaterialDataType type, THREAD_ID tid)
{
  std::shared_ptr<MaterialData> output;
  switch (type)
  {
    case Moose::BLOCK_MATERIAL_DATA:
      output = _material_data[tid];
      break;
    case Moose::NEIGHBOR_MATERIAL_DATA:
      output = _neighbor_material_data[tid];
      break;
    case Moose::BOUNDARY_MATERIAL_DATA:
    case Moose::FACE_MATERIAL_DATA:
      output = _bnd_material_data[tid];
      break;
  }
  return output;
}

void
FEProblemBase::addMaterial(const std::string & mat_name,
                           const std::string & name,
                           InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Materials to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Material.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the general Block/Boundary Material object
    std::shared_ptr<Material> material = _factory.create<Material>(mat_name, name, parameters, tid);
    bool discrete = !material->getParam<bool>("compute");

    // If the object is boundary restricted do not create the neighbor and face objects
    if (material->boundaryRestricted())
    {
      _all_materials.addObject(material, tid);
      if (discrete)
        _discrete_materials.addObject(material, tid);
      else
        _materials.addObject(material, tid);
    }

    // Non-boundary restricted require face and neighbor objects
    else
    {
      // The name of the object being created, this is changed multiple times as objects are created
      // below
      std::string object_name;

      // Create a copy of the supplied parameters to the setting for "_material_data_type" isn't
      // used from a previous tid loop
      InputParameters current_parameters = parameters;

      // face material
      current_parameters.set<Moose::MaterialDataType>("_material_data_type") =
          Moose::FACE_MATERIAL_DATA;
      object_name = name + "_face";
      std::shared_ptr<Material> face_material =
          _factory.create<Material>(mat_name, object_name, current_parameters, tid);

      // neighbor material
      current_parameters.set<Moose::MaterialDataType>("_material_data_type") =
          Moose::NEIGHBOR_MATERIAL_DATA;
      current_parameters.set<bool>("_neighbor") = true;
      object_name = name + "_neighbor";
      std::shared_ptr<Material> neighbor_material =
          _factory.create<Material>(mat_name, object_name, current_parameters, tid);

      // Store the material objects
      _all_materials.addObjects(material, neighbor_material, face_material, tid);

      if (discrete)
        _discrete_materials.addObjects(material, neighbor_material, face_material, tid);
      else
        _materials.addObjects(material, neighbor_material, face_material, tid);

      // link enabled parameter of face and neighbor materials
      MooseObjectParameterName name(MooseObjectName("Material", material->name()), "enabled");
      MooseObjectParameterName face_name(MooseObjectName("Material", face_material->name()),
                                         "enabled");
      MooseObjectParameterName neighbor_name(MooseObjectName("Material", neighbor_material->name()),
                                             "enabled");
      _app.getInputParameterWarehouse().addControllableParameterConnection(name, face_name);
      _app.getInputParameterWarehouse().addControllableParameterConnection(name, neighbor_name);
    }
  }
}

void
FEProblemBase::prepareMaterials(SubdomainID blk_id, THREAD_ID tid)
{
  std::set<MooseVariable *> needed_moose_vars;
  std::set<unsigned int> needed_mat_props;

  if (_all_materials.hasActiveBlockObjects(blk_id, tid))
  {
    _all_materials.updateVariableDependency(needed_moose_vars, tid);
    _all_materials.updateBlockMatPropDependency(blk_id, needed_mat_props, tid);
  }

  const std::set<BoundaryID> & ids = _mesh.getSubdomainBoundaryIds(blk_id);
  for (const auto & id : ids)
  {
    _materials.updateBoundaryVariableDependency(id, needed_moose_vars, tid);
    _materials.updateBoundaryMatPropDependency(id, needed_mat_props, tid);
  }

  const std::set<MooseVariable *> & current_active_elemental_moose_variables =
      getActiveElementalMooseVariables(tid);
  needed_moose_vars.insert(current_active_elemental_moose_variables.begin(),
                           current_active_elemental_moose_variables.end());

  const std::set<unsigned int> & current_active_material_properties =
      getActiveMaterialProperties(tid);
  needed_mat_props.insert(current_active_material_properties.begin(),
                          current_active_material_properties.end());

  setActiveElementalMooseVariables(needed_moose_vars, tid);
  setActiveMaterialProperties(needed_mat_props, tid);
}

void
FEProblemBase::reinitMaterials(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (hasActiveMaterialProperties(tid))
  {
    const Elem *& elem = _assembly[tid]->elem();
    unsigned int n_points = _assembly[tid]->qRule()->n_points();
    _material_data[tid]->resize(n_points);

    // Only swap if requested
    if (swap_stateful)
      _material_data[tid]->swap(*elem);

    if (_discrete_materials.hasActiveBlockObjects(blk_id, tid))
      _material_data[tid]->reset(_discrete_materials.getActiveBlockObjects(blk_id, tid));

    if (_materials.hasActiveBlockObjects(blk_id, tid))
      _material_data[tid]->reinit(_materials.getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsFace(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (hasActiveMaterialProperties(tid))
  {
    const Elem *& elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();

    _bnd_material_data[tid]->resize(n_points);

    if (swap_stateful && !_bnd_material_data[tid]->isSwapped())
      _bnd_material_data[tid]->swap(*elem, side);

    if (_discrete_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _bnd_material_data[tid]->reset(
          _discrete_materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));

    if (_materials[Moose::FACE_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _bnd_material_data[tid]->reinit(
          _materials[Moose::FACE_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsNeighbor(SubdomainID blk_id, THREAD_ID tid, bool swap_stateful)
{
  if (hasActiveMaterialProperties(tid))
  {
    // NOTE: this will not work with h-adaptivity
    const Elem *& neighbor = _assembly[tid]->neighbor();
    unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();
    _neighbor_material_data[tid]->resize(n_points);

    // Only swap if requested
    if (swap_stateful)
      _neighbor_material_data[tid]->swap(*neighbor, neighbor_side);

    if (_discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _neighbor_material_data[tid]->reset(
          _discrete_materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));

    if (_materials[Moose::NEIGHBOR_MATERIAL_DATA].hasActiveBlockObjects(blk_id, tid))
      _neighbor_material_data[tid]->reinit(
          _materials[Moose::NEIGHBOR_MATERIAL_DATA].getActiveBlockObjects(blk_id, tid));
  }
}

void
FEProblemBase::reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid, bool swap_stateful)
{
  if (hasActiveMaterialProperties(tid))
  {
    const Elem *& elem = _assembly[tid]->elem();
    unsigned int side = _assembly[tid]->side();
    unsigned int n_points = _assembly[tid]->qRuleFace()->n_points();
    _bnd_material_data[tid]->resize(n_points);

    if (swap_stateful && !_bnd_material_data[tid]->isSwapped())
      _bnd_material_data[tid]->swap(*elem, side);

    if (_discrete_materials.hasActiveBoundaryObjects(boundary_id, tid))
      _bnd_material_data[tid]->reset(
          _discrete_materials.getActiveBoundaryObjects(boundary_id, tid));

    if (_materials.hasActiveBoundaryObjects(boundary_id, tid))
      _bnd_material_data[tid]->reinit(_materials.getActiveBoundaryObjects(boundary_id, tid));
  }
}

void
FEProblemBase::swapBackMaterials(THREAD_ID tid)
{
  const Elem *& elem = _assembly[tid]->elem();
  _material_data[tid]->swapBack(*elem);
}

void
FEProblemBase::swapBackMaterialsFace(THREAD_ID tid)
{
  const Elem *& elem = _assembly[tid]->elem();
  unsigned int side = _assembly[tid]->side();
  _bnd_material_data[tid]->swapBack(*elem, side);
}

void
FEProblemBase::swapBackMaterialsNeighbor(THREAD_ID tid)
{
  // NOTE: this will not work with h-adaptivity
  const Elem *& neighbor = _assembly[tid]->neighbor();
  unsigned int neighbor_side = neighbor->which_neighbor_am_i(_assembly[tid]->elem());
  _neighbor_material_data[tid]->swapBack(*neighbor, neighbor_side);
}

/**
 * Small helper function used by addPostprocessor to try to get a Postprocessor pointer from a
 * MooseObject
 */
std::shared_ptr<Postprocessor>
getPostprocessorPointer(std::shared_ptr<MooseObject> mo)
{
  {
    std::shared_ptr<ElementPostprocessor> intermediate =
        std::dynamic_pointer_cast<ElementPostprocessor>(mo);
    if (intermediate.get())
      return std::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    std::shared_ptr<NodalPostprocessor> intermediate =
        std::dynamic_pointer_cast<NodalPostprocessor>(mo);
    if (intermediate.get())
      return std::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    std::shared_ptr<InternalSidePostprocessor> intermediate =
        std::dynamic_pointer_cast<InternalSidePostprocessor>(mo);
    if (intermediate.get())
      return std::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    std::shared_ptr<SidePostprocessor> intermediate =
        std::dynamic_pointer_cast<SidePostprocessor>(mo);
    if (intermediate.get())
      return std::static_pointer_cast<Postprocessor>(intermediate);
  }

  {
    std::shared_ptr<GeneralPostprocessor> intermediate =
        std::dynamic_pointer_cast<GeneralPostprocessor>(mo);
    if (intermediate.get())
      return std::static_pointer_cast<Postprocessor>(intermediate);
  }

  return std::shared_ptr<Postprocessor>();
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
FEProblemBase::initPostprocessorData(const std::string & name)
{
  _pps_data.init(name);
}

void
FEProblemBase::addPostprocessor(std::string pp_name,
                                const std::string & name,
                                InputParameters parameters)
{
  // Check for name collision
  if (_all_user_objects.hasActiveObject(name))
    mooseError(std::string("A UserObject with the name \"") + name +
               "\" already exists.  You may not add a Postprocessor by the same name.");

  addUserObject(pp_name, name, parameters);
  initPostprocessorData(name);
}

void
FEProblemBase::addVectorPostprocessor(std::string pp_name,
                                      const std::string & name,
                                      InputParameters parameters)
{
  // Check for name collision
  if (_all_user_objects.hasActiveObject(name))
    mooseError(std::string("A UserObject with the name \"") + name +
               "\" already exists.  You may not add a VectorPostprocessor by the same name.");

  addUserObject(pp_name, name, parameters);
}

void
FEProblemBase::addUserObject(std::string user_object_name,
                             const std::string & name,
                             InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow UserObjects to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this UserObject.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    // Create the UserObject
    std::shared_ptr<UserObject> user_object =
        _factory.create<UserObject>(user_object_name, name, parameters, tid);
    _all_user_objects.addObject(user_object, tid);

    // Attempt to create all the possible UserObject types
    std::shared_ptr<ElementUserObject> euo =
        std::dynamic_pointer_cast<ElementUserObject>(user_object);
    std::shared_ptr<SideUserObject> suo = std::dynamic_pointer_cast<SideUserObject>(user_object);
    std::shared_ptr<InternalSideUserObject> isuo =
        std::dynamic_pointer_cast<InternalSideUserObject>(user_object);
    std::shared_ptr<NodalUserObject> nuo = std::dynamic_pointer_cast<NodalUserObject>(user_object);
    std::shared_ptr<GeneralUserObject> guo =
        std::dynamic_pointer_cast<GeneralUserObject>(user_object);

    // Account for displaced mesh use
    if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      if (euo || nuo)
        _reinit_displaced_elem = true;
      else if (suo)
        _reinit_displaced_face = true;
    }

    // Add the object to the correct warehouse
    if (guo)
    {
      _general_user_objects.addObject(guo);
      break; // not threaded
    }
    else if (nuo)
      _nodal_user_objects.addObject(nuo, tid);
    else if (suo)
      _side_user_objects.addObject(suo, tid);
    else if (isuo)
      _internal_side_user_objects.addObject(isuo, tid);
    else if (euo)
      _elemental_user_objects.addObject(euo, tid);
  }
}

const UserObject &
FEProblemBase::getUserObjectBase(const std::string & name)
{
  if (_all_user_objects.hasActiveObject(name))
    return *(_all_user_objects.getActiveObject(name).get());

  mooseError("Unable to find user object with name '" + name + "'");
}

bool
FEProblemBase::hasUserObject(const std::string & name)
{
  return _all_user_objects.hasActiveObject(name);
}

bool
FEProblemBase::hasPostprocessor(const std::string & name)
{
  return _pps_data.hasPostprocessor(name);
}

PostprocessorValue &
FEProblemBase::getPostprocessorValue(const PostprocessorName & name)
{
  return _pps_data.getPostprocessorValue(name);
}

PostprocessorValue &
FEProblemBase::getPostprocessorValueOld(const std::string & name)
{
  return _pps_data.getPostprocessorValueOld(name);
}

PostprocessorValue &
FEProblemBase::getPostprocessorValueOlder(const std::string & name)
{
  return _pps_data.getPostprocessorValueOlder(name);
}

VectorPostprocessorData &
FEProblemBase::getVectorPostprocessorData()
{
  return _vpps_data;
}

bool
FEProblemBase::hasVectorPostprocessor(const std::string & name)
{
  return _vpps_data.hasVectorPostprocessor(name);
}

VectorPostprocessorValue &
FEProblemBase::getVectorPostprocessorValue(const VectorPostprocessorName & name,
                                           const std::string & vector_name)
{
  return _vpps_data.getVectorPostprocessorValue(name, vector_name);
}

VectorPostprocessorValue &
FEProblemBase::getVectorPostprocessorValueOld(const std::string & name,
                                              const std::string & vector_name)
{
  return _vpps_data.getVectorPostprocessorValueOld(name, vector_name);
}

VectorPostprocessorValue &
FEProblemBase::declareVectorPostprocessorVector(const VectorPostprocessorName & name,
                                                const std::string & vector_name)
{
  return _vpps_data.declareVector(name, vector_name);
}

const std::vector<std::pair<std::string, VectorPostprocessorData::VectorPostprocessorState>> &
FEProblemBase::getVectorPostprocessorVectors(const std::string & vpp_name)
{
  return _vpps_data.vectors(vpp_name);
}

void
FEProblemBase::parentOutputPositionChanged()
{
  for (const auto & it : _multi_apps)
  {
    const auto & objects = it.second.getActiveObjects();
    for (const auto & obj : objects)
      obj->parentOutputPositionChanged();
  }
}

void
FEProblemBase::computeIndicatorsAndMarkers()
{
  computeIndicators();
  computeMarkers();
}

void
FEProblemBase::computeIndicators()
{
  // Initialize indicator aux variable fields
  if (_indicators.hasActiveObjects() || _internal_side_indicators.hasActiveObjects())
  {
    Moose::perf_log.push("Adaptivity: computeIndicators()", "Execution");

    std::vector<std::string> fields;

    // Indicator Fields
    const auto & indicators = _indicators.getActiveObjects();
    for (const auto & indicator : indicators)
      fields.push_back(indicator->name());

    // InternalSideIndicator Fields
    const auto & internal_indicators = _internal_side_indicators.getActiveObjects();
    for (const auto & internal_indicator : internal_indicators)
      fields.push_back(internal_indicator->name());

    _aux->zeroVariables(fields);

    // compute Indicators
    ComputeIndicatorThread cit(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cit);
    _aux->solution().close();
    _aux->update();

    ComputeIndicatorThread finalize_cit(*this, true);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), finalize_cit);
    _aux->solution().close();
    _aux->update();

    Moose::perf_log.pop("Adaptivity: computeIndicators()", "Execution");
  }
}

void
FEProblemBase::computeMarkers()
{
  if (_markers.hasActiveObjects())
  {
    Moose::perf_log.push("Adaptivity: computeMarkers()", "Execution");

    std::vector<std::string> fields;

    // Marker Fields
    const auto & markers = _markers.getActiveObjects();
    for (const auto & marker : markers)
      fields.push_back(marker->name());

    _aux->zeroVariables(fields);

    _adaptivity.updateErrorVectors();

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      const auto & markers = _markers.getActiveObjects(tid);
      for (const auto & marker : markers)
        marker->markerSetup();
    }

    ComputeMarkerThread cmt(*this);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cmt);

    _aux->solution().close();
    _aux->update();

    Moose::perf_log.pop("Adaptivity: computeMarkers()", "Execution");
  }
}

const ExecFlagType &
FEProblemBase::getCurrentExecuteOnFlag() const
{
  return _current_execute_on_flag;
}

void
FEProblemBase::execute(const ExecFlagType & exec_type)
{
  // Set the current flag
  _current_execute_on_flag = exec_type;
  if (exec_type == EXEC_NONLINEAR)
    _currently_computing_jacobian = true;

  // Samplers
  executeSamplers(exec_type);

  // Pre-aux UserObjects
  computeUserObjects(exec_type, Moose::PRE_AUX);

  // AuxKernels
  computeAuxiliaryKernels(exec_type);

  // Post-aux UserObjects
  computeUserObjects(exec_type, Moose::POST_AUX);

  // Controls
  executeControls(exec_type);

  // Return the current flag to None
  _current_execute_on_flag = EXEC_NONE;
  _currently_computing_jacobian = false;
}

void
FEProblemBase::computeAuxiliaryKernels(const ExecFlagType & type)
{
  _aux->compute(type);
}

void
FEProblemBase::computeUserObjects(const ExecFlagType & type, const Moose::AuxGroup & group)
{
  // Get convenience reference to active warehouse
  const MooseObjectWarehouse<ElementUserObject> & elemental = _elemental_user_objects[group][type];
  const MooseObjectWarehouse<SideUserObject> & side = _side_user_objects[group][type];
  const MooseObjectWarehouse<InternalSideUserObject> & internal_side =
      _internal_side_user_objects[group][type];
  const MooseObjectWarehouse<NodalUserObject> & nodal = _nodal_user_objects[group][type];
  const MooseObjectWarehouse<GeneralUserObject> & general = _general_user_objects[group][type];

  if (!elemental.hasActiveObjects() && !side.hasActiveObjects() &&
      !internal_side.hasActiveObjects() && !nodal.hasActiveObjects() && !general.hasActiveObjects())
    // Nothing to do, return early
    return;

  // Start the timer here since we have at least one active user object
  std::string compute_uo_tag = "computeUserObjects(" + Moose::stringify(type) + ")";
  Moose::perf_log.push(compute_uo_tag, "Execution");

  // Perform Residual/Jacobian setups
  switch (type)
  {
    case EXEC_LINEAR:
      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      {
        elemental.residualSetup(tid);
        side.residualSetup(tid);
        internal_side.residualSetup(tid);
        nodal.residualSetup(tid);
      }
      general.residualSetup();
      break;

    case EXEC_NONLINEAR:
      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
      {
        elemental.jacobianSetup(tid);
        side.jacobianSetup(tid);
        internal_side.jacobianSetup(tid);
        nodal.jacobianSetup(tid);
      }
      general.jacobianSetup();
      break;

    default:
      break;
  }

  // Initialize Elemental/Side/InternalSideUserObjects
  initializeUserObjects<ElementUserObject>(elemental);
  initializeUserObjects<SideUserObject>(side);
  initializeUserObjects<InternalSideUserObject>(internal_side);

  // Execute Elemental/Side/InternalSideUserObjects
  if (elemental.hasActiveObjects() || side.hasActiveObjects() || internal_side.hasActiveObjects())
  {
    ComputeUserObjectsThread cppt(*this, getNonlinearSystemBase(), elemental, side, internal_side);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cppt);
  }

  // Finalize, threadJoin, and update PP values of Elemental/Side/InternalSideUserObjects
  finalizeUserObjects<SideUserObject>(side);
  finalizeUserObjects<InternalSideUserObject>(internal_side);
  finalizeUserObjects<ElementUserObject>(elemental);

  // Initialize Nodal
  initializeUserObjects<NodalUserObject>(nodal);

  // Execute NodalUserObjects
  if (nodal.hasActiveObjects())
  {
    ComputeNodalUserObjectsThread cnppt(*this, nodal);
    Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cnppt);
  }

  // Finalize, threadJoin, and update PP values of Nodal
  finalizeUserObjects<NodalUserObject>(nodal);

  // Execute GeneralUserObjects
  if (general.hasActiveObjects())
  {
    const auto & objects = general.getActiveObjects();
    for (const auto & obj : objects)
    {
      obj->initialize();
      obj->execute();
      obj->finalize();

      std::shared_ptr<Postprocessor> pp = std::dynamic_pointer_cast<Postprocessor>(obj);
      if (pp)
        _pps_data.storeValue(obj->name(), pp->getValue());
    }
  }

  Moose::perf_log.pop(compute_uo_tag, "Execution");
}

void
FEProblemBase::executeControls(const ExecFlagType & exec_type)
{
  DependencyResolver<std::shared_ptr<Control>> resolver;

  auto controls_wh = _control_warehouse[exec_type];
  // Add all of the dependencies into the resolver and sort them
  for (const auto & it : controls_wh.getActiveObjects())
  {
    // Make sure an item with no dependencies comes out too!
    resolver.addItem(it);

    std::vector<std::string> & dependent_controls = it->getDependencies();
    for (const auto & depend_name : dependent_controls)
    {
      if (controls_wh.hasActiveObject(depend_name))
      {
        auto dep_control = controls_wh.getActiveObject(depend_name);
        resolver.insertDependency(it, dep_control);
      }
      else
        mooseError("The Control \"",
                   depend_name,
                   "\" was not created, did you make a "
                   "spelling mistake or forget to include it "
                   "in your input file?");
    }
  }

  const auto & ordered_controls = resolver.getSortedValues();

  if (!ordered_controls.empty())
  {
    Moose::perf_log.push("computeControls()", "Execution");

    _control_warehouse.setup(exec_type);
    // Run the controls in the proper order
    for (const auto & control : ordered_controls)
      control->execute();

    Moose::perf_log.pop("computeControls()", "Execution");
  }
}

void
FEProblemBase::executeSamplers(const ExecFlagType & exec_type)
{
  // TODO: This should be done in a threaded loop, but this should be super quick so for now
  // do a serial loop.
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    const auto & objects = _samplers[exec_type].getActiveObjects(tid);
    if (!objects.empty())
    {
      Moose::perf_log.push("executeSamplers()", "Execution");
      _samplers.setup(exec_type);
      for (auto & sampler : objects)
        sampler->execute();
      Moose::perf_log.pop("executeSamplers()", "Execution");
    }
  }
}

void
FEProblemBase::updateActiveObjects()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _nl->updateActive(tid);
    _aux->updateActive(tid);
    _indicators.updateActive(tid);
    _internal_side_indicators.updateActive(tid);
    _markers.updateActive(tid);
    _all_materials.updateActive(tid);
    _materials.updateActive(tid);
    _discrete_materials.updateActive(tid);
    _nodal_user_objects.updateActive(tid);
    _elemental_user_objects.updateActive(tid);
    _side_user_objects.updateActive(tid);
    _internal_side_user_objects.updateActive(tid);
    _samplers.updateActive(tid);
  }

  _general_user_objects.updateActive();
  _control_warehouse.updateActive();
  _multi_apps.updateActive();
  _transient_multi_apps.updateActive();
  _transfers.updateActive();
  _to_multi_app_transfers.updateActive();
  _from_multi_app_transfers.updateActive();
}

void
FEProblemBase::reportMooseObjectDependency(MooseObject * /*a*/, MooseObject * /*b*/)
{
  //<< "Object " << a->name() << " -> " << b->name() << std::endl;
}

void
FEProblemBase::reinitBecauseOfGhostingOrNewGeomObjects()
{
  // Need to see if _any_ processor has ghosted elems or geometry objects.
  bool needs_reinit = !_ghosted_elems.empty();
  needs_reinit = needs_reinit || !_geometric_search_data._nearest_node_locators.empty();
  needs_reinit =
      needs_reinit ||
      (_displaced_problem && !_displaced_problem->geomSearchData()._nearest_node_locators.empty());
  _communicator.max(needs_reinit);

  if (needs_reinit)
  {
    // Call reinit to get the ghosted vectors correct now that some geometric search has been done
    _eq.reinit();

    if (_displaced_mesh)
      _displaced_problem->es().reinit();
  }
}

void
FEProblemBase::addDamper(std::string damper_name,
                         const std::string & name,
                         InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  parameters.set<SubProblem *>("_subproblem") = this;
  parameters.set<SystemBase *>("_sys") = _nl.get();

  _has_dampers = true;
  _nl->addDamper(damper_name, name, parameters);
}

void
FEProblemBase::setupDampers()
{
  _nl->setupDampers();
}

void
FEProblemBase::addIndicator(std::string indicator_name,
                            const std::string & name,
                            InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Indicators to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Indicator.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Indicator> indicator =
        _factory.create<Indicator>(indicator_name, name, parameters, tid);

    std::shared_ptr<InternalSideIndicator> isi =
        std::dynamic_pointer_cast<InternalSideIndicator>(indicator);
    if (isi)
      _internal_side_indicators.addObject(isi, tid);
    else
      _indicators.addObject(indicator, tid);
  }
}

void
FEProblemBase::addMarker(std::string marker_name,
                         const std::string & name,
                         InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Markers to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Marker.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<Marker> marker = _factory.create<Marker>(marker_name, name, parameters, tid);
    _markers.addObject(marker, tid);
  }
}

void
FEProblemBase::addMultiApp(const std::string & multi_app_name,
                           const std::string & name,
                           InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  parameters.set<MPI_Comm>("_mpi_comm") = _communicator.get();
  parameters.set<std::shared_ptr<CommandLine>>("_command_line") = _app.commandLine();

  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow MultiApps to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this MultiApp.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  std::shared_ptr<MultiApp> multi_app = _factory.create<MultiApp>(multi_app_name, name, parameters);

  _multi_apps.addObject(multi_app);

  // Store TranseintMultiApp objects in another container, this is needed for calling computeDT
  std::shared_ptr<TransientMultiApp> trans_multi_app =
      std::dynamic_pointer_cast<TransientMultiApp>(multi_app);
  if (trans_multi_app)
    _transient_multi_apps.addObject(trans_multi_app);
}

bool
FEProblemBase::hasMultiApp(const std::string & multi_app_name)
{
  return _multi_apps.hasActiveObject(multi_app_name);
}

std::shared_ptr<MultiApp>
FEProblemBase::getMultiApp(const std::string & multi_app_name)
{
  return _multi_apps.getActiveObject(multi_app_name);
}

void
FEProblemBase::execMultiAppTransfers(ExecFlagType type, MultiAppTransfer::DIRECTION direction)
{
  bool to_multiapp = direction == MultiAppTransfer::TO_MULTIAPP;
  std::string string_direction = to_multiapp ? " To " : " From ";
  const MooseObjectWarehouse<Transfer> & wh =
      to_multiapp ? _to_multi_app_transfers[type] : _from_multi_app_transfers[type];

  if (wh.hasActiveObjects())
  {
    const auto & transfers = wh.getActiveObjects();

    _console << COLOR_CYAN << "\nStarting Transfers on " << Moose::stringify(type)
             << string_direction << "MultiApps" << COLOR_DEFAULT << std::endl;
    for (const auto & transfer : transfers)
    {
      Moose::perf_log.push(transfer->name(), "Transfers");
      transfer->execute();
      Moose::perf_log.pop(transfer->name(), "Transfers");
    }

    _console << "Waiting For Transfers To Finish" << '\n';
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Transfers on " << Moose::stringify(type) << " Are Finished\n"
             << COLOR_DEFAULT << std::endl;
  }
  else if (_multi_apps[type].getActiveObjects().size())
    _console << COLOR_CYAN << "\nNo Transfers on " << Moose::stringify(type) << " To MultiApps\n"
             << COLOR_DEFAULT << std::endl;
}

std::vector<std::shared_ptr<Transfer>>
FEProblemBase::getTransfers(ExecFlagType type, MultiAppTransfer::DIRECTION direction) const
{
  const MooseObjectWarehouse<Transfer> & wh = direction == MultiAppTransfer::TO_MULTIAPP
                                                  ? _to_multi_app_transfers[type]
                                                  : _from_multi_app_transfers[type];
  return wh.getActiveObjects();
}

bool
FEProblemBase::execMultiApps(ExecFlagType type, bool auto_advance)
{
  // Active MultiApps
  const std::vector<MooseSharedPointer<MultiApp>> & multi_apps =
      _multi_apps[type].getActiveObjects();

  // Do anything that needs to be done to Apps before transfers
  for (const auto & multi_app : multi_apps)
    multi_app->preTransfer(_dt, _time);

  // Execute Transfers _to_ MultiApps
  execMultiAppTransfers(type, MultiAppTransfer::TO_MULTIAPP);

  // Execute MultiApps
  if (multi_apps.size())
  {
    _console << COLOR_CYAN << "\nExecuting MultiApps on " << Moose::stringify(type) << COLOR_DEFAULT
             << std::endl;

    bool success = true;

    for (const auto & multi_app : multi_apps)
      success = multi_app->solveStep(_dt, _time, auto_advance);

    _console << "Waiting For Other Processors To Finish" << '\n';
    MooseUtils::parallelBarrierNotify(_communicator);

    _communicator.max(success);

    if (!success)
      return false;

    _console << COLOR_CYAN << "Finished Executing MultiApps on " << Moose::stringify(type) << "\n"
             << COLOR_DEFAULT << std::endl;
  }

  // Execute Transfers _from_ MultiApps
  execMultiAppTransfers(type, MultiAppTransfer::FROM_MULTIAPP);

  // If we made it here then everything passed
  return true;
}

void
FEProblemBase::postExecute()
{
  const auto & multi_apps = _multi_apps.getActiveObjects();

  for (const auto & multi_app : multi_apps)
    // If the app has been solved, then postExecute() will have been called already too
    if (!multi_app->isSolved())
      multi_app->postExecute();
}

void
FEProblemBase::advanceMultiApps(ExecFlagType type)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    _console << COLOR_CYAN << "\nAdvancing MultiApps" << COLOR_DEFAULT << std::endl;

    for (const auto & multi_app : multi_apps)
      multi_app->advanceStep();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Finished Advancing MultiApps\n" << COLOR_DEFAULT << std::endl;
  }
}

void
FEProblemBase::backupMultiApps(ExecFlagType type)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    _console << COLOR_CYAN << "\nBacking Up MultiApps" << COLOR_DEFAULT << std::endl;

    for (const auto & multi_app : multi_apps)
      multi_app->backup();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Finished Backing Up MultiApps\n" << COLOR_DEFAULT << std::endl;
  }
}

void
FEProblemBase::restoreMultiApps(ExecFlagType type, bool force)
{
  const auto & multi_apps = _multi_apps[type].getActiveObjects();

  if (multi_apps.size())
  {
    if (force)
      _console << COLOR_CYAN << "\nRestoring Multiapps because of solve failure!" << COLOR_DEFAULT
               << std::endl;
    else
      _console << COLOR_CYAN << "\nRestoring MultiApps" << COLOR_DEFAULT << std::endl;

    for (const auto & multi_app : multi_apps)
      if (force || multi_app->needsRestoration())
        multi_app->restore();

    _console << "Waiting For Other Processors To Finish" << std::endl;
    MooseUtils::parallelBarrierNotify(_communicator);

    _console << COLOR_CYAN << "Finished Restoring MultiApps\n" << COLOR_DEFAULT << std::endl;
  }
}

Real
FEProblemBase::computeMultiAppsDT(ExecFlagType type)
{
  const auto & multi_apps = _transient_multi_apps[type].getActiveObjects();

  Real smallest_dt = std::numeric_limits<Real>::max();

  for (const auto & multi_app : multi_apps)
    smallest_dt = std::min(smallest_dt, multi_app->computeDT());

  return smallest_dt;
}

void
FEProblemBase::execTransfers(ExecFlagType type)
{
  if (_transfers[type].hasActiveObjects())
  {
    const auto & transfers = _transfers[type].getActiveObjects();
    for (const auto & transfer : transfers)
      transfer->execute();
  }
}

void
FEProblemBase::addTransfer(const std::string & transfer_name,
                           const std::string & name,
                           InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  if (_displaced_problem != NULL && parameters.get<bool>("use_displaced_mesh"))
  {
    parameters.set<SubProblem *>("_subproblem") = _displaced_problem.get();
    parameters.set<SystemBase *>("_sys") = &_displaced_problem->auxSys();
    _reinit_displaced_elem = true;
  }
  else
  {
    if (_displaced_problem == NULL && parameters.get<bool>("use_displaced_mesh"))
    {
      // We allow Transfers to request that they use_displaced_mesh,
      // but then be overridden when no displacements variables are
      // provided in the Mesh block.  If that happened, update the value
      // of use_displaced_mesh appropriately for this Transfer.
      if (parameters.have_parameter<bool>("use_displaced_mesh"))
        parameters.set<bool>("use_displaced_mesh") = false;
    }

    parameters.set<SubProblem *>("_subproblem") = this;
    parameters.set<SystemBase *>("_sys") = _aux.get();
  }

  // Create the Transfer objects
  std::shared_ptr<Transfer> transfer = _factory.create<Transfer>(transfer_name, name, parameters);

  // Add MultiAppTransfer object
  std::shared_ptr<MultiAppTransfer> multi_app_transfer =
      std::dynamic_pointer_cast<MultiAppTransfer>(transfer);
  if (multi_app_transfer)
  {
    if (multi_app_transfer->direction() == MultiAppTransfer::TO_MULTIAPP)
      _to_multi_app_transfers.addObject(multi_app_transfer);
    else
      _from_multi_app_transfers.addObject(multi_app_transfer);
  }
  else
    _transfers.addObject(transfer);
}

bool
FEProblemBase::hasVariable(const std::string & var_name)
{
  if (_nl->hasVariable(var_name))
    return true;
  else if (_aux->hasVariable(var_name))
    return true;
  else
    return false;
}

MooseVariable &
FEProblemBase::getVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl->hasVariable(var_name))
    return _nl->getVariable(tid, var_name);
  else if (!_aux->hasVariable(var_name))
    mooseError("Unknown variable " + var_name);

  return _aux->getVariable(tid, var_name);
}

bool
FEProblemBase::hasScalarVariable(const std::string & var_name)
{
  if (_nl->hasScalarVariable(var_name))
    return true;
  else if (_aux->hasScalarVariable(var_name))
    return true;
  else
    return false;
}

MooseVariableScalar &
FEProblemBase::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  if (_nl->hasScalarVariable(var_name))
    return _nl->getScalarVariable(tid, var_name);
  else if (_aux->hasScalarVariable(var_name))
    return _aux->getScalarVariable(tid, var_name);
  else
    mooseError("Unknown variable " + var_name);
}

System &
FEProblemBase::getSystem(const std::string & var_name)
{
  if (_nl->hasVariable(var_name))
    return _nl->system();
  else if (_aux->hasVariable(var_name))
    return _aux->system();
  else
    mooseError("Unable to find a system containing the variable " + var_name);
}

void
FEProblemBase::setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars,
                                                THREAD_ID tid)
{
  SubProblem::setActiveElementalMooseVariables(moose_vars, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveElementalMooseVariables(moose_vars, tid);
}

const std::set<MooseVariable *> &
FEProblemBase::getActiveElementalMooseVariables(THREAD_ID tid)
{
  return SubProblem::getActiveElementalMooseVariables(tid);
}

bool
FEProblemBase::hasActiveElementalMooseVariables(THREAD_ID tid)
{
  return SubProblem::hasActiveElementalMooseVariables(tid);
}

void
FEProblemBase::clearActiveElementalMooseVariables(THREAD_ID tid)
{
  SubProblem::clearActiveElementalMooseVariables(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveElementalMooseVariables(tid);
}

void
FEProblemBase::setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids,
                                           THREAD_ID tid)
{
  SubProblem::setActiveMaterialProperties(mat_prop_ids, tid);

  if (_displaced_problem)
    _displaced_problem->setActiveMaterialProperties(mat_prop_ids, tid);
}

const std::set<unsigned int> &
FEProblemBase::getActiveMaterialProperties(THREAD_ID tid)
{
  return SubProblem::getActiveMaterialProperties(tid);
}

bool
FEProblemBase::hasActiveMaterialProperties(THREAD_ID tid)
{
  return SubProblem::hasActiveMaterialProperties(tid);
}

void
FEProblemBase::clearActiveMaterialProperties(THREAD_ID tid)
{
  SubProblem::clearActiveMaterialProperties(tid);

  if (_displaced_problem)
    _displaced_problem->clearActiveMaterialProperties(tid);
}

void
FEProblemBase::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  if (order == INVALID_ORDER)
  {
    // automatically determine the integration order
    order = _nl->getMinQuadratureOrder();
    if (order < _aux->getMinQuadratureOrder())
      order = _aux->getMinQuadratureOrder();
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
    MaxQpsThread mqt(*this, type, std::max(order, volume_order), face_order);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), mqt);
    _max_qps = mqt.max();
    _max_shape_funcs = mqt.max_shape_funcs();

    // If we have more shape functions or more quadrature points on
    // another processor, then we may need to handle those elements
    // ourselves later after repartitioning.
    _communicator.max(_max_qps);
    _communicator.max(_max_shape_funcs);
  }

  for (unsigned int tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _zero[tid].resize(getMaxQps(), 0);
    _grad_zero[tid].resize(getMaxQps(), RealGradient(0.));
    _second_zero[tid].resize(getMaxQps(), RealTensor(0.));
    _second_phi_zero[tid].resize(getMaxQps(),
                                 std::vector<RealTensor>(getMaxShapeFunctions(), RealTensor(0.)));
  }
}

void
FEProblemBase::setCoupling(Moose::CouplingType type)
{
  _coupling = type;
}

void
FEProblemBase::setCouplingMatrix(CouplingMatrix * cm)
{
  // TODO: Deprecate method

  _coupling = Moose::COUPLING_CUSTOM;
  _cm.reset(cm);
}

void
FEProblemBase::setCouplingMatrix(std::unique_ptr<CouplingMatrix> cm)
{
  _coupling = Moose::COUPLING_CUSTOM;
  _cm = std::move(cm);
}

void
FEProblemBase::setNonlocalCouplingMatrix()
{
  unsigned int n_vars = _nl->nVariables();
  _nonlocal_cm.resize(n_vars);
  const auto & vars = _nl->getVariables(0);
  const auto & nonlocal_kernel = _nonlocal_kernels.getObjects();
  const auto & nonlocal_integrated_bc = _nonlocal_integrated_bcs.getObjects();
  for (const auto & ivar : vars)
  {
    for (const auto & kernel : nonlocal_kernel)
    {
      unsigned int i = ivar->number();
      if (i == kernel->variable().number())
        for (const auto & jvar : vars)
        {
          const auto it = _var_dof_map.find(jvar->name());
          if (it != _var_dof_map.end())
          {
            unsigned int j = jvar->number();
            _nonlocal_cm(i, j) = 1;
          }
        }
    }
    for (const auto & integrated_bc : nonlocal_integrated_bc)
    {
      unsigned int i = ivar->number();
      if (i == integrated_bc->variable().number())
        for (const auto & jvar : vars)
        {
          const auto it = _var_dof_map.find(jvar->name());
          if (it != _var_dof_map.end())
          {
            unsigned int j = jvar->number();
            _nonlocal_cm(i, j) = 1;
          }
        }
    }
  }
}

bool
FEProblemBase::areCoupled(unsigned int ivar, unsigned int jvar)
{
  return (*_cm)(ivar, jvar);
}

std::vector<std::pair<MooseVariable *, MooseVariable *>> &
FEProblemBase::couplingEntries(THREAD_ID tid)
{
  return _assembly[tid]->couplingEntries();
}

std::vector<std::pair<MooseVariable *, MooseVariable *>> &
FEProblemBase::nonlocalCouplingEntries(THREAD_ID tid)
{
  return _assembly[tid]->nonlocalCouplingEntries();
}

void
FEProblemBase::useFECache(bool fe_cache)
{
  if (fe_cache)
    _console << "\nUtilizing FE Shape Function Caching\n" << std::endl;

  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    _assembly[i]->useFECache(fe_cache); // fe_cache);
}

void
FEProblemBase::init()
{
  if (_initialized)
    return;

  unsigned int n_vars = _nl->nVariables();
  switch (_coupling)
  {
    case Moose::COUPLING_DIAG:
      _cm = libmesh_make_unique<CouplingMatrix>(n_vars);
      for (unsigned int i = 0; i < n_vars; i++)
        for (unsigned int j = 0; j < n_vars; j++)
          (*_cm)(i, j) = (i == j ? 1 : 0);
      break;

    // for full jacobian
    case Moose::COUPLING_FULL:
      _cm = libmesh_make_unique<CouplingMatrix>(n_vars);
      for (unsigned int i = 0; i < n_vars; i++)
        for (unsigned int j = 0; j < n_vars; j++)
          (*_cm)(i, j) = 1;
      break;

    case Moose::COUPLING_CUSTOM:
      // do nothing, _cm was already set through couplingMatrix() call
      break;
  }

  _nl->dofMap()._dof_coupling = _cm.get();
  _nl->dofMap().attach_extra_sparsity_function(&extraSparsity, _nl.get());
  _nl->dofMap().attach_extra_send_list_function(&extraSendList, _nl.get());
  _aux->dofMap().attach_extra_send_list_function(&extraSendList, _aux.get());

  if (_solve && n_vars == 0)
    mooseError("No variables specified in the FEProblemBase '", name(), "'.");

  ghostGhostedBoundaries(); // We do this again right here in case new boundaries have been added

  // do not assemble system matrix for JFNK solve
  if (solverParams()._type == Moose::ST_JFNK)
    _nl->turnOffJacobian();

  Moose::perf_log.push("eq.init()", "Setup");
  _eq.init();
  Moose::perf_log.pop("eq.init()", "Setup");

  Moose::perf_log.push("FEProblemBase::init::meshChanged()", "Setup");
  _mesh.meshChanged();
  if (_displaced_problem)
    _displaced_mesh->meshChanged();
  Moose::perf_log.pop("FEProblemBase::init::meshChanged()", "Setup");

  Moose::perf_log.push("NonlinearSystem::update()", "Setup");
  _nl->update();
  Moose::perf_log.pop("NonlinearSystem::update()", "Setup");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _assembly[tid]->init(_cm.get());

  _nl->init();

  if (_displaced_problem)
    _displaced_problem->init();

  _aux->init();

  _initialized = true;
}

void
FEProblemBase::solve()
{
  Moose::perf_log.push("solve()", "Execution");

#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(*this); // Make sure the PETSc options are setup for this app
#endif

  Moose::setSolverDefaults(*this);

  // Setup the output system for printing linear/nonlinear iteration information
  initPetscOutput();

  possiblyRebuildGeomSearchPatches();

  // reset flag so that linear solver does not use
  // the old converged reason "DIVERGED_NANORINF", when
  // we throw  an exception and stop solve
  _fail_next_linear_convergence_check = false;

  if (_solve)
    _nl->solve();

  if (_solve)
    _nl->update();

  // sync solutions in displaced problem
  if (_displaced_problem)
    _displaced_problem->syncSolutions();

  Moose::perf_log.pop("solve()", "Execution");
}

void
FEProblemBase::setException(const std::string & message)
{
  _has_exception = true;
  _exception_message = message;
}

void
FEProblemBase::checkExceptionAndStopSolve()
{
  // See if any processor had an exception.  If it did, get back the
  // processor that the exception occurred on.
  unsigned int processor_id;

  _communicator.maxloc(_has_exception, processor_id);

  if (_has_exception)
  {
    _communicator.broadcast(_exception_message, processor_id);

    // Print the message
    if (_communicator.rank() == 0)
      Moose::err << _exception_message << std::endl;

    // Stop the solve -- this entails setting
    // SNESSetFunctionDomainError() or directly inserting NaNs in the
    // residual vector to let PETSc >= 3.6 return DIVERGED_NANORINF.
    _nl->stopSolve();

    // We've handled this exception, so we no longer have one.
    _has_exception = false;

    // Force the next linear convergence check to fail.
    _fail_next_linear_convergence_check = true;

    // Repropagate the exception, so it can be caught at a higher level, typically
    // this is NonlinearSystem::computeResidual().
    throw MooseException(_exception_message);
  }
}

bool
FEProblemBase::converged()
{
  if (_solve)
    return _nl->converged();
  else
    return true;
}

unsigned int
FEProblemBase::nNonlinearIterations()
{
  return _nl->nNonlinearIterations();
}

unsigned int
FEProblemBase::nLinearIterations()
{
  return _nl->nLinearIterations();
}

Real
FEProblemBase::finalNonlinearResidual()
{
  return _nl->finalNonlinearResidual();
}

bool
FEProblemBase::computingInitialResidual()
{
  return _nl->computingInitialResidual();
}

void
FEProblemBase::copySolutionsBackwards()
{
  _nl->copySolutionsBackwards();
  _aux->copySolutionsBackwards();
}

void
FEProblemBase::advanceState()
{
  _nl->copyOldSolutions();
  _aux->copyOldSolutions();

  if (_displaced_problem != NULL)
  {
    _displaced_problem->nlSys().copyOldSolutions();
    _displaced_problem->auxSys().copyOldSolutions();
  }

  _pps_data.copyValuesBack();

  if (_material_props.hasStatefulProperties())
    _material_props.shift();

  if (_bnd_material_props.hasStatefulProperties())
    _bnd_material_props.shift();
}

void
FEProblemBase::restoreSolutions()
{
  _nl->restoreSolutions();
  _aux->restoreSolutions();

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();
}

void
FEProblemBase::saveOldSolutions()
{
  _nl->saveOldSolutions();
  _aux->saveOldSolutions();
}

void
FEProblemBase::restoreOldSolutions()
{
  _nl->restoreOldSolutions();
  _aux->restoreOldSolutions();
}

void
FEProblemBase::outputStep(ExecFlagType type)
{
  _nl->update();
  _aux->update();
  if (_displaced_problem != NULL)
    _displaced_problem->syncSolutions();
  _app.getOutputWarehouse().outputStep(type);
}

void
FEProblemBase::allowOutput(bool state)
{
  _app.getOutputWarehouse().allowOutput(state);
}

void
FEProblemBase::forceOutput()
{
  _app.getOutputWarehouse().forceOutput();
}

void
FEProblemBase::initPetscOutput()
{
  _app.getOutputWarehouse().solveSetup();
  Moose::PetscSupport::petscSetDefaults(*this);
}

void
FEProblemBase::onTimestepBegin()
{
  _nl->onTimestepBegin();
}

void
FEProblemBase::onTimestepEnd()
{
}

void
FEProblemBase::addTimeIntegrator(const std::string & type,
                                 const std::string & name,
                                 InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  parameters.set<SubProblem *>("_subproblem") = this;
  _aux->addTimeIntegrator(type, name + ":aux", parameters);
  _nl->addTimeIntegrator(type, name, parameters);
  _has_time_integrator = true;
}

void
FEProblemBase::addPredictor(const std::string & type,
                            const std::string & name,
                            InputParameters parameters)
{
  setInputParametersFEProblem(parameters);
  parameters.set<SubProblem *>("_subproblem") = this;
  std::shared_ptr<Predictor> predictor = _factory.create<Predictor>(type, name, parameters);
  _nl->setPredictor(predictor);
}

Real
FEProblemBase::computeResidualL2Norm()
{
  computeResidualType(*_nl->currentSolution(), _nl->RHS());

  return _nl->RHS().l2_norm();
}

void
FEProblemBase::computeResidual(NonlinearImplicitSystem & /*sys*/,
                               const NumericVector<Number> & soln,
                               NumericVector<Number> & residual)
{
  computeResidual(soln, residual);
}

void
FEProblemBase::computeResidual(const NumericVector<Number> & soln, NumericVector<Number> & residual)
{
  try
  {
    computeResidualType(soln, residual, _kernel_type);
  }
  catch (MooseException & e)
  {
    // If a MooseException propagates all the way to here, it means
    // that it was thrown from a MOOSE system where we do not
    // (currently) properly support the throwing of exceptions, and
    // therefore we have no choice but to error out.  It may be
    // *possible* to handle exceptions from other systems, but in the
    // meantime, we don't want to silently swallow any unhandled
    // exceptions here.
    mooseError("An unhandled MooseException was raised during residual computation.  Please "
               "contact the MOOSE team for assistance.");
  }
}

void
FEProblemBase::computeTransientImplicitResidual(Real time,
                                                const NumericVector<Number> & u,
                                                const NumericVector<Number> & udot,
                                                NumericVector<Number> & residual)
{
  _nl->setSolutionUDot(udot);
  _time = time;
  computeResidual(u, residual);
}

void
FEProblemBase::computeResidualType(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual,
                                   Moose::KernelType type)
{
  _nl->setSolution(soln);

  _nl->zeroVariablesForResidual();
  _aux->zeroVariablesForResidual();

  unsigned int n_threads = libMesh::n_threads();

  _current_execute_on_flag = EXEC_LINEAR;

  // Random interface objects
  for (const auto & it : _random_data_objects)
    it.second->updateSeeds(EXEC_LINEAR);

  execTransfers(EXEC_LINEAR);

  execMultiApps(EXEC_LINEAR);

  for (unsigned int tid = 0; tid < n_threads; tid++)
    reinitScalars(tid);

  computeUserObjects(EXEC_LINEAR, Moose::PRE_AUX);

  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();

  for (THREAD_ID tid = 0; tid < n_threads; tid++)
  {
    _all_materials.residualSetup(tid);
    _functions.residualSetup(tid);
  }
  _aux->residualSetup();

  _nl->computeTimeDerivatives();

  try
  {
    _aux->compute(EXEC_LINEAR);
  }
  catch (MooseException & e)
  {
    _console << "\nA MooseException was raised during Auxiliary variable computation.\n"
             << "The next solve will fail, the timestep will be reduced, and we will try again.\n"
             << std::endl;

    // We know the next solve is going to fail, so there's no point in
    // computing anything else after this.  Plus, using incompletely
    // computed AuxVariables in subsequent calculations could lead to
    // other errors or unhandled exceptions being thrown.
    return;
  }

  computeUserObjects(EXEC_LINEAR, Moose::POST_AUX);

  executeControls(EXEC_LINEAR);

  _current_execute_on_flag = EXEC_NONE;

  _app.getOutputWarehouse().residualSetup();

  _nl->computeResidual(residual, type);
}

void
FEProblemBase::computeJacobian(NonlinearImplicitSystem & /*sys*/,
                               const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian)
{
  computeJacobian(soln, jacobian, Moose::KT_ALL);
}

void
FEProblemBase::computeJacobian(const NumericVector<Number> & soln,
                               SparseMatrix<Number> & jacobian,
                               Moose::KernelType kernel_type)
{
  if (!_has_jacobian || !_const_jacobian)
  {
    _nl->setSolution(soln);

    _nl->zeroVariablesForJacobian();
    _aux->zeroVariablesForJacobian();

    unsigned int n_threads = libMesh::n_threads();

    // Random interface objects
    for (const auto & it : _random_data_objects)
      it.second->updateSeeds(EXEC_NONLINEAR);

    _current_execute_on_flag = EXEC_NONLINEAR;
    _currently_computing_jacobian = true;

    execTransfers(EXEC_NONLINEAR);
    execMultiApps(EXEC_NONLINEAR);

    for (unsigned int tid = 0; tid < n_threads; tid++)
      reinitScalars(tid);

    computeUserObjects(EXEC_NONLINEAR, Moose::PRE_AUX);

    if (_displaced_problem != NULL)
      _displaced_problem->updateMesh();

    for (unsigned int tid = 0; tid < n_threads; tid++)
    {
      _all_materials.jacobianSetup(tid);
      _functions.jacobianSetup(tid);
    }

    _aux->jacobianSetup();

    _aux->compute(EXEC_NONLINEAR);

    computeUserObjects(EXEC_NONLINEAR, Moose::POST_AUX);

    executeControls(EXEC_NONLINEAR);

    _app.getOutputWarehouse().jacobianSetup();

    _nl->computeJacobian(jacobian, kernel_type);

    _current_execute_on_flag = EXEC_NONE;
    _currently_computing_jacobian = false;
    _has_jacobian = true;
  }

  if (_solver_params._type == Moose::ST_JFNK || _solver_params._type == Moose::ST_PJFNK)
  {
    // This call is here to make sure the residual vector is up to date with any decisions that have
    // been made in
    // the Jacobian evaluation.  That is important in JFNK because that residual is used for finite
    // differencing
    computeResidual(soln, _nl->RHS());
    _nl->RHS().close();
  }
}

void
FEProblemBase::computeTransientImplicitJacobian(Real time,
                                                const NumericVector<Number> & u,
                                                const NumericVector<Number> & udot,
                                                Real shift,
                                                SparseMatrix<Number> & jacobian)
{
  if (0)
  { // The current interface guarantees that the residual is called before Jacobian, thus udot has
    // already been set
    _nl->setSolutionUDot(udot);
  }
  _nl->duDotDu() = shift;
  _time = time;
  computeJacobian(u, jacobian);
}

void
FEProblemBase::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  if (_displaced_problem != NULL)
    _displaced_problem->updateMesh();

  _aux->compute(EXEC_NONLINEAR);

  _nl->computeJacobianBlocks(blocks);
}

void
FEProblemBase::computeJacobianBlock(SparseMatrix<Number> & jacobian,
                                    libMesh::System & precond_system,
                                    unsigned int ivar,
                                    unsigned int jvar)
{
  std::vector<JacobianBlock *> blocks;
  JacobianBlock * block = new JacobianBlock(precond_system, jacobian, ivar, jvar);
  blocks.push_back(block);
  computeJacobianBlocks(blocks);
  delete block;
}

void
FEProblemBase::computeBounds(NonlinearImplicitSystem & /*sys*/,
                             NumericVector<Number> & lower,
                             NumericVector<Number> & upper)
{
  if (!_nl->hasVector("lower_bound") || !_nl->hasVector("upper_bound"))
    return;

  NumericVector<Number> & _lower = _nl->getVector("lower_bound");
  NumericVector<Number> & _upper = _nl->getVector("upper_bound");
  _lower.swap(lower);
  _upper.swap(upper);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    _all_materials.residualSetup(tid);

  _aux->residualSetup();
  _aux->compute(EXEC_LINEAR);
  _lower.swap(lower);
  _upper.swap(upper);

  checkExceptionAndStopSolve();
}

void
FEProblemBase::computeNearNullSpace(NonlinearImplicitSystem & /*sys*/,
                                    std::vector<NumericVector<Number> *> & sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NearNullSpace"); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << i;
    std::string modename = "NearNullSpace" + postfix.str();
    sp.push_back(&_nl->getVector(modename));
  }
}

void
FEProblemBase::computeNullSpace(NonlinearImplicitSystem & /*sys*/,
                                std::vector<NumericVector<Number> *> & sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("NullSpace"); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_nl->getVector("NullSpace" + postfix.str()));
  }
}

void
FEProblemBase::computeTransposeNullSpace(NonlinearImplicitSystem & /*sys*/,
                                         std::vector<NumericVector<Number> *> & sp)
{
  sp.clear();
  for (unsigned int i = 0; i < subspaceDim("TransposeNullSpace"); ++i)
  {
    std::stringstream postfix;
    postfix << "_" << i;
    sp.push_back(&_nl->getVector("TransposeNullSpace" + postfix.str()));
  }
}

void
FEProblemBase::computePostCheck(NonlinearImplicitSystem & sys,
                                const NumericVector<Number> & old_soln,
                                NumericVector<Number> & search_direction,
                                NumericVector<Number> & new_soln,
                                bool & changed_search_direction,
                                bool & changed_new_soln)
{
  // This function replaces the old PetscSupport::dampedCheck() function.
  //
  // 1.) Recreate code in PetscSupport::dampedCheck() for constructing
  //     ghosted "soln" and "update" vectors.
  // 2.) Call FEProblemBase::computeDamping() with these ghost vectors.
  // 3.) Recreate the code in PetscSupport::dampedCheck() to actually update
  //     the solution vector based on the damping, and set the "changed" flags
  //     appropriately.
  Moose::perf_log.push("computePostCheck()", "Execution");

  // MOOSE's FEProblemBase doesn't update the solution during the
  // postcheck, but FEProblemBase-derived classes might.
  if (_has_dampers || shouldUpdateSolution())
  {
    // We need ghosted versions of new_soln and search_direction (the
    // ones we get from libmesh/PETSc are PARALLEL vectors.  To make
    // our lives simpler, we use the same ghosting pattern as the
    // system's current_local_solution to create new ghosted vectors.

    // Construct zeroed-out clones with the same ghosted dofs as the
    // System's current_local_solution.
    std::unique_ptr<NumericVector<Number>> ghosted_solution =
                                               sys.current_local_solution->zero_clone(),
                                           ghosted_search_direction =
                                               sys.current_local_solution->zero_clone();

    // Copy values from input vectors into clones with ghosted values.
    *ghosted_solution = new_soln;
    *ghosted_search_direction = search_direction;

    if (_has_dampers)
    {
      // Compute the damping coefficient using the ghosted vectors
      Real damping = computeDamping(*ghosted_solution, *ghosted_search_direction);

      // If some non-trivial damping was computed, update the new_soln
      // vector accordingly.
      if (damping < 1.0)
      {
        new_soln = old_soln;
        new_soln.add(-damping, search_direction);
        changed_new_soln = true;
      }
    }

    if (shouldUpdateSolution())
    {
      // Update the ghosted copy of the new solution, if necessary.
      if (changed_new_soln)
        *ghosted_solution = new_soln;

      bool updated_solution = updateSolution(new_soln, *ghosted_solution);
      if (updated_solution)
        changed_new_soln = true;
    }
  }

  if (_needs_old_newton_iter)
  {
    _nl->setPreviousNewtonSolution(old_soln);
    _aux->setPreviousNewtonSolution();
  }

  // MOOSE doesn't change the search_direction
  changed_search_direction = false;

  Moose::perf_log.pop("computePostCheck()", "Execution");
}

Real
FEProblemBase::computeDamping(const NumericVector<Number> & soln,
                              const NumericVector<Number> & update)
{
  Moose::perf_log.push("compute_dampers()", "Execution");

  // Default to no damping
  Real damping = 1.0;

  if (_has_dampers)
  {
    // Save pointer to the current solution
    const NumericVector<Number> * _saved_current_solution = _nl->currentSolution();

    _nl->setSolution(soln);
    // For now, do not re-compute auxiliary variables.  Doing so allows a wild solution increment
    //   to get to the material models, which may not be able to cope with drastically different
    //   values.  Once more complete dependency checking is in place, auxiliary variables (and
    //   material properties) will be computed as needed by dampers.
    //    _aux.compute();
    damping = _nl->computeDamping(soln, update);

    // restore saved solution
    _nl->setSolution(*_saved_current_solution);
  }

  Moose::perf_log.pop("compute_dampers()", "Execution");

  return damping;
}

bool
FEProblemBase::shouldUpdateSolution()
{
  return false;
}

bool
FEProblemBase::updateSolution(NumericVector<Number> & /*vec_solution*/,
                              NumericVector<Number> & /*ghosted_solution*/)
{
  return false;
}

void
FEProblemBase::predictorCleanup(NumericVector<Number> & /*ghosted_solution*/)
{
}

void
FEProblemBase::addDisplacedProblem(std::shared_ptr<DisplacedProblem> displaced_problem)
{
  _displaced_mesh = &displaced_problem->mesh();
  _displaced_problem = displaced_problem;
}

void
FEProblemBase::updateGeomSearch(GeometricSearchData::GeometricSearchType type)
{
  _geometric_search_data.update(type);

  if (_displaced_problem)
    _displaced_problem->updateGeomSearch(type);
}

void
FEProblemBase::possiblyRebuildGeomSearchPatches()
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
        // Flush output here to see the message before the reinitialization, which could take a
        // while
        _console << "\n\nUpdating geometric search patches\n" << std::endl;

        _geometric_search_data.clearNearestNodeLocators();
        _mesh.updateActiveSemiLocalNodeRange(_ghosted_elems);

        _displaced_problem->geomSearchData().clearNearestNodeLocators();
        _displaced_mesh->updateActiveSemiLocalNodeRange(_ghosted_elems);

        reinitBecauseOfGhostingOrNewGeomObjects();

        // This is needed to reinitialize PETSc output
        initPetscOutput();
    }
  }
}

#ifdef LIBMESH_ENABLE_AMR
void
FEProblemBase::initialAdaptMesh()
{
  unsigned int n = adaptivity().getInitialSteps();
  _cycles_completed = 0;
  for (unsigned int i = 0; i < n; i++)
  {
    _console << "Initial adaptivity step " << i + 1 << " of " << n << std::endl;
    computeIndicators();
    computeMarkers();

    if (_adaptivity.initialAdaptMesh())
    {
      meshChanged();

      // reproject the initial condition
      projectSolution();

      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      return;
    }
  }
}

void
FEProblemBase::adaptMesh()
{
  // reset cycle counter
  _cycles_completed = 0;

  if (!_adaptivity.isAdaptivityDue())
    return;

  unsigned int cycles_per_step = _adaptivity.getCyclesPerStep();

  Moose::perf_log.push("Adaptivity: adaptMesh()", "Execution");

  for (unsigned int i = 0; i < cycles_per_step; ++i)
  {
    _console << "Adaptivity step " << i + 1 << " of " << cycles_per_step << '\n';
    // Markers were already computed once by Executioner
    if (_adaptivity.getRecomputeMarkersFlag() && i > 0)
      computeMarkers();
    if (_adaptivity.adaptMesh())
    {
      meshChanged();
      _cycles_completed++;
    }
    else
    {
      _console << "Mesh unchanged, skipping remaining steps..." << std::endl;
      break;
    }

    // Show adaptivity progress
    _console << std::flush;
  }

  Moose::perf_log.pop("Adaptivity: adaptMesh()", "Execution");
}
#endif // LIBMESH_ENABLE_AMR

void
FEProblemBase::initXFEM(std::shared_ptr<XFEMInterface> xfem)
{
  _xfem = xfem;
  _xfem->setMesh(&_mesh);
  if (_displaced_mesh)
    _xfem->setDisplacedMesh(_displaced_mesh);
  _xfem->setMaterialData(&_material_data);
  _xfem->setBoundaryMaterialData(&_bnd_material_data);

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; ++i)
  {
    _assembly[i]->setXFEM(_xfem);
    if (_displaced_problem != NULL)
      _displaced_problem->assembly(i).setXFEM(_xfem);
  }
}

bool
FEProblemBase::updateMeshXFEM()
{
  bool updated = false;
  if (haveXFEM())
  {
    updated = _xfem->update(_time, *_nl, *_aux);
    if (updated)
    {
      meshChanged();
      _xfem->initSolution(*_nl, *_aux);
      restoreSolutions();
    }
  }
  return updated;
}

void
FEProblemBase::meshChanged()
{
  if (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties())
    _mesh.cacheChangedLists(); // Currently only used with adaptivity and stateful material
                               // properties

  // Clear these out because they corresponded to the old mesh
  _ghosted_elems.clear();

  ghostGhostedBoundaries();

  // The mesh changed.  We notify the MooseMesh first, because
  // callbacks (e.g. for sparsity calculations) triggered by the
  // EquationSystems reinit may require up-to-date MooseMesh caches.
  _mesh.meshChanged();
  _eq.reinit();

  // But that breaks other adaptivity code, unless we then *again*
  // update the MooseMesh caches.  E.g. the definition of "active" and
  // "local" may be *changed* by EquationSystems::reinit().
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

  reinitBecauseOfGhostingOrNewGeomObjects();

  // We need to create new storage for the new elements and copy stateful properties from the old
  // elements.
  if (_has_initialized_stateful &&
      (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
  {
    {
      ProjectMaterialProperties pmp(true,
                                    *this,
                                    *_nl,
                                    _material_data,
                                    _bnd_material_data,
                                    _material_props,
                                    _bnd_material_props,
                                    _assembly);
      Threads::parallel_reduce(*_mesh.refinedElementRange(), pmp);
    }

    {
      ProjectMaterialProperties pmp(false,
                                    *this,
                                    *_nl,
                                    _material_data,
                                    _bnd_material_data,
                                    _material_props,
                                    _bnd_material_props,
                                    _assembly);
      Threads::parallel_reduce(*_mesh.coarsenedElementRange(), pmp);
    }
  }

  if (_calculate_jacobian_in_uo)
    setVariableAllDoFMap(_uo_jacobian_moose_vars[0]);

  _has_jacobian = false; // we have to recompute jacobian when mesh changed

  for (const auto & mci : _notify_when_mesh_changes)
    mci->meshChanged();
}

void
FEProblemBase::notifyWhenMeshChanges(MeshChangedInterface * mci)
{
  _notify_when_mesh_changes.push_back(mci);
}

void
FEProblemBase::checkProblemIntegrity()
{
  // Check for unsatisfied actions
  const std::set<SubdomainID> & mesh_subdomains = _mesh.meshSubdomains();

  // Check kernel coverage of subdomains (blocks) in the mesh
  if (_solve && _kernel_coverage_check)
    _nl->checkKernelCoverage(mesh_subdomains);

  // Check materials
  {
#ifdef LIBMESH_ENABLE_AMR
    if (_adaptivity.isOn() &&
        (_material_props.hasStatefulProperties() || _bnd_material_props.hasStatefulProperties()))
    {
      _console << "Using EXPERIMENTAL Stateful Material Property projection with Adaptivity!\n";

      if (n_processors() > 1)
      {
        if (_mesh.uniformRefineLevel() > 0 && _mesh.getMesh().skip_partitioning() == false)
          mooseError("This simulation is using uniform refinement on the mesh, with stateful "
                     "properties and adaptivity. "
                     "You must skip partitioning to run this case:\nMesh/skip_partitioning=true");

        _console << "\nWarning! Mesh re-partitioning is disabled while using stateful material "
                    "properties!  This can lead to large load imbalances and degraded "
                    "performance!!\n\n";
        _mesh.getMesh().skip_partitioning(true);

        _mesh.errorIfDistributedMesh("StatefulMaterials + Adaptivity");

        if (_displaced_problem)
          _displaced_problem->mesh().getMesh().skip_partitioning(true);
      }
    }
#endif

    std::set<SubdomainID> local_mesh_subs(mesh_subdomains);

    if (_material_coverage_check)
    {
      /**
       * If a material is specified for any block in the simulation, then all blocks must
       * have a material specified.
       */
      bool check_material_coverage = false;
      std::set<SubdomainID> ids = _all_materials.getActiveBlocks();
      for (const auto & id : ids)
      {
        local_mesh_subs.erase(id);
        check_material_coverage = true;
      }

      // also exclude mortar spaces from the material check
      auto & mortar_ifaces = _mesh.getMortarInterfaces();
      for (const auto & mortar_iface : mortar_ifaces)
        local_mesh_subs.erase(mortar_iface->_id);

      // Check Material Coverage
      if (check_material_coverage && !local_mesh_subs.empty())
      {
        std::stringstream extra_subdomain_ids;
        /// unsigned int is necessary to print SubdomainIDs in the statement below
        std::copy(local_mesh_subs.begin(),
                  local_mesh_subs.end(),
                  std::ostream_iterator<unsigned int>(extra_subdomain_ids, " "));

        mooseError("The following blocks from your input mesh do not contain an active material: " +
                   extra_subdomain_ids.str() + "\nWhen ANY mesh block contains a Material object, "
                                               "all blocks must contain a Material object.\n");
      }
    }

    // Check material properties on blocks and boundaries
    checkBlockMatProps();
    checkBoundaryMatProps();

    // Check that material properties exist when requested by other properties on a given block
    const auto & materials = _all_materials.getActiveObjects();
    for (const auto & material : materials)
      material->checkStatefulSanity();

    checkDependMaterialsHelper(_all_materials.getActiveBlockObjects());
  }

  // Check UserObjects and Postprocessors
  checkUserObjects();

  // Verify that we don't have any Element type/Coordinate Type conflicts
  checkCoordinateSystems();

  // If using displacements, verify that the order of the displacement
  // variables matches the order of the elements in the displaced
  // mesh.
  checkDisplacementOrders();
}

void
FEProblemBase::checkDisplacementOrders()
{
  if (_displaced_problem)
  {
    MeshBase::const_element_iterator it = _displaced_mesh->activeLocalElementsBegin(),
                                     end = _displaced_mesh->activeLocalElementsEnd();

    bool mesh_has_second_order_elements = false;
    for (; it != end; ++it)
    {
      if ((*it)->default_order() == SECOND)
      {
        mesh_has_second_order_elements = true;
        break;
      }
    }

    // We checked our local elements, so take the max over all processors.
    _displaced_mesh->comm().max(mesh_has_second_order_elements);

    // If the Mesh has second order elements, make sure the
    // displacement variables are second-order.
    if (mesh_has_second_order_elements)
    {
      const std::vector<std::string> & displacement_variables =
          _displaced_problem->getDisplacementVarNames();

      for (const auto & var_name : displacement_variables)
      {
        MooseVariable & mv = _displaced_problem->getVariable(/*tid=*/0, var_name);
        if (mv.order() != SECOND)
          mooseError("Error: mesh has SECOND order elements, so all displacement variables must be "
                     "SECOND order.");
      }
    }
  }
}

void
FEProblemBase::checkUserObjects()
{
  // Check user_objects block coverage
  std::set<SubdomainID> mesh_subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID> user_objects_blocks;

  // gather names of all user_objects that were defined in the input file
  // and the blocks that they are defined on
  std::set<std::string> names;

  const auto & objects = _all_user_objects.getActiveObjects();
  for (const auto & obj : objects)
    names.insert(obj->name());

  // See if all referenced blocks are covered
  mesh_subdomains.insert(Moose::ANY_BLOCK_ID);
  std::set<SubdomainID> difference;
  std::set_difference(user_objects_blocks.begin(),
                      user_objects_blocks.end(),
                      mesh_subdomains.begin(),
                      mesh_subdomains.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more UserObjects is referencing a nonexistent block:\n";
    for (const auto & id : difference)
      oss << id << "\n";
    mooseError(oss.str());
  }

  // check that all requested UserObjects were defined in the input file
  for (const auto & it : _pps_data.values())
  {
    if (names.find(it.first) == names.end())
      mooseError("Postprocessor '" + it.first + "' requested but not specified in the input file.");
  }
}

void
FEProblemBase::checkDependMaterialsHelper(
    const std::map<SubdomainID, std::vector<std::shared_ptr<Material>>> & materials_map)
{
  auto & prop_names = _material_props.statefulPropNames();

  for (const auto & it : materials_map)
  {
    /// These two sets are used to make sure that all dependent props on a block are actually supplied
    std::set<std::string> block_depend_props, block_supplied_props;

    for (const auto & mat1 : it.second)
    {
      const std::set<std::string> & depend_props = mat1->getRequestedItems();
      block_depend_props.insert(depend_props.begin(), depend_props.end());

      auto & alldeps = mat1->getMatPropDependencies(); // includes requested stateful props
      for (auto & dep : alldeps)
      {
        if (prop_names.count(dep) > 0)
          block_depend_props.insert(prop_names.at(dep));
      }

      // See if any of the active materials supply this property
      for (const auto & mat2 : it.second)
      {
        const std::set<std::string> & supplied_props = mat2->Material::getSuppliedItems();
        block_supplied_props.insert(supplied_props.begin(), supplied_props.end());
      }
    }

    // Add zero material properties specific to this block and unrestricted
    block_supplied_props.insert(_zero_block_material_props[it.first].begin(),
                                _zero_block_material_props[it.first].end());
    block_supplied_props.insert(_zero_block_material_props[Moose::ANY_BLOCK_ID].begin(),
                                _zero_block_material_props[Moose::ANY_BLOCK_ID].end());

    // Error check to make sure all properties consumed by materials are supplied on this block
    std::set<std::string> difference;
    std::set_difference(block_depend_props.begin(),
                        block_depend_props.end(),
                        block_supplied_props.begin(),
                        block_supplied_props.end(),
                        std::inserter(difference, difference.end()));

    if (!difference.empty())
    {
      std::ostringstream oss;
      oss << "One or more Material Properties were not supplied on block " << it.first << ":\n";
      for (const auto & name : difference)
        oss << name << "\n";
      mooseError(oss.str());
    }
  }

  // This loop checks that materials are not supplied by multiple Material objects
  for (const auto & it : materials_map)
  {
    const auto & materials = it.second;
    std::set<std::string> inner_supplied, outer_supplied;

    for (const auto & outer_mat : materials)
    {
      // Storage for properties for this material (outer) and all other materials (inner)
      outer_supplied = outer_mat->getSuppliedItems();
      inner_supplied.clear();

      // Property to material map for error reporting
      std::map<std::string, std::set<std::string>> prop_to_mat;
      for (const auto & name : outer_supplied)
        prop_to_mat[name].insert(outer_mat->name());

      for (const auto & inner_mat : materials)
      {
        if (outer_mat == inner_mat)
          continue;
        inner_supplied.insert(inner_mat->getSuppliedItems().begin(),
                              inner_mat->getSuppliedItems().end());

        for (const auto & inner_supplied_name : inner_supplied)
          prop_to_mat[inner_supplied_name].insert(inner_mat->name());
      }

      // Test that a property isn't supplied on multiple blocks
      std::set<std::string> intersection;
      std::set_intersection(outer_supplied.begin(),
                            outer_supplied.end(),
                            inner_supplied.begin(),
                            inner_supplied.end(),
                            std::inserter(intersection, intersection.end()));

      if (!intersection.empty())
      {
        std::ostringstream oss;
        oss << "The following material properties are declared on block " << it.first
            << " by multiple materials:\n";
        oss << ConsoleUtils::indent(2) << std::setw(30) << std::left << "Material Property"
            << "Material Objects\n";
        for (const auto & outer_name : intersection)
        {
          oss << ConsoleUtils::indent(2) << std::setw(30) << std::left << outer_name;
          for (const auto & inner_name : prop_to_mat[outer_name])
            oss << inner_name << " ";
          oss << '\n';
        }

        mooseError(oss.str());
        break;
      }
    }
  }
}

void
FEProblemBase::checkCoordinateSystems()
{
  MeshBase::const_element_iterator it = _mesh.getMesh().elements_begin();
  MeshBase::const_element_iterator it_end = _mesh.getMesh().elements_end();

  for (; it != it_end; ++it)
  {
    SubdomainID sid = (*it)->subdomain_id();
    if (_coord_sys[sid] == Moose::COORD_RZ && (*it)->dim() == 3)
      mooseError("An RZ coordinate system was requested for subdomain " + Moose::stringify(sid) +
                 " which contains 3D elements.");
    if (_coord_sys[sid] == Moose::COORD_RSPHERICAL && (*it)->dim() > 1)
      mooseError("An RSPHERICAL coordinate system was requested for subdomain " +
                 Moose::stringify(sid) + " which contains 2D or 3D elements.");
  }
}

void
FEProblemBase::serializeSolution()
{
  _nl->serializeSolution();
  _aux->serializeSolution();
}

void
FEProblemBase::setRestartFile(const std::string & file_name)
{
  _app.setRestart(true);
  _resurrector->setRestartFile(file_name);
  if (_app.getRecoverFileSuffix() == "cpa")
    _resurrector->setRestartSuffix("xda");
}

std::vector<VariableName>
FEProblemBase::getVariableNames()
{
  std::vector<VariableName> names;

  const std::vector<VariableName> & nl_var_names = _nl->getVariableNames();
  names.insert(names.end(), nl_var_names.begin(), nl_var_names.end());

  const std::vector<VariableName> & aux_var_names = _aux->getVariableNames();
  names.insert(names.end(), aux_var_names.begin(), aux_var_names.end());

  return names;
}

MooseNonlinearConvergenceReason
FEProblemBase::checkNonlinearConvergence(std::string & msg,
                                         const PetscInt it,
                                         const Real xnorm,
                                         const Real snorm,
                                         const Real fnorm,
                                         const Real rtol,
                                         const Real stol,
                                         const Real abstol,
                                         const PetscInt nfuncs,
                                         const PetscInt max_funcs,
                                         const Real initial_residual_before_preset_bcs,
                                         const Real div_threshold)
{
  NonlinearSystemBase & system = getNonlinearSystemBase();
  MooseNonlinearConvergenceReason reason = MOOSE_NONLINEAR_ITERATING;

  // This is the first residual before any iterations have been done,
  // but after PresetBCs (if any) have been imposed on the solution
  // vector.  We save it, and use it to detect convergence if
  // compute_initial_residual_before_preset_bcs=false.
  if (it == 0)
    system._initial_residual_after_preset_bcs = fnorm;

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
    oss << "Exceeded maximum number of function evaluations: " << nfuncs << " > " << max_funcs
        << '\n';
    reason = MOOSE_DIVERGED_FUNCTION_COUNT;
  }
  else if (it && fnorm > system._last_nl_rnorm && fnorm >= div_threshold)
  {
    oss << "Nonlinear solve was blowing up!\n";
    reason = MOOSE_DIVERGED_LINE_SEARCH;
  }

  if (it && !reason)
  {
    // If compute_initial_residual_before_preset_bcs==false, then use the
    // first residual computed by Petsc to determine convergence.
    Real the_residual = system._compute_initial_residual_before_preset_bcs
                            ? initial_residual_before_preset_bcs
                            : system._initial_residual_after_preset_bcs;
    if (fnorm <= the_residual * rtol)
    {
      oss << "Converged due to function norm " << fnorm << " < "
          << " (relative tolerance)\n";
      reason = MOOSE_CONVERGED_FNORM_RELATIVE;
    }
    else if (snorm < stol * xnorm)
    {
      oss << "Converged due to small update length: " << snorm << " < " << stol << " * " << xnorm
          << '\n';
      reason = MOOSE_CONVERGED_SNORM_RELATIVE;
    }
  }

  system._last_nl_rnorm = fnorm;
  system._current_nl_its = static_cast<unsigned int>(it);

  msg = oss.str();
  if (_app.multiAppLevel() > 0)
    MooseUtils::indentMessage(_app.name(), msg);

  return (reason);
}

MooseLinearConvergenceReason
FEProblemBase::checkLinearConvergence(std::string & /*msg*/,
                                      const PetscInt n,
                                      const Real rnorm,
                                      const Real /*rtol*/,
                                      const Real /*atol*/,
                                      const Real /*dtol*/,
                                      const PetscInt maxits)
{
  if (_fail_next_linear_convergence_check)
  {
    // Unset the flag
    _fail_next_linear_convergence_check = false;
    return MOOSE_DIVERGED_NANORINF;
  }

  // We initialize the reason to something that basically means MOOSE
  // has not made a decision on convergence yet.
  MooseLinearConvergenceReason reason = MOOSE_LINEAR_ITERATING;

  // Get a reference to our Nonlinear System
  NonlinearSystemBase & system = getNonlinearSystemBase();

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

SolverParams &
FEProblemBase::solverParams()
{
  return _solver_params;
}

void
FEProblemBase::registerRandomInterface(RandomInterface & random_interface, const std::string & name)
{
  auto insert_pair = moose_try_emplace(
      _random_data_objects, name, libmesh_make_unique<RandomData>(*this, random_interface));

  auto random_data_ptr = insert_pair.first->second.get();
  random_interface.setRandomDataPointer(random_data_ptr);
}

bool
FEProblemBase::needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid)
{
  if (_bnd_mat_side_cache[tid].find(bnd_id) == _bnd_mat_side_cache[tid].end())
  {
    _bnd_mat_side_cache[tid][bnd_id] = false;

    if (_nl->needMaterialOnSide(bnd_id, tid) || _aux->needMaterialOnSide(bnd_id))
      _bnd_mat_side_cache[tid][bnd_id] = true;

    else if (_side_user_objects.hasActiveBoundaryObjects(bnd_id, tid))
      _bnd_mat_side_cache[tid][bnd_id] = true;
  }

  return _bnd_mat_side_cache[tid][bnd_id];
}

bool
FEProblemBase::needMaterialOnSide(SubdomainID subdomain_id, THREAD_ID tid)
{
  if (_block_mat_side_cache[tid].find(subdomain_id) == _block_mat_side_cache[tid].end())
  {
    _block_mat_side_cache[tid][subdomain_id] = false;

    if (_nl->needMaterialOnSide(subdomain_id, tid))
      _block_mat_side_cache[tid][subdomain_id] = true;
    else if (_internal_side_user_objects.hasActiveBlockObjects(subdomain_id, tid))
      _block_mat_side_cache[tid][subdomain_id] = true;
  }

  return _block_mat_side_cache[tid][subdomain_id];
}

bool
FEProblemBase::needsPreviousNewtonIteration()
{
  return _needs_old_newton_iter;
}

void
FEProblemBase::needsPreviousNewtonIteration(bool state /* = true*/)
{
  _needs_old_newton_iter = state;
}
