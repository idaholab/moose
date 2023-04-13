//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingStudy.h"

// Local Includes
#include "AuxRayKernel.h"
#include "RayBoundaryConditionBase.h"
#include "RayKernel.h"
#include "TraceRay.h"
#include "TraceRayTools.h"

// MOOSE Includes
#include "AuxiliarySystem.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"

// libMesh Includes
#include "libmesh/enum_to_string.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/remote_elem.h"

InputParameters
RayTracingStudy::validParams()
{
  auto params = GeneralUserObject::validParams();

  // Parameters for the execution of the rays
  params += ParallelRayStudy::validParams();

  params.addRangeCheckedParam<Real>("ray_distance",
                                    std::numeric_limits<Real>::max(),
                                    "ray_distance > 0",
                                    "The maximum distance all Rays can travel");

  params.addParam<bool>(
      "tolerate_failure", false, "Whether or not to tolerate a ray tracing failure");

  MooseEnum work_buffers("lifo circular", "circular");
  params.addParam<MooseEnum>("work_buffer_type", work_buffers, "The work buffer type to use");

  params.addParam<bool>(
      "ray_kernel_coverage_check", true, "Whether or not to perform coverage checks on RayKernels");
  params.addParam<bool>("warn_non_planar",
                        true,
                        "Whether or not to produce a warning if any element faces are non-planar.");

  params.addParam<bool>(
      "always_cache_traces",
      false,
      "Whether or not to cache the Ray traces on every execution, primarily for use in output. "
      "Warning: this can get expensive very quick with a large number of rays!");
  params.addParam<bool>("data_on_cache_traces",
                        false,
                        "Whether or not to also cache the Ray's data when caching its traces");
  params.addParam<bool>("aux_data_on_cache_traces",
                        false,
                        "Whether or not to also cache the Ray's aux data when caching its traces");
  params.addParam<bool>(
      "segments_on_cache_traces",
      true,
      "Whether or not to cache individual segments when trace caching is enabled. If false, we "
      "will instead cache a segment for each part of the trace where the direction is the same. "
      "This minimizes the number of segments requied to represent the Ray's path, but removes the "
      "ability to show Ray field data on each segment through an element.");

  params.addParam<bool>("use_internal_sidesets",
                        false,
                        "Whether or not to use internal sidesets for RayBCs in ray tracing");

  params.addParam<bool>("warn_subdomain_hmax",
                        true,
                        "Whether or not to warn if the approximated hmax (constant on subdomain) "
                        "varies significantly for an element");

  params.addParam<bool>(
      "verify_rays",
      true,
      "Whether or not to verify the generated Rays. This includes checking their "
      "starting information and the uniqueness of Rays before and after execution. This is also "
      "used by derived studies for more specific verification.");
  params.addParam<bool>("verify_trace_intersections",
                        true,
                        "Whether or not to verify the trace intersections in devel and dbg modes. "
                        "Trace intersections are not verified regardless of this parameter in "
                        "optimized modes (opt, oprof).");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);

  params.addParamNamesToGroup(
      "always_cache_traces data_on_cache_traces aux_data_on_cache_traces segments_on_cache_traces",
      "Trace cache");
  params.addParamNamesToGroup("warn_non_planar warn_subdomain_hmax", "Tracing Warnings");
  params.addParamNamesToGroup("ray_kernel_coverage_check verify_rays verify_trace_intersections",
                              "Checks and verifications");

  // Whether or not each Ray must be registered using the registerRay() API
  params.addPrivateParam<bool>("_use_ray_registration", true);
  // Whether or not to bank Rays on completion
  params.addPrivateParam<bool>("_bank_rays_on_completion", true);
  /// Whether or not subdomain setup is dependent on the Ray
  params.addPrivateParam<bool>("_ray_dependent_subdomain_setup", true);

  // Add a point neighbor relationship manager
  params.addRelationshipManager("ElementPointNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 1; });

  return params;
}

RayTracingStudy::RayTracingStudy(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh()),
    _comm(_mesh.comm()),
    _pid(_comm.rank()),

    _ray_kernel_coverage_check(getParam<bool>("ray_kernel_coverage_check")),
    _warn_non_planar(getParam<bool>("warn_non_planar")),
    _use_ray_registration(getParam<bool>("_use_ray_registration")),
    _use_internal_sidesets(getParam<bool>("use_internal_sidesets")),
    _tolerate_failure(getParam<bool>("tolerate_failure")),
    _bank_rays_on_completion(getParam<bool>("_bank_rays_on_completion")),
    _ray_dependent_subdomain_setup(getParam<bool>("_ray_dependent_subdomain_setup")),

    _always_cache_traces(getParam<bool>("always_cache_traces")),
    _data_on_cache_traces(getParam<bool>("data_on_cache_traces")),
    _aux_data_on_cache_traces(getParam<bool>("aux_data_on_cache_traces")),
    _segments_on_cache_traces(getParam<bool>("segments_on_cache_traces")),
    _ray_max_distance(getParam<Real>("ray_distance")),
    _verify_rays(getParam<bool>("verify_rays")),
#ifndef NDEBUG
    _verify_trace_intersections(getParam<bool>("verify_trace_intersections")),
#endif

    _threaded_elem_side_builders(libMesh::n_threads()),

    _registered_ray_map(
        declareRestartableData<std::unordered_map<std::string, RayID>>("registered_ray_map")),
    _reverse_registered_ray_map(
        declareRestartableData<std::vector<std::string>>("reverse_registered_ray_map")),

    _threaded_cached_traces(libMesh::n_threads()),

    _num_cached(libMesh::n_threads(), 0),

    _has_non_planar_sides(true),
    _has_same_level_active_elems(sameLevelActiveElems()),

    _b_box(MeshTools::create_nodal_bounding_box(_mesh.getMesh())),
    _domain_max_length(1.01 * (_b_box.max() - _b_box.min()).norm()),
    _total_volume(computeTotalVolume()),

    _threaded_cache_ray_kernel(libMesh::n_threads()),
    _threaded_cache_ray_bc(libMesh::n_threads()),
    _threaded_ray_object_registration(libMesh::n_threads()),
    _threaded_current_ray_kernels(libMesh::n_threads()),
    _threaded_trace_ray(libMesh::n_threads()),
    _threaded_fe_face(libMesh::n_threads()),
    _threaded_q_face(libMesh::n_threads()),
    _threaded_cached_normals(libMesh::n_threads()),
    _threaded_next_ray_id(libMesh::n_threads()),

    _parallel_ray_study(std::make_unique<ParallelRayStudy>(*this, _threaded_trace_ray)),

    _local_trace_ray_results(TraceRay::FAILED_TRACES + 1, 0),

    _called_initial_setup(false),

    _elem_index_helper(_mesh.getMesh(), name() + "_elem_index")
{
  // Initialize a tracing object for each thread
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    // Initialize a tracing object for each thread
    _threaded_trace_ray[tid] = std::make_shared<TraceRay>(*this, tid);

    // Setup the face FEs for normal computation on the fly
    _threaded_fe_face[tid] = FEBase::build(_mesh.dimension(), FEType(CONSTANT, MONOMIAL));
    _threaded_q_face[tid] = QBase::build(QGAUSS, _mesh.dimension() - 1, CONSTANT);
    _threaded_fe_face[tid]->attach_quadrature_rule(_threaded_q_face[tid].get());
    _threaded_fe_face[tid]->get_normals();
  }

  // Evaluating on residual and Jacobian evaluation
  if (_execute_enum.contains(EXEC_PRE_KERNELS))
  {
    if (_execute_enum.size() > 1)
      paramError("execute_on",
                 "PRE_KERNELS cannot be mixed with any other execution flag.\nThat is, you cannot "
                 "currently "
                 "mix RayKernels that contribute to the Jacobian/residual with those that do not.");

    if (_app.useEigenvalue())
      mooseError("Execution on residual and Jacobian evaluation (execute_on = PRE_KERNELS)\n",
                 "is not supported for an eigenvalue solve.");
  }

  resetUniqueRayIDs();
  resetReplicatedRayIDs();

  // Scale the bounding box for loose checking
  _loose_b_box = _b_box;
  _loose_b_box.scale(TOLERANCE * TOLERANCE);
}

void
RayTracingStudy::initialSetup()
{
  // Keep track of initialSetup call to avoid registration of various things
  _called_initial_setup = true;

  // Sets up a local index for each elem this proc knows about
  localElemIndexSetup();

  // Check for RayKernel coverage
  coverageChecks();

  // Make sure the dependencies exist, if any
  dependencyChecks();

  // Check for traceable element types
  traceableMeshChecks();

  // Setup for internal sidesets
  internalSidesetSetup();

  nonPlanarSideSetup();

  // Setup approximate hmax for each subdomain
  subdomainHMaxSetup();

  // Call initial setup on all of the objects
  for (auto & rto : getRayTracingObjects())
    rto->initialSetup();

  // Check for proper exec flags with RayKernels
  std::vector<RayKernelBase *> ray_kernels;
  getRayKernels(ray_kernels, 0);
  for (const auto & rkb : ray_kernels)
    if (dynamic_cast<RayKernel *>(rkb) && !_execute_enum.contains(EXEC_PRE_KERNELS))
      mooseError("This study has RayKernel objects that contribute to residuals and Jacobians.",
                 "\nIn this case, the study must use the execute_on = PRE_KERNELS");

  // Build 1D quadrature rule for along a segment
  _segment_qrule =
      QBase::build(QGAUSS, 1, _fe_problem.getNonlinearSystemBase().getMinQuadratureOrder());
}

void
RayTracingStudy::residualSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    mooseAssert(_num_cached[tid] == 0, "Cached residuals/Jacobians not empty");

  for (auto & rto : getRayTracingObjects())
    rto->residualSetup();
}

void
RayTracingStudy::jacobianSetup()
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    mooseAssert(_num_cached[tid] == 0, "Cached residuals/Jacobians not empty");

  for (auto & rto : getRayTracingObjects())
    rto->jacobianSetup();
}

void
RayTracingStudy::timestepSetup()
{
  for (auto & rto : getRayTracingObjects())
    rto->timestepSetup();
}

void
RayTracingStudy::meshChanged()
{
  localElemIndexSetup();
  internalSidesetSetup();
  nonPlanarSideSetup();
  subdomainHMaxSetup();

  _has_same_level_active_elems = sameLevelActiveElems();

  for (const auto & trace_ray : _threaded_trace_ray)
    trace_ray->meshChanged();
}

void
RayTracingStudy::execute()
{
  executeStudy();
}

void
RayTracingStudy::coverageChecks()
{
  // Check for coverage of RayKernels on domain
  if (_ray_kernel_coverage_check)
  {
    std::vector<RayKernelBase *> ray_kernels;
    getRayKernels(ray_kernels, 0);

    std::set<SubdomainID> ray_kernel_blocks;
    for (const auto & rk : ray_kernels)
      ray_kernel_blocks.insert(rk->blockIDs().begin(), rk->blockIDs().end());

    std::set<SubdomainID> missing;
    std::set_difference(_mesh.meshSubdomains().begin(),
                        _mesh.meshSubdomains().end(),
                        ray_kernel_blocks.begin(),
                        ray_kernel_blocks.end(),
                        std::inserter(missing, missing.begin()));

    if (!missing.empty() && !ray_kernel_blocks.count(Moose::ANY_BLOCK_ID))
    {
      std::ostringstream error;
      error << "Subdomains { ";
      std::copy(missing.begin(), missing.end(), std::ostream_iterator<SubdomainID>(error, " "));
      error << "} do not have RayKernels defined!";

      mooseError(error.str());
    }
  }
}

void
RayTracingStudy::dependencyChecks()
{
  std::vector<RayTracingObject *> ray_tracing_objects;

  getRayKernels(ray_tracing_objects, 0);
  verifyDependenciesExist(ray_tracing_objects);

  getRayBCs(ray_tracing_objects, 0);
  verifyDependenciesExist(ray_tracing_objects);
}

void
RayTracingStudy::verifyDependenciesExist(const std::vector<RayTracingObject *> & rtos)
{
  for (const auto & rto : rtos)
    for (const auto & dep_name : rto->getRequestedItems())
    {
      bool found = false;
      for (const auto & rto_search : rtos)
        if (rto_search->name() == dep_name)
        {
          found = true;
          break;
        }

      if (!found)
        rto->paramError("depends_on",
                        "The ",
                        rto->parameters().get<std::string>("_moose_base"),
                        " '",
                        dep_name,
                        "' does not exist");
    }
}

void
RayTracingStudy::traceableMeshChecks()
{

  for (const auto & elem : *_mesh.getActiveLocalElementRange())
  {
    if (_fe_problem.adaptivity().isOn())
    {
      if (!TraceRayTools::isAdaptivityTraceableElem(elem))
        mooseError("Element type ",
                   Utility::enum_to_string(elem->type()),
                   " is not supported in ray tracing with adaptivity");
    }
    else if (!TraceRayTools::isTraceableElem(elem))
      mooseError("Element type ",
                 Utility::enum_to_string(elem->type()),
                 " is not supported in ray tracing");
  }
}

void
RayTracingStudy::localElemIndexSetup()
{
  // TODO: We could probably minimize this to local active elements followed by
  // boundary point neighbors, but if using distibuted mesh it really shouldn't matter
  _elem_index_helper.initialize(_mesh.getMesh().active_element_ptr_range());
}

void
RayTracingStudy::internalSidesetSetup()
{
  // Even if we have _use_internal_sidesets == false, we will make sure the user didn't add RayBCs
  // on internal boundaries

  // Clear the data structures and size the map based on the elements that we know about
  _internal_sidesets.clear();
  _internal_sidesets_map.clear();
  _internal_sidesets_map.resize(_elem_index_helper.maxIndex() + 1);

  // First, we are going to store all elements with internal sidesets (if any) that have active
  // RayBCs on them as elem -> vector of (side, vector of boundary ids)
  for (const auto & bnd_elem : *_mesh.getBoundaryElementRange())
  {
    Elem * elem = bnd_elem->_elem;
    const unsigned int side = bnd_elem->_side;
    const auto bnd_id = bnd_elem->_bnd_id;

    // Not internal
    const Elem * const neighbor = elem->neighbor_ptr(side);
    if (!neighbor || neighbor == remote_elem)
      continue;

    // No RayBCs on this sideset
    std::vector<RayBoundaryConditionBase *> result;
    getRayBCs(result, bnd_id, 0);
    if (result.empty())
      continue;

    if (neighbor->subdomain_id() == elem->subdomain_id())
      mooseError("RayBCs exist on internal sidesets that are not bounded by a different",
                 "\nsubdomain on each side.",
                 "\n\nIn order to use RayBCs on internal sidesets, said sidesets must have",
                 "\na different subdomain on each side.");

    // Mark that this boundary is an internal sideset with RayBC(s)
    _internal_sidesets.insert(bnd_id);

    // Get elem's entry in the internal sidset data structure
    const auto index = _elem_index_helper.getIndex(elem);
    auto & entry = _internal_sidesets_map[index];

    // Initialize this elem's sides if they have not been already
    if (entry.empty())
      entry.resize(elem->n_sides(), std::vector<BoundaryID>());

    // Add the internal boundary to the side entry
    entry[side].push_back(bnd_id);
  }

  if (!_use_internal_sidesets && _internal_sidesets.size())
    mooseError("RayBCs are defined on internal sidesets, but the study is not set to use ",
               "internal sidesets during tracing.",
               "\n\nSet the parameter use_internal_sidesets = true to enable this capability.");
}

void
RayTracingStudy::nonPlanarSideSetup()
{
  _has_non_planar_sides = false;
  bool warned = !_warn_non_planar;

  // Nothing to do here for 2D or 1D
  if (_mesh.dimension() != 3)
    return;

  // Clear the data structure and size it based on the elements that we know about
  _non_planar_sides.clear();
  _non_planar_sides.resize(_elem_index_helper.maxIndex() + 1);

  for (const Elem * elem : _mesh.getMesh().active_element_ptr_range())
  {
    const auto index = _elem_index_helper.getIndex(elem);
    auto & entry = _non_planar_sides[index];
    entry.resize(elem->n_sides(), 0);

    for (const auto s : elem->side_index_range())
    {
      const auto & side = elemSide(*elem, s);
      if (side.n_vertices() < 4)
        continue;

      if (!side.has_affine_map())
      {
        entry[s] = 1;
        _has_non_planar_sides = true;

        if (!warned)
        {
          mooseWarning("The mesh contains non-planar faces.\n\n",
                       "Ray tracing on non-planar faces is an approximation and may fail.\n\n",
                       "Use at your own risk! You can disable this warning by setting the\n",
                       "parameter 'warn_non_planar' to false.");
          warned = true;
        }
      }
    }
  }
}

void
RayTracingStudy::subdomainHMaxSetup()
{
  // Setup map with subdomain keys
  _subdomain_hmax.clear();
  for (const auto subdomain_id : _mesh.meshSubdomains())
    _subdomain_hmax[subdomain_id] = std::numeric_limits<Real>::min();

  // Set local max for each subdomain
  for (const auto & elem : *_mesh.getActiveLocalElementRange())
  {
    auto & entry = _subdomain_hmax.at(elem->subdomain_id());
    entry = std::max(entry, elem->hmax());
  }

  // Accumulate global max for each subdomain
  _communicator.max(_subdomain_hmax);

  if (getParam<bool>("warn_subdomain_hmax"))
  {
    const auto warn_prefix = type() + " '" + name() + "': ";
    const auto warn_suffix =
        "\n\nRay tracing uses an approximate element size for each subdomain to scale the\n"
        "tolerances used in computing ray intersections. This warning suggests that the\n"
        "approximate element size is not a good approximation. This is likely due to poor\n"
        "element aspect ratios.\n\n"
        "To disable this warning, set warn_subdomain_hmax = false.\n";

    for (const auto & elem : *_mesh.getActiveLocalElementRange())
    {
      const auto hmin = elem->hmin();
      const auto hmax = elem->hmax();
      const auto max_hmax = subdomainHmax(elem->subdomain_id());

      const auto hmax_rel = hmax / max_hmax;
      if (hmax_rel < 1.e-2 || hmax_rel > 1.e2)
        mooseWarning(
            warn_prefix, "Element hmax varies significantly from subdomain hmax.\n", warn_suffix);

      const auto h_rel = max_hmax / hmin;
      if (h_rel > 1.e2)
        mooseWarning(
            warn_prefix, "Element hmin varies significantly from subdomain hmax.\n", warn_suffix);
    }
  }
}

void
RayTracingStudy::registeredRaySetup()
{
  // First, clear the objects associated with each Ray on each thread
  const auto num_rays = _registered_ray_map.size();
  for (auto & entry : _threaded_ray_object_registration)
  {
    entry.clear();
    entry.resize(num_rays);
  }

  const auto rtos = getRayTracingObjects();

  if (_use_ray_registration)
  {
    // All of the registered ray names - used when a RayTracingObject did not specify
    // any Rays so it should be associated with all Rays.
    std::vector<std::string> all_ray_names;
    all_ray_names.reserve(_registered_ray_map.size());
    for (const auto & pair : _registered_ray_map)
      all_ray_names.push_back(pair.first);

    for (auto & rto : rtos)
    {
      // The Ray names associated with this RayTracingObject
      const auto & ray_names = rto->parameters().get<std::vector<std::string>>("rays");
      // The registration for RayTracingObjects for the thread rto is on
      const auto tid = rto->parameters().get<THREAD_ID>("_tid");
      auto & registration = _threaded_ray_object_registration[tid];

      // Register each Ray for this object in the registration
      for (const auto & ray_name : (ray_names.empty() ? all_ray_names : ray_names))
      {
        const auto id = registeredRayID(ray_name, /* graceful = */ true);
        if (ray_names.size() && id == Ray::INVALID_RAY_ID)
          rto->paramError(
              "rays", "Supplied ray '", ray_name, "' is not a registered Ray in ", typeAndName());
        registration[id].insert(rto);
      }
    }
  }
  // Not using Ray registration
  else
  {
    for (const auto & rto : rtos)
      if (rto->parameters().get<std::vector<std::string>>("rays").size())
        rto->paramError(
            "rays",
            "Rays cannot be supplied when the study does not require Ray registration.\n\n",
            type(),
            " does not require Ray registration.");
  }
}

void
RayTracingStudy::zeroAuxVariables()
{
  std::set<std::string> vars_to_be_zeroed;
  std::vector<RayKernelBase *> ray_kernels;
  getRayKernels(ray_kernels, 0);
  for (auto & rk : ray_kernels)
  {
    AuxRayKernel * aux_rk = dynamic_cast<AuxRayKernel *>(rk);
    if (aux_rk)
      vars_to_be_zeroed.insert(aux_rk->variable().name());
  }

  std::vector<std::string> vars_to_be_zeroed_vec(vars_to_be_zeroed.begin(),
                                                 vars_to_be_zeroed.end());
  _fe_problem.getAuxiliarySystem().zeroVariables(vars_to_be_zeroed_vec);
}

void
RayTracingStudy::segmentSubdomainSetup(const SubdomainID subdomain,
                                       const THREAD_ID tid,
                                       const RayID ray_id)
{
  mooseAssert(currentlyPropagating(), "Should not call while not propagating");

  // Call subdomain setup on FE
  _fe_problem.subdomainSetup(subdomain, tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  std::set<unsigned int> needed_mat_props;

  // Get RayKernels and their dependencies and call subdomain setup
  getRayKernels(_threaded_current_ray_kernels[tid], subdomain, tid, ray_id);
  for (auto & rkb : _threaded_current_ray_kernels[tid])
  {
    rkb->subdomainSetup();

    const auto & mv_deps = rkb->getMooseVariableDependencies();
    needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());

    const auto & mp_deps = rkb->getMatPropDependencies();
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  }

  // Prepare aux vars
  for (auto & var : needed_moose_vars)
    if (var->kind() == Moose::VarKindType::VAR_AUXILIARY)
      var->prepareAux();

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, tid);
  _fe_problem.prepareMaterials(subdomain, tid);
}

void
RayTracingStudy::reinitSegment(
    const Elem * elem, const Point & start, const Point & end, const Real length, THREAD_ID tid)
{
  mooseAssert(MooseUtils::absoluteFuzzyEqual((start - end).norm(), length), "Invalid length");
  mooseAssert(currentlyPropagating(), "Should not call while not propagating");

  _fe_problem.setCurrentSubdomainID(elem, tid);

  // If we have any variables or material properties that are active, we definitely need to reinit
  bool reinit = _fe_problem.hasActiveElementalMooseVariables(tid) ||
                _fe_problem.hasActiveMaterialProperties(tid);
  // If not, make sure that the RayKernels have not requested a reinit (this could happen when a
  // RayKernel doesn't have variables or materials but still does an integration and needs qps)
  if (!reinit)
    for (const RayKernelBase * rk : currentRayKernels(tid))
      if (rk->needSegmentReinit())
      {
        reinit = true;
        break;
      }

  if (reinit)
  {
    _fe_problem.prepare(elem, tid);

    std::vector<Point> points;
    std::vector<Real> weights;
    buildSegmentQuadrature(start, end, length, points, weights);
    _fe_problem.reinitElemPhys(elem, points, tid);
    _fe_problem.assembly(tid).modifyArbitraryWeights(weights);

    _fe_problem.reinitMaterials(elem->subdomain_id(), tid);
  }
}

void
RayTracingStudy::buildSegmentQuadrature(const Point & start,
                                        const Point & end,
                                        const Real length,
                                        std::vector<Point> & points,
                                        std::vector<Real> & weights) const
{
  points.resize(_segment_qrule->n_points());
  weights.resize(_segment_qrule->n_points());

  const Point diff = end - start;
  const Point sum = end + start;
  mooseAssert(MooseUtils::absoluteFuzzyEqual(length, diff.norm()), "Invalid length");

  // The standard quadrature rule should be on x = [-1, 1]
  // To scale the points, you...
  //  - Scale to size of the segment in 3D
  //    initial_scaled_qp = x_qp * 0.5 * (end - start) = 0.5 * x_qp * diff
  //  - Shift quadrature midpoint to segment midpoint
  //    final_qp = initial_scaled_qp + 0.5 * (end - start) = initial_scaled_qp + 0.5 * sum
  //             = 0.5 * (x_qp * diff + sum)
  for (unsigned int qp = 0; qp < _segment_qrule->n_points(); ++qp)
  {
    points[qp] = 0.5 * (_segment_qrule->qp(qp)(0) * diff + sum);
    weights[qp] = 0.5 * _segment_qrule->w(qp) * length;
  }
}

void
RayTracingStudy::postOnSegment(const THREAD_ID tid, const std::shared_ptr<Ray> & /* ray */)
{
  mooseAssert(currentlyPropagating(), "Should not call while not propagating");
  if (!_fe_problem.currentlyComputingJacobian() && !_fe_problem.currentlyComputingResidual())
    mooseAssert(_num_cached[tid] == 0,
                "Values should only be cached when computing Jacobian/residual");

  // Fill into cached Jacobian/residuals if necessary
  if (_fe_problem.currentlyComputingJacobian())
  {
    _fe_problem.cacheJacobian(tid);

    if (++_num_cached[tid] == 20)
    {
      Threads::spin_mutex::scoped_lock lock(_spin_mutex);
      _fe_problem.addCachedJacobian(tid);
      _num_cached[tid] = 0;
    }
  }
  else if (_fe_problem.currentlyComputingResidual())
  {
    _fe_problem.cacheResidual(tid);

    if (++_num_cached[tid] == 20)
    {
      Threads::spin_mutex::scoped_lock lock(_spin_mutex);
      _fe_problem.addCachedResidual(tid);
      _num_cached[tid] = 0;
    }
  }
}

void
RayTracingStudy::executeStudy()
{
  TIME_SECTION("executeStudy", 2, "Executing Study");

  mooseAssert(_called_initial_setup, "Initial setup not called");

  // Reset ray start/complete timers
  _total_intersections = 0;
  _max_intersections = 0;
  _max_trajectory_changes = 0;

  // Reset physical tracing stats
  for (auto & val : _local_trace_ray_results)
    val = 0;

  // Reset crossing and intersection
  _ending_processor_crossings = 0;
  _total_processor_crossings = 0;
  _ending_max_processor_crossings = 0;
  _ending_intersections = 0;
  _ending_max_intersections = 0;
  _ending_max_trajectory_changes = 0;
  _ending_distance = 0;
  _total_distance = 0;

  // Zero the AuxVariables that our AuxRayKernels contribute to before they accumulate
  zeroAuxVariables();

  preExecuteStudy();
  for (auto & rto : getRayTracingObjects())
    rto->preExecuteStudy();

  _ray_bank.clear();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _threaded_trace_ray[tid]->preExecute();
    _threaded_cached_normals[tid].clear();
  }

  _communicator.barrier();
  _execution_start_time = std::chrono::steady_clock::now();

  _parallel_ray_study->preExecute();

  {
    {
      auto generation_start_time = std::chrono::steady_clock::now();

      TIME_SECTION("generateRays", 2, "Generating Rays");

      generateRays();

      _generation_time = std::chrono::steady_clock::now() - generation_start_time;
    }

    // At this point, nobody is working so this is good time to make sure
    // Rays are unique across all processors in the working buffer
    if (verifyRays())
    {
      verifyUniqueRays(_parallel_ray_study->workBuffer().begin(),
                       _parallel_ray_study->workBuffer().end(),
                       /* error_suffix = */ "after generateRays()");

      verifyUniqueRayIDs(_parallel_ray_study->workBuffer().begin(),
                         _parallel_ray_study->workBuffer().end(),
                         /* global = */ true,
                         /* error_suffix = */ "after generateRays()");
    }

    registeredRaySetup();

    {
      TIME_SECTION("propagateRays", 2, "Propagating Rays");

      const auto propagation_start_time = std::chrono::steady_clock::now();

      _parallel_ray_study->execute();

      _propagation_time = std::chrono::steady_clock::now() - propagation_start_time;
    }
  }

  _execution_time = std::chrono::steady_clock::now() - _execution_start_time;

  if (verifyRays())
  {
    verifyUniqueRays(_parallel_ray_study->workBuffer().begin(),
                     _parallel_ray_study->workBuffer().end(),
                     /* error_suffix = */ "after tracing completed");

#ifndef NDEBUG
    // Outside of debug, _ray_bank always holds all of the Rays that have ended on this processor
    // We can use this as a global point to check for unique IDs for every Ray that has traced
    verifyUniqueRayIDs(_ray_bank.begin(),
                       _ray_bank.end(),
                       /* global = */ true,
                       /* error_suffix = */ "after tracing completed");
#endif
  }

  // Update counters from the threaded trace objects
  for (const auto & tr : _threaded_trace_ray)
    for (std::size_t i = 0; i < _local_trace_ray_results.size(); ++i)
      _local_trace_ray_results[i] += tr->results()[i];

  // Update local ending counters
  _total_processor_crossings = _ending_processor_crossings;
  _max_processor_crossings = _ending_max_processor_crossings;
  _total_intersections = _ending_intersections;
  _max_intersections = _ending_max_intersections;
  _max_trajectory_changes = _ending_max_trajectory_changes;
  _total_distance = _ending_distance;
  // ...and communicate the global values
  _communicator.sum(_total_processor_crossings);
  _communicator.max(_max_processor_crossings);
  _communicator.sum(_total_intersections);
  _communicator.max(_max_intersections);
  _communicator.max(_max_trajectory_changes);
  _communicator.sum(_total_distance);

  // Throw a warning with the number of failed (tolerated) traces
  if (_tolerate_failure)
  {
    auto failures = _local_trace_ray_results[TraceRay::FAILED_TRACES];
    _communicator.sum(failures);
    if (failures)
      mooseWarning(
          type(), " '", name(), "': ", failures, " ray tracing failures were tolerated.\n");
  }

  // Clear the current RayKernels
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _threaded_current_ray_kernels[tid].clear();

  // Move the threaded cache trace information into the full cached trace vector
  // Here, we only clear the cached vectors so that we might not have to
  // reallocate on future traces
  std::size_t num_entries = 0;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    num_entries += _threaded_cached_traces[tid].size();
  _cached_traces.clear();
  _cached_traces.reserve(num_entries);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    for (const auto & entry : _threaded_cached_traces[tid])
      _cached_traces.emplace_back(std::move(entry));
    _threaded_cached_traces[tid].clear();
  }

  // Add any stragglers that contribute to the Jacobian or residual
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    if (_num_cached[tid] != 0)
    {
      mooseAssert(_fe_problem.currentlyComputingJacobian() ||
                      _fe_problem.currentlyComputingResidual(),
                  "Should not have cached values without Jacobian/residual computation");

      if (_fe_problem.currentlyComputingJacobian())
        _fe_problem.addCachedJacobian(tid);
      else
        _fe_problem.addCachedResidual(tid);

      _num_cached[tid] = 0;
    }

  // AuxRayKernels may have modified AuxVariables
  _fe_problem.getAuxiliarySystem().solution().close();
  _fe_problem.getAuxiliarySystem().update();

  // Clear FE
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _fe_problem.clearActiveElementalMooseVariables(tid);
    _fe_problem.clearActiveMaterialProperties(tid);
  }

  postExecuteStudy();
  for (auto & rto : getRayTracingObjects())
    rto->postExecuteStudy();
}

void
RayTracingStudy::onCompleteRay(const std::shared_ptr<Ray> & ray)
{
  mooseAssert(currentlyPropagating(), "Should only be called during Ray propagation");

  _ending_processor_crossings += ray->processorCrossings();
  _ending_max_processor_crossings =
      std::max(_ending_max_processor_crossings, ray->processorCrossings());
  _ending_intersections += ray->intersections();
  _ending_max_intersections = std::max(_ending_max_intersections, ray->intersections());
  _ending_max_trajectory_changes =
      std::max(_ending_max_trajectory_changes, ray->trajectoryChanges());
  _ending_distance += ray->distance();

#ifdef NDEBUG
  // In non-opt modes, we will always bank the Rays for debugging
  if (_bank_rays_on_completion)
#endif
    _ray_bank.emplace_back(ray);
}

RayDataIndex
RayTracingStudy::registerRayDataInternal(const std::string & name, const bool aux)
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  if (_called_initial_setup)
    mooseError("Cannot register Ray ", (aux ? "aux " : ""), "data after initialSetup()");

  auto & map = aux ? _ray_aux_data_map : _ray_data_map;
  const auto find = map.find(name);
  if (find != map.end())
    return find->second;

  auto & other_map = aux ? _ray_data_map : _ray_aux_data_map;
  if (other_map.find(name) != other_map.end())
    mooseError("Cannot register Ray aux data with name ",
               name,
               " because Ray ",
               (aux ? "(non-aux)" : "aux"),
               " data already exists with said name.");

  // Add into the name -> index map
  map.emplace(name, map.size());

  // Add into the index -> names vector
  auto & vector = aux ? _ray_aux_data_names : _ray_data_names;
  vector.push_back(name);

  return map.size() - 1;
}

std::vector<RayDataIndex>
RayTracingStudy::registerRayDataInternal(const std::vector<std::string> & names, const bool aux)
{
  std::vector<RayDataIndex> indices(names.size());
  for (std::size_t i = 0; i < names.size(); ++i)
    indices[i] = registerRayDataInternal(names[i], aux);
  return indices;
}

RayDataIndex
RayTracingStudy::getRayDataIndexInternal(const std::string & name,
                                         const bool aux,
                                         const bool graceful) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  const auto & map = aux ? _ray_aux_data_map : _ray_data_map;
  const auto find = map.find(name);
  if (find != map.end())
    return find->second;

  if (graceful)
    return Ray::INVALID_RAY_DATA_INDEX;

  const auto & other_map = aux ? _ray_data_map : _ray_aux_data_map;
  if (other_map.find(name) != other_map.end())
    mooseError("Ray data with name '",
               name,
               "' was not found.\n\n",
               "However, Ray ",
               (aux ? "non-aux" : "aux"),
               " data with said name was found.\n",
               "Did you mean to use ",
               (aux ? "getRayDataIndex()/getRayDataIndices()?"
                    : "getRayAuxDataIndex()/getRayAuxDataIndices()"),
               "?");

  mooseError("Unknown Ray ", (aux ? "aux " : ""), "data with name ", name);
}

std::vector<RayDataIndex>
RayTracingStudy::getRayDataIndicesInternal(const std::vector<std::string> & names,
                                           const bool aux,
                                           const bool graceful) const
{
  std::vector<RayDataIndex> indices(names.size());
  for (std::size_t i = 0; i < names.size(); ++i)
    indices[i] = getRayDataIndexInternal(names[i], aux, graceful);
  return indices;
}

const std::string &
RayTracingStudy::getRayDataNameInternal(const RayDataIndex index, const bool aux) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  if ((aux ? rayAuxDataSize() : rayDataSize()) < index)
    mooseError("Unknown Ray ", aux ? "aux " : "", "data with index ", index);
  return aux ? _ray_aux_data_names[index] : _ray_data_names[index];
}

RayDataIndex
RayTracingStudy::registerRayData(const std::string & name)
{
  return registerRayDataInternal(name, /* aux = */ false);
}

std::vector<RayDataIndex>
RayTracingStudy::registerRayData(const std::vector<std::string> & names)
{
  return registerRayDataInternal(names, /* aux = */ false);
}

RayDataIndex
RayTracingStudy::getRayDataIndex(const std::string & name, const bool graceful /* = false */) const
{
  return getRayDataIndexInternal(name, /* aux = */ false, graceful);
}

std::vector<RayDataIndex>
RayTracingStudy::getRayDataIndices(const std::vector<std::string> & names,
                                   const bool graceful /* = false */) const
{
  return getRayDataIndicesInternal(names, /* aux = */ false, graceful);
}

const std::string &
RayTracingStudy::getRayDataName(const RayDataIndex index) const
{
  return getRayDataNameInternal(index, /* aux = */ false);
}

RayDataIndex
RayTracingStudy::registerRayAuxData(const std::string & name)
{
  return registerRayDataInternal(name, /* aux = */ true);
}

std::vector<RayDataIndex>
RayTracingStudy::registerRayAuxData(const std::vector<std::string> & names)
{
  return registerRayDataInternal(names, /* aux = */ true);
}

RayDataIndex
RayTracingStudy::getRayAuxDataIndex(const std::string & name,
                                    const bool graceful /* = false */) const
{
  return getRayDataIndexInternal(name, /* aux = */ true, graceful);
}

std::vector<RayDataIndex>
RayTracingStudy::getRayAuxDataIndices(const std::vector<std::string> & names,
                                      const bool graceful /* = false */) const
{
  return getRayDataIndicesInternal(names, /* aux = */ true, graceful);
}

const std::string &
RayTracingStudy::getRayAuxDataName(const RayDataIndex index) const
{
  return getRayDataNameInternal(index, /* aux = */ true);
}

bool
RayTracingStudy::hasRayKernels(const THREAD_ID tid)
{
  std::vector<RayKernelBase *> result;
  getRayKernels(result, tid);
  return result.size();
}

void
RayTracingStudy::getRayKernels(std::vector<RayKernelBase *> & result, SubdomainID id, THREAD_ID tid)
{
  // If the cache doesn't have any attributes yet, it means that we haven't set
  // the conditions yet. We do this so that it can be generated on the fly on first use.
  if (!_threaded_cache_ray_kernel[tid].numAttribs())
  {
    if (!_called_initial_setup)
      mooseError("Should not call getRayKernels() before initialSetup()");

    auto query = _fe_problem.theWarehouse()
                     .query()
                     .condition<AttribRayTracingStudy>(this)
                     .condition<AttribSystem>("RayKernel")
                     .condition<AttribThread>(tid);
    _threaded_cache_ray_kernel[tid] = query.clone();
  }

  _threaded_cache_ray_kernel[tid].queryInto(result, id);
}

void
RayTracingStudy::getRayKernels(std::vector<RayKernelBase *> & result,
                               SubdomainID id,
                               THREAD_ID tid,
                               RayID ray_id)
{
  // No Ray registration: no need to sift through objects
  if (!_use_ray_registration)
  {
    getRayKernels(result, id, tid);
  }
  // Has Ray registration: only pick the objects associated with ray_id
  else
  {
    // Get all of the kernels on this block
    std::vector<RayKernelBase *> rkbs;
    getRayKernels(rkbs, id, tid);

    // The RayTracingObjects associated with this ray
    const auto & ray_id_rtos = _threaded_ray_object_registration[tid][ray_id];

    // The result is the union of all of the kernels and the objects associated with this Ray
    result.clear();
    for (auto rkb : rkbs)
      if (ray_id_rtos.count(rkb))
        result.push_back(rkb);
  }
}

void
RayTracingStudy::getRayBCs(std::vector<RayBoundaryConditionBase *> & result,
                           BoundaryID id,
                           THREAD_ID tid)
{
  // If the cache doesn't have any attributes yet, it means that we haven't set
  // the conditions yet. We do this so that it can be generated on the fly on first use.
  if (!_threaded_cache_ray_bc[tid].numAttribs())
  {
    if (!_called_initial_setup)
      mooseError("Should not call getRayBCs() before initialSetup()");

    auto query = _fe_problem.theWarehouse()
                     .query()
                     .condition<AttribRayTracingStudy>(this)
                     .condition<AttribSystem>("RayBoundaryCondition")
                     .condition<AttribThread>(tid);
    _threaded_cache_ray_bc[tid] = query.clone();
  }

  _threaded_cache_ray_bc[tid].queryInto(result, std::make_tuple(id, false));
}

void
RayTracingStudy::getRayBCs(std::vector<RayBoundaryConditionBase *> & result,
                           const std::vector<TraceRayBndElement> & bnd_elems,
                           THREAD_ID tid,
                           RayID ray_id)
{
  // No Ray registration: no need to sift through objects
  if (!_use_ray_registration)
  {
    if (bnd_elems.size() == 1)
      getRayBCs(result, bnd_elems[0].bnd_id, tid);
    else
    {
      std::vector<BoundaryID> bnd_ids(bnd_elems.size());
      for (MooseIndex(bnd_elems.size()) i = 0; i < bnd_elems.size(); ++i)
        bnd_ids[i] = bnd_elems[i].bnd_id;
      getRayBCs(result, bnd_ids, tid);
    }
  }
  // Has Ray registration: only pick the objects associated with ray_id
  else
  {
    // Get all of the RayBCs on these boundaries
    std::vector<RayBoundaryConditionBase *> rbcs;
    if (bnd_elems.size() == 1)
      getRayBCs(rbcs, bnd_elems[0].bnd_id, tid);
    else
    {
      std::vector<BoundaryID> bnd_ids(bnd_elems.size());
      for (MooseIndex(bnd_elems.size()) i = 0; i < bnd_elems.size(); ++i)
        bnd_ids[i] = bnd_elems[i].bnd_id;
      getRayBCs(rbcs, bnd_ids, tid);
    }

    // The RayTracingObjects associated with this ray
    mooseAssert(ray_id < _threaded_ray_object_registration[tid].size(), "Not in registration");
    const auto & ray_id_rtos = _threaded_ray_object_registration[tid][ray_id];

    // The result is the union of all of the kernels and the objects associated with this Ray
    result.clear();
    for (auto rbc : rbcs)
      if (ray_id_rtos.count(rbc))
        result.push_back(rbc);
  }
}

std::vector<RayTracingObject *>
RayTracingStudy::getRayTracingObjects()
{
  std::vector<RayTracingObject *> result;
  _fe_problem.theWarehouse().query().condition<AttribRayTracingStudy>(this).queryInto(result);
  return result;
}

const std::vector<std::shared_ptr<Ray>> &
RayTracingStudy::rayBank() const
{
  if (!_bank_rays_on_completion)
    mooseError("The Ray bank is not available because the private parameter "
               "'_bank_rays_on_completion' is set to false.");
  if (currentlyGenerating() || currentlyPropagating())
    mooseError("Cannot get the Ray bank during generation or propagation.");

  return _ray_bank;
}

std::shared_ptr<Ray>
RayTracingStudy::getBankedRay(const RayID ray_id) const
{
  // This is only a linear search - can be improved on with a map in the future
  // if this is used on a larger scale
  std::shared_ptr<Ray> ray;
  for (const std::shared_ptr<Ray> & possible_ray : rayBank())
    if (possible_ray->id() == ray_id)
    {
      ray = possible_ray;
      break;
    }

  // Make sure one and only one processor has the Ray
  unsigned int have_ray = ray ? 1 : 0;
  _communicator.sum(have_ray);
  if (have_ray == 0)
    mooseError("Could not find a Ray with the ID ", ray_id, " in the Ray banks.");

  // This should never happen... but let's make sure
  mooseAssert(have_ray == 1, "Multiple rays with the same ID were found in the Ray banks");

  return ray;
}

RayData
RayTracingStudy::getBankedRayDataInternal(const RayID ray_id,
                                          const RayDataIndex index,
                                          const bool aux) const
{
  // Will be a nullptr shared_ptr if this processor doesn't own the Ray
  const std::shared_ptr<Ray> ray = getBankedRay(ray_id);

  Real value = ray ? (aux ? ray->auxData(index) : ray->data(index)) : 0;
  _communicator.sum(value);
  return value;
}

RayData
RayTracingStudy::getBankedRayData(const RayID ray_id, const RayDataIndex index) const
{
  return getBankedRayDataInternal(ray_id, index, /* aux = */ false);
}

RayData
RayTracingStudy::getBankedRayAuxData(const RayID ray_id, const RayDataIndex index) const
{
  return getBankedRayDataInternal(ray_id, index, /* aux = */ true);
}

RayID
RayTracingStudy::registerRay(const std::string & name)
{
  libmesh_parallel_only(comm());

  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  if (!_use_ray_registration)
    mooseError("Cannot use registerRay() with Ray registration disabled");

  // This is parallel only for now. We could likely stagger the ID building like we do with
  // the unique IDs, but it would require a sync point which isn't there right now
  libmesh_parallel_only(comm());

  const auto & it = _registered_ray_map.find(name);
  if (it != _registered_ray_map.end())
    return it->second;

  const auto id = _reverse_registered_ray_map.size();
  _registered_ray_map.emplace(name, id);
  _reverse_registered_ray_map.push_back(name);
  return id;
}

RayID
RayTracingStudy::registeredRayID(const std::string & name, const bool graceful /* = false */) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  if (!_use_ray_registration)
    mooseError("Should not use registeredRayID() with Ray registration disabled");

  const auto search = _registered_ray_map.find(name);
  if (search != _registered_ray_map.end())
    return search->second;

  if (graceful)
    return Ray::INVALID_RAY_ID;

  mooseError("Attempted to obtain ID of registered Ray ",
             name,
             ", but a Ray with said name is not registered.");
}

const std::string &
RayTracingStudy::registeredRayName(const RayID ray_id) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  if (!_use_ray_registration)
    mooseError("Should not use registeredRayName() with Ray registration disabled");

  if (_reverse_registered_ray_map.size() > ray_id)
    return _reverse_registered_ray_map[ray_id];

  mooseError("Attempted to obtain name of registered Ray with ID ",
             ray_id,
             ", but a Ray with said ID is not registered.");
}

Real
RayTracingStudy::computeTotalVolume()
{
  Real volume = 0;
  for (const auto & elem : *_mesh.getActiveLocalElementRange())
    volume += elem->volume();
  _communicator.sum(volume);
  return volume;
}

const std::vector<std::vector<BoundaryID>> &
RayTracingStudy::getInternalSidesets(const Elem * elem) const
{
  mooseAssert(_use_internal_sidesets, "Not using internal sidesets");
  mooseAssert(hasInternalSidesets(), "Processor does not have internal sidesets");
  mooseAssert(_internal_sidesets_map.size() > _elem_index_helper.getIndex(elem),
              "Internal sideset map not initialized");

  const auto index = _elem_index_helper.getIndex(elem);
  return _internal_sidesets_map[index];
}

TraceData &
RayTracingStudy::initThreadedCachedTrace(const std::shared_ptr<Ray> & ray, THREAD_ID tid)
{
  mooseAssert(shouldCacheTrace(ray), "Not caching trace");
  mooseAssert(currentlyPropagating(), "Should only use while tracing");

  _threaded_cached_traces[tid].emplace_back(ray);
  return _threaded_cached_traces[tid].back();
}

void
RayTracingStudy::verifyUniqueRayIDs(const std::vector<std::shared_ptr<Ray>>::const_iterator begin,
                                    const std::vector<std::shared_ptr<Ray>>::const_iterator end,
                                    const bool global,
                                    const std::string & error_suffix) const
{
  // Determine the unique set of Ray IDs on this processor,
  // and if not locally unique throw an error. Once we build this set,
  // we will send it to rank 0 to verify globally
  std::set<RayID> local_rays;
  for (const std::shared_ptr<Ray> & ray : as_range(begin, end))
  {
    mooseAssert(ray, "Null ray");

    // Try to insert into the set; the second entry in the pair
    // will be false if it was not inserted
    if (!local_rays.insert(ray->id()).second)
    {
      for (const std::shared_ptr<Ray> & other_ray : as_range(begin, end))
        if (ray.get() != other_ray.get() && ray->id() == other_ray->id())
          mooseError("Multiple Rays exist with ID ",
                     ray->id(),
                     " on processor ",
                     _pid,
                     " ",
                     error_suffix,
                     "\n\nOffending Ray information:\n\n",
                     ray->getInfo(),
                     "\n",
                     other_ray->getInfo());
    }
  }

  // Send IDs from all procs to rank 0 and verify on rank 0
  if (global)
  {
    // Package our local IDs and send to rank 0
    std::map<processor_id_type, std::vector<RayID>> send_ids;
    if (local_rays.size())
      send_ids.emplace(std::piecewise_construct,
                       std::forward_as_tuple(0),
                       std::forward_as_tuple(local_rays.begin(), local_rays.end()));
    local_rays.clear();

    // Mapping on rank 0 from ID -> processor ID
    std::map<RayID, processor_id_type> global_map;

    // Verify another processor's IDs against the global map on rank 0
    const auto check_ids =
        [this, &global_map, &error_suffix](processor_id_type pid, const std::vector<RayID> & ids)
    {
      for (const RayID id : ids)
      {
        const auto emplace_pair = global_map.emplace(id, pid);

        // Means that this ID already exists in the map
        if (!emplace_pair.second)
          mooseError("Ray with ID ",
                     id,
                     " exists on ranks ",
                     emplace_pair.first->second,
                     " and ",
                     pid,
                     "\n",
                     error_suffix);
      }
    };

    Parallel::push_parallel_vector_data(_communicator, send_ids, check_ids);
  }
}

void
RayTracingStudy::verifyUniqueRays(const std::vector<std::shared_ptr<Ray>>::const_iterator begin,
                                  const std::vector<std::shared_ptr<Ray>>::const_iterator end,
                                  const std::string & error_suffix)
{
  std::set<const Ray *> rays;
  for (const std::shared_ptr<Ray> & ray : as_range(begin, end))
    if (!rays.insert(ray.get()).second) // false if not inserted into rays
      mooseError("Multiple shared_ptrs were found that point to the same Ray ",
                 error_suffix,
                 "\n\nOffending Ray:\n",
                 ray->getInfo());
}

void
RayTracingStudy::moveRayToBuffer(std::shared_ptr<Ray> & ray)
{
  mooseAssert(currentlyGenerating(), "Can only use while generating");
  mooseAssert(ray, "Null ray");
  mooseAssert(ray->shouldContinue(), "Ray is not continuing");

  _parallel_ray_study->moveWorkToBuffer(ray, /* tid = */ 0);
}

void
RayTracingStudy::moveRaysToBuffer(std::vector<std::shared_ptr<Ray>> & rays)
{
  mooseAssert(currentlyGenerating(), "Can only use while generating");
#ifndef NDEBUG
  for (const std::shared_ptr<Ray> & ray : rays)
  {
    mooseAssert(ray, "Null ray");
    mooseAssert(ray->shouldContinue(), "Ray is not continuing");
  }
#endif

  _parallel_ray_study->moveWorkToBuffer(rays, /* tid = */ 0);
}

void
RayTracingStudy::moveRayToBufferDuringTrace(std::shared_ptr<Ray> & ray,
                                            const THREAD_ID tid,
                                            const AcquireMoveDuringTraceKey &)
{
  mooseAssert(ray, "Null ray");
  mooseAssert(currentlyPropagating(), "Can only use while tracing");

  _parallel_ray_study->moveWorkToBuffer(ray, tid);
}

void
RayTracingStudy::reserveRayBuffer(const std::size_t size)
{
  if (!currentlyGenerating())
    mooseError("Can only reserve in Ray buffer during generateRays()");

  _parallel_ray_study->reserveBuffer(size);
}

const Point &
RayTracingStudy::getSideNormal(const Elem * elem, unsigned short side, const THREAD_ID tid)
{
  std::unordered_map<std::pair<const Elem *, unsigned short>, Point> & cache =
      _threaded_cached_normals[tid];

  // See if we've already cached this side normal
  const auto elem_side_pair = std::make_pair(elem, side);
  const auto search = cache.find(elem_side_pair);

  // Haven't cached this side normal: compute it and then cache it
  if (search == cache.end())
  {
    _threaded_fe_face[tid]->reinit(elem, side);
    const auto & normal = _threaded_fe_face[tid]->get_normals()[0];
    cache.emplace(elem_side_pair, normal);
    return normal;
  }

  // Have cached this side normal: simply return it
  return search->second;
}

bool
RayTracingStudy::sameLevelActiveElems() const
{
  unsigned int min_level = std::numeric_limits<unsigned int>::max();
  unsigned int max_level = std::numeric_limits<unsigned int>::min();

  for (const auto & elem : *_mesh.getActiveLocalElementRange())
  {
    const auto level = elem->level();
    min_level = std::min(level, min_level);
    max_level = std::max(level, max_level);
  }

  _communicator.min(min_level);
  _communicator.max(max_level);

  return min_level == max_level;
}

Real
RayTracingStudy::subdomainHmax(const SubdomainID subdomain_id) const
{
  const auto find = _subdomain_hmax.find(subdomain_id);
  if (find == _subdomain_hmax.end())
    mooseError("Subdomain ", subdomain_id, " not found in subdomain hmax map");
  return find->second;
}

bool
RayTracingStudy::isRectangularDomain() const
{
  Real bbox_volume = 1;
  for (unsigned int d = 0; d < _mesh.dimension(); ++d)
    bbox_volume *= std::abs(_b_box.max()(d) - _b_box.min()(d));

  return MooseUtils::absoluteFuzzyEqual(bbox_volume, totalVolume(), TOLERANCE);
}

void
RayTracingStudy::resetUniqueRayIDs()
{
  libmesh_parallel_only(comm());

  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  mooseAssert(!currentlyGenerating() && !currentlyPropagating(),
              "Cannot be reset during generation or propagation");

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    _threaded_next_ray_id[tid] = (RayID)_pid * (RayID)libMesh::n_threads() + (RayID)tid;
}

RayID
RayTracingStudy::generateUniqueRayID(const THREAD_ID tid)
{
  // Get the current ID to return
  const auto id = _threaded_next_ray_id[tid];

  // Advance so that the next call has the correct ID
  _threaded_next_ray_id[tid] += (RayID)n_processors() * (RayID)libMesh::n_threads();

  return id;
}

void
RayTracingStudy::resetReplicatedRayIDs()
{
  libmesh_parallel_only(comm());

  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  mooseAssert(!currentlyGenerating() && !currentlyPropagating(),
              "Cannot be reset during generation or propagation");

  _replicated_next_ray_id = 0;
}

RayID
RayTracingStudy::generateReplicatedRayID()
{
  return _replicated_next_ray_id++;
}

bool
RayTracingStudy::sideIsIncoming(const Elem * const elem,
                                const unsigned short side,
                                const Point & direction,
                                const THREAD_ID tid)
{
  const auto & normal = getSideNormal(elem, side, tid);
  const auto dot = normal * direction;
  return dot < TraceRayTools::TRACE_TOLERANCE;
}

std::shared_ptr<Ray>
RayTracingStudy::acquireRay()
{
  mooseAssert(currentlyGenerating(), "Can only use during generateRays()");

  return _parallel_ray_study->acquireParallelData(
      /* tid = */ 0,
      this,
      generateUniqueRayID(/* tid = */ 0),
      rayDataSize(),
      rayAuxDataSize(),
      /* reset = */ true,
      Ray::ConstructRayKey());
}

std::shared_ptr<Ray>
RayTracingStudy::acquireUnsizedRay()
{
  mooseAssert(currentlyGenerating(), "Can only use during generateRays()");

  return _parallel_ray_study->acquireParallelData(/* tid = */ 0,
                                                  this,
                                                  generateUniqueRayID(/* tid = */ 0),
                                                  /* data_size = */ 0,
                                                  /* aux_data_size = */ 0,
                                                  /* reset = */ true,
                                                  Ray::ConstructRayKey());
}

std::shared_ptr<Ray>
RayTracingStudy::acquireReplicatedRay()
{
  mooseAssert(currentlyGenerating(), "Can only use during generateRays()");
  libmesh_parallel_only(comm());

  return _parallel_ray_study->acquireParallelData(
      /* tid = */ 0,
      this,
      generateReplicatedRayID(),
      rayDataSize(),
      rayAuxDataSize(),
      /* reset = */ true,
      Ray::ConstructRayKey());
}

std::shared_ptr<Ray>
RayTracingStudy::acquireRegisteredRay(const std::string & name)
{
  mooseAssert(currentlyGenerating(), "Can only use during generateRays()");

  // Either register a Ray or get an already registered Ray id
  const RayID id = registerRay(name);

  // Acquire a Ray with the properly sized data initialized to zero
  return _parallel_ray_study->acquireParallelData(
      /* tid = */ 0,
      this,
      id,
      rayDataSize(),
      rayAuxDataSize(),
      /* reset = */ true,
      Ray::ConstructRayKey());
}

std::shared_ptr<Ray>
RayTracingStudy::acquireCopiedRay(const Ray & ray)
{
  mooseAssert(currentlyGenerating(), "Can only use during generateRays()");
  return _parallel_ray_study->acquireParallelData(
      /* tid = */ 0, &ray, Ray::ConstructRayKey());
}

std::shared_ptr<Ray>
RayTracingStudy::acquireRayDuringTrace(const THREAD_ID tid, const AcquireMoveDuringTraceKey &)
{
  mooseAssert(currentlyPropagating(), "Can only use during propagation");
  return _parallel_ray_study->acquireParallelData(tid,
                                                  this,
                                                  generateUniqueRayID(tid),
                                                  rayDataSize(),
                                                  rayAuxDataSize(),
                                                  /* reset = */ true,
                                                  Ray::ConstructRayKey());
}
