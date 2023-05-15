//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "Positions.h"
#include "MultiAppPositions.h" // remove after use_nearest_app deprecation

// libmesh includes
#include "libmesh/point_locator_base.h"
#include "libmesh/enum_point_locator_type.h"

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

namespace GeneralFieldTransfer
{
Number BetterOutOfMeshValue = std::numeric_limits<Real>::infinity();
}

InputParameters
MultiAppGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  // Expansion default to make a node in the target mesh overlapping with a node in the origin
  // mesh always register as being inside the origin application bounding box. The contains_point
  // bounding box checks uses exact comparisons
  params.addRangeCheckedParam<Real>("bbox_factor",
                                    1.00000001,
                                    "bbox_factor>0",
                                    "Factor to inflate or deflate the source app bounding boxes");
  params.addRangeCheckedParam<std::vector<Real>>(
      "fixed_bounding_box_size",
      "fixed_bounding_box_size >= 0",
      "Override source app bounding box size(s) for searches. App bounding boxes will still be  "
      "centered on the same coordinates. Only non-zero components passed will override.");
  params.addParam<Real>(
      "extrapolation_constant",
      0,
      "Constant to use when no source app can provide a valid value for a target location.");

  // Block restrictions
  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "Subdomain restriction to transfer from (defaults to all the origin app domain)");
  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "Subdomain restriction to transfer to, (defaults to all the target app domain)");

  // Boundary restrictions
  params.addParam<std::vector<BoundaryName>>(
      "from_boundaries",
      "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "to_boundaries",
      "The boundary we are transferring to (if not specified, whole domain is used).");
  MooseEnum nodes_or_sides("nodes sides", "sides");
  params.addParam<MooseEnum>("elemental_boundary_restriction",
                             nodes_or_sides,
                             "Whether elemental variable boundary restriction is considered by "
                             "element side or element nodes");

  // Array and vector variables
  params.addParam<std::vector<unsigned int>>("source_variable_components",
                                             std::vector<unsigned int>(),
                                             "The source array or vector variable component(s).");
  params.addParam<std::vector<unsigned int>>("target_variable_components",
                                             std::vector<unsigned int>(),
                                             "The target array or vector variable component(s).");

  // Search options
  params.addParam<bool>(
      "greedy_search",
      false,
      "Whether or not to send a point to all the domains. If true, all the processors will be "
      "checked for a given point."
      "The code will be slow if this flag is on but it will give a better solution.");
  params.addParam<bool>(
      "error_on_miss",
      false,
      "Whether or not to error in the case that a target point is not found in the source domain.");
  params.addParam<bool>(
      "use_nearest_app",
      false,
      "When True, transfers from a child application will work by finding the nearest (using "
      "the `position` + mesh centroid) sub-app and query that app for the value to transfer.");
  params.addParam<PositionsName>(
      "use_nearest_position",
      "Name of the the Positions object (in main app) such that transfers to/from a child "
      "application will work by finding the nearest position to a target and query only the "
      "app / points closer to this position than any other position for the value to transfer.");
  params.addParam<bool>(
      "from_app_must_contain_point",
      false,
      "Wether on not the origin mesh must contain the point to evaluate data at. If false, this "
      "allows for interpolation between origin app meshes. Origin app bounding boxes are still "
      "considered so you may want to increase them with 'fixed_bounding_box_size'");
  params.addParam<bool>("search_value_conflicts",
                        false,
                        "Whether to look for potential conflicts between two valid and different "
                        "source values for any target point");

  params.addParamNamesToGroup(
      "to_blocks from_blocks to_boundaries from_boundaries elemental_boundary_restriction",
      "Transfer spatial restriction");
  params.addParamNamesToGroup("greedy_search use_nearest_app use_nearest_position "
                              "search_value_conflicts",
                              "Search algorithm");
  params.addParamNamesToGroup("error_on_miss from_app_must_contain_point extrapolation_constant",
                              "Extrapolation behavior");
  params.addParamNamesToGroup("bbox_factor fixed_bounding_box_size", "Source app bounding box");
  return params;
}

MultiAppGeneralFieldTransfer::MultiAppGeneralFieldTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _from_var_components(getParam<std::vector<unsigned int>>("source_variable_components")),
    _to_var_components(getParam<std::vector<unsigned int>>("target_variable_components")),
    _use_nearest_app(getParam<bool>("use_nearest_app")),
    _nearest_positions_obj(
        isParamValid("use_nearest_position")
            ? &_fe_problem.getPositionsObject(getParam<PositionsName>("use_nearest_position"))
            : nullptr),
    _source_app_must_contain_point(getParam<bool>("from_app_must_contain_point")),
    _elemental_boundary_restriction_on_sides(
        getParam<MooseEnum>("elemental_boundary_restriction") == "sides"),
    _greedy_search(getParam<bool>("greedy_search")),
    _search_value_conflicts(getParam<bool>("search_value_conflicts")),
    _error_on_miss(getParam<bool>("error_on_miss")),
    _default_extrapolation_value(getParam<Real>("extrapolation_constant")),
    _bbox_factor(getParam<Real>("bbox_factor")),
    _fixed_bbox_size(isParamValid("fixed_bounding_box_size")
                         ? getParam<std::vector<Real>>("fixed_bounding_box_size")
                         : std::vector<Real>(3, 0))
{
  _var_size = _to_var_names.size();
  if (_to_var_names.size() != _from_var_names.size() && !parameters.isPrivate("source_variable"))
    paramError("variable", "The number of variables to transfer to and from should be equal");

  // Check the parameters of the components of the array / vector variable
  if (_from_var_names.size() != _from_var_components.size() && _from_var_components.size() > 0)
    paramError("source_variable_components",
               "This parameter must be equal to the number of source variables");
  if (_to_var_names.size() != _to_var_components.size() && _to_var_components.size() > 0)
    paramError("target_variable_components",
               "This parameter must be equal to the number of target variables");

  // Make simple 'use_nearest_app' parameter rely on Positions
  if (_use_nearest_app)
  {
    if (_nearest_positions_obj)
      paramError("use_nearest_app", "Cannot use nearest-app and nearest-position together");
    if (!hasFromMultiApp())
      mooseError("Should have a source multiapp when using the nearest-app informed search");
    auto pos_params = MultiAppPositions::validParams();
    pos_params.set<std::vector<MultiAppName>>("multiapps") = {getMultiApp()->name()};
    pos_params.set<MooseApp *>("_moose_app") =
        parameters.getCheckedPointerParam<MooseApp *>("_moose_app");
    _fe_problem.addReporter("MultiAppPositions", "_created_for_" + name(), pos_params);
    _nearest_positions_obj = &_fe_problem.getPositionsObject("_created_for_" + name());
  }

  // Dont let users get wrecked by bounding boxes if it looks like they are trying to extrapolate
  if (!_source_app_must_contain_point &&
      (_nearest_positions_obj || isParamSetByUser("from_app_must_contain_point")))
    if (!isParamSetByUser("bbox_factor") && !isParamSetByUser("fixed_bounding_box_size"))
      mooseWarning(
          "Extrapolation (nearest-source options, outside-app source) parameters have been passed, "
          "but no subapp bounding box expansion parameters have been passed.");
}

void
MultiAppGeneralFieldTransfer::initialSetup()
{
  MultiAppConservativeTransfer::initialSetup();

  // Use IDs for block and boundary restriction
  // Loop over all source problems
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    const auto & from_moose_mesh = _from_problems[i_from]->mesh(_displaced_source_mesh);
    if (isParamValid("from_blocks"))
    {
      auto & blocks = getParam<std::vector<SubdomainName>>("from_blocks");
      std::vector<SubdomainID> ids = from_moose_mesh.getSubdomainIDs(blocks);
      _from_blocks.insert(ids.begin(), ids.end());
      if (_from_blocks.size() != blocks.size())
        paramError("from_blocks", "Some blocks were not found in the mesh");
    }

    if (isParamValid("from_boundaries"))
    {
      auto & boundary_names = getParam<std::vector<BoundaryName>>("from_boundaries");
      std::vector<BoundaryID> boundary_ids = from_moose_mesh.getBoundaryIDs(boundary_names);
      _from_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
      if (_from_boundaries.size() != boundary_names.size())
        paramError("from_boundaries", "Some boundaries were not found in the mesh");
    }
  }

  // Loop over all target problems
  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    const auto & to_moose_mesh = _to_problems[i_to]->mesh(_displaced_target_mesh);
    if (isParamValid("to_blocks"))
    {
      auto & blocks = getParam<std::vector<SubdomainName>>("to_blocks");
      std::vector<SubdomainID> ids = to_moose_mesh.getSubdomainIDs(blocks);
      _to_blocks.insert(ids.begin(), ids.end());
      if (_to_blocks.size() != blocks.size())
        paramError("to_blocks", "Some blocks were not found in the mesh");
    }

    if (isParamValid("to_boundaries"))
    {
      auto & boundary_names = getParam<std::vector<BoundaryName>>("to_boundaries");
      std::vector<BoundaryID> boundary_ids = to_moose_mesh.getBoundaryIDs(boundary_names);
      _to_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
      if (_to_boundaries.size() != boundary_names.size())
        paramError("to_boundaries", "Some boundaries were not found in the mesh");
    }
  }

  // Check if components are set correctly if using an array variable
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    for (const auto var_index : make_range(_from_var_names.size()))
    {
      MooseVariableFieldBase & from_var =
          _from_problems[i_from]->getVariable(0,
                                              _from_var_names[var_index],
                                              Moose::VarKindType::VAR_ANY,
                                              Moose::VarFieldType::VAR_FIELD_ANY);
      if (from_var.count() > 1 && _from_var_components.empty())
        paramError("source_variable_components", "Component must be passed for an array variable");
      if (_from_var_components.size() && from_var.count() < _from_var_components[var_index])
        paramError("source_variable_components",
                   "Component passed is larger than size of variable");
    }
  }
  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    for (const auto var_index : make_range(_to_var_names.size()))
    {
      MooseVariableFieldBase & to_var =
          _to_problems[i_to]->getVariable(0,
                                          _to_var_names[var_index],
                                          Moose::VarKindType::VAR_ANY,
                                          Moose::VarFieldType::VAR_FIELD_ANY);
      if (to_var.count() > 1 && _to_var_components.empty())
        paramError("target_variable_components", "Component must be passed for an array variable");
      if (_to_var_components.size() && to_var.count() < _to_var_components[var_index])
        paramError("target_variable_components",
                   "Component passed is larger than size of variable");
    }
  }

  // Cache some quantities to avoid having to get them on every transferred point
  if (_to_problems.size())
  {
    _to_variables.resize(_to_var_names.size());
    for (unsigned int i_to = 0; i_to < _to_var_names.size(); ++i_to)
      _to_variables[i_to] = &_to_problems[0]->getVariable(
          0, _to_var_names[i_to], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  }
}

void
MultiAppGeneralFieldTransfer::getAppInfo()
{
  MultiAppFieldTransfer::getAppInfo();

  // Create the point locators to locate evaluation points in the origin mesh(es)
  _from_point_locators.resize(_from_problems.size());
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    const auto & from_moose_mesh = _from_problems[i_from]->mesh(_displaced_source_mesh);
    _from_point_locators[i_from] =
        PointLocatorBase::build(TREE_LOCAL_ELEMENTS, from_moose_mesh.getMesh());
    _from_point_locators[i_from]->enable_out_of_mesh_mode();
  }
}

void
MultiAppGeneralFieldTransfer::execute()
{
  getAppInfo();

  // loop over the vector of variables and make the transfer one by one
  for (unsigned int i = 0; i < _var_size; ++i)
    transferVariable(i);

  postExecute();
}

void
MultiAppGeneralFieldTransfer::transferVariable(unsigned int i)
{
  mooseAssert(i < _var_size, "The variable of index " << i << " does not exist");

  // Get the bounding boxes for the "from" domains.
  // Clean up _bboxes
  _bboxes.clear();

  // NOTE: This ignores the app's bounding box inflation and padding
  _bboxes = getRestrictedFromBoundingBoxes();

  // Expand bounding boxes. Some desired points might be excluded
  // without an expansion
  extendBoundingBoxes(_bbox_factor, _bboxes);

  // Figure out how many "from" domains each processor owns.
  _froms_per_proc.clear();
  _froms_per_proc = getFromsPerProc();

  // Find outgoing target points
  // We need to know what points we need to send which processors
  // One processor will receive many points from many processors
  // One point may go to different processors
  ProcessorToPointVec outgoing_points;
  extractOutgoingPoints(i, outgoing_points);

  if (_from_var_names.size())
    prepareEvaluationOfInterpValues(i);
  else
    prepareEvaluationOfInterpValues(-1);

  // Fill values and app ids for incoming points
  // We are responsible to compute values for these incoming points
  auto gather_functor = [this](processor_id_type /*pid*/,
                               const std::vector<Point> & incoming_points,
                               std::vector<std::pair<Real, Real>> & outgoing_vals)
  {
    outgoing_vals.resize(
        incoming_points.size(),
        {GeneralFieldTransfer::BetterOutOfMeshValue, GeneralFieldTransfer::BetterOutOfMeshValue});
    // Evaluate interpolation values for these incoming points
    evaluateInterpValues(incoming_points, outgoing_vals);
  };

  DofobjectToInterpValVec dofobject_to_valsvec(_to_problems.size());
  InterpCaches interp_caches(_to_problems.size(), getMaxToProblemsBBoxDimensions());
  InterpCaches distance_caches(_to_problems.size(), getMaxToProblemsBBoxDimensions());

  // Copy data out to incoming_vals_ids
  auto action_functor = [this, &i, &dofobject_to_valsvec, &interp_caches, &distance_caches](
                            processor_id_type pid,
                            const std::vector<Point> & my_outgoing_points,
                            const std::vector<std::pair<Real, Real>> & incoming_vals)
  {
    auto & pointInfoVec = _processor_to_pointInfoVec[pid];

    // Cache interpolation values for each dof object / points
    cacheIncomingInterpVals(pid,
                            i,
                            pointInfoVec,
                            my_outgoing_points,
                            incoming_vals,
                            dofobject_to_valsvec,
                            interp_caches,
                            distance_caches);
  };

  // We assume incoming_vals_ids is ordered in the same way as outgoing_points
  // Hopefully, pull_parallel_vector_data will not mess up this
  const std::pair<Real, Real> * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), outgoing_points, gather_functor, action_functor, ex);

  // Check for conflicts and overlaps from the maps that were built during the transfer
  if (_search_value_conflicts)
    outputValueConflicts(_to_var_names[i], dofobject_to_valsvec, distance_caches);

  // Set cached values into solution vector
  setSolutionVectorValues(i, dofobject_to_valsvec, interp_caches);
}

void
MultiAppGeneralFieldTransfer::locatePointReceivers(const Point point,
                                                   std::set<processor_id_type> & processors)
{
  // Check which processors include this point
  // One point might have more than one points
  bool found = false;
  unsigned int from0 = 0;
  // Find which bboxes might have the nearest node to this point.
  Real nearest_max_distance = std::numeric_limits<Real>::max();
  for (const auto & bbox : _bboxes)
  {
    Real distance = bboxMaxDistance(point, bbox);
    if (distance < nearest_max_distance)
      nearest_max_distance = distance;
  }
  for (processor_id_type i_proc = 0; i_proc < n_processors();
       from0 += _froms_per_proc[i_proc], ++i_proc)
    for (unsigned int i_from = from0; i_from < from0 + _froms_per_proc[i_proc]; ++i_from)
    {
      Real distance = bboxMinDistance(point, _bboxes[i_from]);
      // We will not break here because we want to send a point to all possible source domains
      if (_greedy_search || distance <= nearest_max_distance ||
          _bboxes[i_from].contains_point(point))
      {
        processors.insert(i_proc);
        found = true;
      }
    }
  // Error out if we could not find this point when ask us to do so
  if (!found && _error_on_miss)
    mooseError("Cannot locate point ", point, " \n ", "mismatched meshes are used");
}

void
MultiAppGeneralFieldTransfer::cacheOutgoingPointInfo(const Point point,
                                                     const dof_id_type dof_object_id,
                                                     const unsigned int problem_id,
                                                     ProcessorToPointVec & outgoing_points)
{
  std::set<processor_id_type> processors;
  // Try to find which processors
  processors.clear();
  locatePointReceivers(point, processors);

  // We need to send these data to these processors
  for (auto pid : processors)
  {
    outgoing_points[pid].push_back(point);
    // Store point information
    // We can use these information when insert values to solution vector
    PointInfo pointinfo;
    pointinfo.problem_id = problem_id;
    pointinfo.dof_object_id = dof_object_id;
    pointinfo.offset = 0;
    _processor_to_pointInfoVec[pid].push_back(pointinfo);
  }
}

void
MultiAppGeneralFieldTransfer::extractOutgoingPoints(const unsigned int var_index,
                                                    ProcessorToPointVec & outgoing_points)
{
  // Get the variable name, with the accomodation for array/vector names
  const auto & var_name = getToVarName(var_index);

  // Clean up the map from processor to pointInfo vector
  // This map should be consistent with outgoing_points
  _processor_to_pointInfoVec.clear();

  // Loop over all problems
  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    // libMesh EquationSystems
    auto & es = getEquationSystem(*_to_problems[i_to], _displaced_target_mesh);
    // libMesh system that has this variable
    System * to_sys = find_sys(es, var_name);
    auto sys_num = to_sys->number();
    auto var_num = _to_variables[var_index]->number();
    auto & fe_type = _to_variables[var_index]->feType();
    bool is_nodal = _to_variables[var_index]->isNodal();

    // Moose mesh
    const auto & to_moose_mesh = _to_problems[i_to]->mesh(_displaced_target_mesh);
    const auto & to_mesh = to_moose_mesh.getMesh();

    // We support more general variables via libMesh GenericProjector
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      GeneralFieldTransfer::RecordRequests<Number> f;
      GeneralFieldTransfer::RecordRequests<Gradient> g;
      GeneralFieldTransfer::NullAction<Number> nullsetter;
      const std::vector<unsigned int> varvec(1, var_num);

      libMesh::GenericProjector<GeneralFieldTransfer::RecordRequests<Number>,
                                GeneralFieldTransfer::RecordRequests<Gradient>,
                                Number,
                                GeneralFieldTransfer::NullAction<Number>>
          request_gather(*to_sys, f, &g, nullsetter, varvec);

      // We dont look at boundary restriction, not supported for higher order target variables
      const auto & to_begin = _to_blocks.empty()
                                  ? to_mesh.active_local_elements_begin()
                                  : to_mesh.active_local_subdomain_set_elements_begin(_to_blocks);

      const auto & to_end = _to_blocks.empty()
                                ? to_mesh.active_local_elements_end()
                                : to_mesh.active_local_subdomain_set_elements_end(_to_blocks);

      ConstElemRange to_elem_range(to_begin, to_end);

      request_gather.project(to_elem_range);

      dof_id_type point_id = 0;
      for (Point p : f.points_requested())
        // using the point number as a "dof_object_id" will serve to identify the point if we ever
        // rework interp/distance_cache into the dof_id_to_value maps
        this->cacheOutgoingPointInfo(p + _to_positions[i_to], point_id++, i_to, outgoing_points);

      // This is going to require more complicated transfer work
      if (!g.points_requested().empty())
        mooseError("We don't currently support variables with gradient degrees of freedom");
    }
    else if (is_nodal)
    {
      for (const auto & node : to_mesh.local_node_ptr_range())
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if it is a block restricted transfer and current node does not have
        // specified blocks
        if (!_to_blocks.empty() && !inBlocks(_to_blocks, to_moose_mesh, node))
          continue;

        if (!_to_boundaries.empty() && !onBoundaries(_to_boundaries, to_moose_mesh, node))
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfo(*node + _to_positions[i_to], node->id(), i_to, outgoing_points);
      }
    }
    else // Elemental, constant monomial
    {
      for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if it is a block restricted block and current elem does not have
        // specified blocks
        if (!_to_blocks.empty() && !inBlocks(_to_blocks, elem))
          continue;

        if (!_to_boundaries.empty() && !onBoundaries(_to_boundaries, to_moose_mesh, elem))
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfo(
            elem->vertex_average() + _to_positions[i_to], elem->id(), i_to, outgoing_points);
      } // for
    }   // else
  }     // for
}

void
MultiAppGeneralFieldTransfer::extractLocalFromBoundingBoxes(std::vector<BoundingBox> & local_bboxes)
{
  local_bboxes.resize(_froms_per_proc[processor_id()]);
  // Find the index to the first of this processor's local bounding boxes.
  unsigned int local_start = 0;
  for (processor_id_type i_proc = 0; i_proc < n_processors() && i_proc != processor_id(); ++i_proc)
    local_start += _froms_per_proc[i_proc];

  // Extract the local bounding boxes.
  for (unsigned int i_from = 0; i_from < _froms_per_proc[processor_id()]; ++i_from)
    local_bboxes[i_from] = _bboxes[local_start + i_from];
}

void
MultiAppGeneralFieldTransfer::cacheIncomingInterpVals(
    processor_id_type pid,
    const unsigned int var_index,
    std::vector<PointInfo> & pointInfoVec,
    const std::vector<Point> & point_requests,
    const std::vector<std::pair<Real, Real>> & incoming_vals,
    DofobjectToInterpValVec & dofobject_to_valsvec,
    InterpCaches & interp_caches,
    InterpCaches & distance_caches)
{
  mooseAssert(pointInfoVec.size() == incoming_vals.size(),
              "Number of dof objects does not equal to the number of incoming values");

  dof_id_type val_offset = 0;
  for (auto & pointinfo : pointInfoVec)
  {
    // Retrieve target information from cached point infos
    const auto problem_id = pointinfo.problem_id;
    const auto dof_object_id = pointinfo.dof_object_id;

    auto & fe_type = _to_variables[var_index]->feType();
    bool is_nodal = _to_variables[var_index]->isNodal();

    // In the higher order elemental variable case, we receive point values, not nodal or elemental
    // We use an InterpCache to store the values
    // The distance_cache is necessary to choose between multiple origin problems sending values
    // This code could be unified with the lower order order case by using the dofobject_to_valsvec
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      // Defining only boundary values will not be enough to describe the variable, disallow it
      if (_to_boundaries.size() && (_to_variables[var_index]->getContinuity() == DISCONTINUOUS))
        mooseError(
            "Higher order discontinuous elemental variables are not supported for target-boundary "
            "restricted transfers");

      // Cache solution on target mesh in its local frame of reference
      InterpCache & value_cache = interp_caches[problem_id];
      InterpCache & distance_cache = distance_caches[problem_id];
      Point p = point_requests[val_offset] - _to_positions[problem_id];
      const Number val = incoming_vals[val_offset].first;

      // Initialize distance to be able to compare
      if (!distance_cache.hasKey(p))
        distance_cache[p] = std::numeric_limits<Real>::max();

      // We should only have one closest value for each variable at any given point.
      // While there are shared Qps, on vertices for higher order variables usually,
      // the generic projector only queries each point once
      if (_search_value_conflicts && !GeneralFieldTransfer::isBetterOutOfMeshValue(val) &&
          value_cache.hasKey(p) != 0 && !MooseUtils::absoluteFuzzyEqual(value_cache[p], val) &&
          MooseUtils::absoluteFuzzyEqual(distance_cache[p], incoming_vals[val_offset].second))
        registerConflict(problem_id, dof_object_id, p, incoming_vals[val_offset].second, false);

      if ((!GeneralFieldTransfer::isBetterOutOfMeshValue(val) || _use_nearest_app) &&
          MooseUtils::absoluteFuzzyGreaterThan(distance_cache[p], incoming_vals[val_offset].second))
      {
        // NOTE: We store the distance as well as the value. We really only need the
        // value to construct the variable, but the distance is used to make decisions in nearest
        // node schemes on which value to use
        value_cache[p] = val;
        distance_cache[p] = incoming_vals[val_offset].second;
      }
    }
    else
    {
      // Use this dof object pointer, so we can handle
      // both element and node using the same code
#ifndef NDEBUG
      auto var_num = _to_variables[var_index]->number();
      auto & to_sys = _to_variables[var_index]->sys();

      const MeshBase & to_mesh = _to_problems[problem_id]->mesh(_displaced_target_mesh).getMesh();
      const DofObject * dof_object_ptr = nullptr;
      const auto sys_num = to_sys.number();
      // It is a node
      if (is_nodal)
        dof_object_ptr = to_mesh.node_ptr(dof_object_id);
      // It is an element
      else
        dof_object_ptr = to_mesh.elem_ptr(dof_object_id);

      // We should only be supporting nodal and constant elemental
      // variables in this code path; if we see multiple DoFs on one
      // object we should have been using GenericProjector
      mooseAssert(dof_object_ptr->n_dofs(sys_num, var_num) == 1,
                  "Unexpectedly found " << dof_object_ptr->n_dofs(sys_num, var_num)
                                        << "dofs instead of 1");
#endif

      auto & dofobject_to_val = dofobject_to_valsvec[problem_id];

      // Check if we visited this dof object earlier
      auto values_ptr = dofobject_to_val.find(dof_object_id);
      // We did not visit this
      if (values_ptr == dofobject_to_val.end())
      {
        // Values for this dof object
        auto & val = dofobject_to_val[dof_object_id];
        // Interpolation value
        val.interp = incoming_vals[val_offset].first;
        // Where this value came from
        val.pid = pid;
        // Distance
        val.distance = incoming_vals[val_offset].second;
      }
      else
      {
        auto & val = values_ptr->second;
        // We adopt values that are, in order of priority
        // - valid
        // - closest distance
        // - the smallest rank with the same distance
        // If conflict search is on, we register the overlap, but selection rules stay the same
        if (!GeneralFieldTransfer::isBetterOutOfMeshValue(incoming_vals[val_offset].first) &&
            (MooseUtils::absoluteFuzzyGreaterThan(val.distance, incoming_vals[val_offset].second) ||
             ((val.pid > pid || _search_value_conflicts) &&
              MooseUtils::absoluteFuzzyEqual(val.distance, incoming_vals[val_offset].second))))
        {
          // Keep track of overlaps
          if (detectConflict(val.interp,
                             incoming_vals[val_offset].first,
                             val.distance,
                             incoming_vals[val_offset].second))
          {
            // Keep track of distance and value
            const auto p = point_requests[val_offset] - _to_positions[problem_id];
            registerConflict(problem_id, dof_object_id, p, incoming_vals[val_offset].second, false);

            // We could have been let in the loop by the conflict search
            if (val.pid < pid)
              continue;
          }
          val.interp = incoming_vals[val_offset].first;
          val.pid = pid;
          val.distance = incoming_vals[val_offset].second;
        }
      }
    }

    // Move it to next position
    val_offset++;
  }
}

void
MultiAppGeneralFieldTransfer::registerConflict(
    unsigned int problem, dof_id_type dof_id, Point p, Real dist, bool local)
{
  // NOTE We could be registering the same conflict several times, we could count them instead
  if (local)
    _local_conflicts.push_back(std::make_tuple(problem, dof_id, p, dist));
  else
    _received_conflicts.push_back(std::make_tuple(problem, dof_id, p, dist));
}

void
MultiAppGeneralFieldTransfer::examineValueConflicts(
    const VariableName var_name,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & distance_caches,
    std::vector<std::tuple<unsigned int, dof_id_type, Point, Real>> conflicts_vec)
{
  // We must check a posteriori because we could have two
  // equidistant points with different values from two different problems, but a third point from
  // another problem is actually closer, so there is no conflict because only that last one matters
  // We check here whether the potential conflicts actually were the nearest points
  // Loop over potential conflicts
  for (auto conflict_it = conflicts_vec.begin(); conflict_it != conflicts_vec.end();)
  {
    const auto potential_conflict = *conflict_it;
    bool overlap_found = false;

    // Extract info for the potential overlap
    const unsigned int problem_id = std::get<0>(potential_conflict);
    const dof_id_type dof_object_id = std::get<1>(potential_conflict);
    const Point p = std::get<2>(potential_conflict);
    const Real distance = std::get<3>(potential_conflict);

    // Extract variable info
    auto & es = getEquationSystem(*_to_problems[problem_id], _displaced_target_mesh);
    System * to_sys = find_sys(es, var_name);
    auto var_num = to_sys->variable_number(var_name);
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    // Higher order elemental
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      auto cached_distance = distance_caches[problem_id].find(p);
      if (cached_distance == distance_caches[problem_id].end())
        mooseError("Conflict point was not found in the map of all origin-target distances");

      // Distance is still the distance when we detected a potential overlap
      if (MooseUtils::absoluteFuzzyEqual(cached_distance->second, distance))
        overlap_found = true;
    }
    // Nodal and const monomial variable
    else if (MooseUtils::absoluteFuzzyEqual(
                 dofobject_to_valsvec[problem_id].find(dof_object_id)->second.distance, distance))
      overlap_found = true;

    // Map will only keep the actual overlaps
    if (!overlap_found)
      conflicts_vec.erase(conflict_it);
    else
      ++conflict_it;
  }
}

void
MultiAppGeneralFieldTransfer::outputValueConflicts(
    const VariableName var_name,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & distance_caches)
{
  // Remove potential conflicts that did not materialize, the value did not end up being used
  examineValueConflicts(var_name, dofobject_to_valsvec, distance_caches, _received_conflicts);

  // Output the conflicts from the selection of local values (evaluateInterpValues-type routines)
  // to send in response to value requests at target points
  const std::string rank_str = std::to_string(_communicator.rank());
  if (_local_conflicts.size())
  {
    std::string local_conflicts_string = "";
    for (const auto & conflict : _local_conflicts)
    {
      const unsigned int problem_id = std::get<0>(conflict);
      const Point p = std::get<2>(conflict);

      std::string origin_domain_message;
      if (hasFromMultiApp() && !_nearest_positions_obj)
      {
        // NOTES:
        // - The origin app for a conflict may not be unique.
        // - The conflicts vectors only store the conflictual points, not the original one
        //   The original value found with a given distance could be retrieved from the main caches
        const auto app_id = _from_local2global_map[problem_id];
        origin_domain_message = "In source child app " + std::to_string(app_id) + " mesh,";
      }
      // We can't locate the source app when considering nearest positions, we return the conflict
      // location in the target app (parent or sibling) instead
      else if (hasFromMultiApp() && _nearest_positions_obj)
        origin_domain_message = "In target app mesh,";
      else
        origin_domain_message = "In source parent app mesh,";

      local_conflicts_string += origin_domain_message + " point: (" + std::to_string(p(0)) + ", " +
                                std::to_string(p(1)) + ", " + std::to_string(p(2)) + ")\n";
    }
    mooseWarning(
        "On rank " + rank_str +
        ", multiple valid values from equidistant points were "
        "found in the origin mesh for variable '" +
        var_name + "' for " + std::to_string(_local_conflicts.size()) +
        " target points.\nAre multiple subapps overlapping?\n"
        "Are some points in target mesh equidistant from source nodes in origin mesh(es)?\n"
        "Conflicts detected at :\n" +
        local_conflicts_string);
  }

  // Output the conflicts discovered when receiving values from multiple origin problems
  if (_received_conflicts.size())
  {
    std::string received_conflict_string = "";
    for (const auto & conflict : _received_conflicts)
    {
      // Extract info for the potential overlap
      const unsigned int problem_id = std::get<0>(conflict);
      const Point p = std::get<2>(conflict);

      std::string target_domain_message;
      if (hasToMultiApp())
      {
        const auto app_id = _to_local2global_map[problem_id];
        target_domain_message = "In target child app " + std::to_string(app_id) + " mesh,";
      }
      else
        target_domain_message = "In target parent app mesh,";

      received_conflict_string += target_domain_message + " point: (" + std::to_string(p(0)) +
                                  ", " + std::to_string(p(1)) + ", " + std::to_string(p(2)) + ")\n";
    }
    mooseWarning(
        "On rank " + rank_str +
        ", multiple valid values from equidistant points were "
        "received for variable '" +
        var_name + "' for " + std::to_string(_received_conflicts.size()) +
        " target points.\nAre multiple subapps overlapping?\n"
        "Are some points in target mesh equidistant from source nodes in origin mesh(es)?\n"
        "Conflicts detected at :\n" +
        received_conflict_string);
  }

  // Reset the conflicts vectors, to be used for checking conflicts when transferring the next
  // variable
  _local_conflicts.clear();
  _received_conflicts.clear();
}

void
MultiAppGeneralFieldTransfer::setSolutionVectorValues(
    const unsigned int var_index,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & interp_caches)
{
  // Get the variable name, with the accomodation for array/vector names
  const auto & var_name = getToVarName(var_index);

  for (unsigned int problem_id = 0; problem_id < _to_problems.size(); ++problem_id)
  {
    auto & dofobject_to_val = dofobject_to_valsvec[problem_id];

    // libMesh EquationSystems
    // NOTE: we would expect to set variables from the displaced equation system here
    auto & es = getEquationSystem(*_to_problems[problem_id], false);

    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    const MeshBase & to_mesh = _to_problems[problem_id]->mesh(_displaced_target_mesh).getMesh();
    auto var_num = to_sys->variable_number(var_name);
    auto sys_num = to_sys->number();

    auto & fe_type = _to_variables[var_index]->feType();
    bool is_nodal = _to_variables[var_index]->isNodal();

    if (fe_type.order > CONSTANT && !is_nodal)
    {
      // We may need to use existing data values in places where the
      // from app domain doesn't overlap
      MeshFunction to_func(es, *to_sys->current_local_solution, to_sys->get_dof_map(), var_num);
      to_func.init();

      GeneralFieldTransfer::CachedData<Number> f(
          interp_caches[problem_id], to_func, _default_extrapolation_value);
      libMesh::VectorSetAction<Number> setter(*to_sys->solution);
      const std::vector<unsigned int> varvec(1, var_num);

      libMesh::GenericProjector<GeneralFieldTransfer::CachedData<Number>,
                                GeneralFieldTransfer::CachedData<Gradient>,
                                Number,
                                libMesh::VectorSetAction<Number>>
          set_solution(*to_sys, f, nullptr, setter, varvec);

      // We dont look at boundary restriction, not supported for higher order target variables
      const auto & to_begin = _to_blocks.empty()
                                  ? to_mesh.active_local_elements_begin()
                                  : to_mesh.active_local_subdomain_set_elements_begin(_to_blocks);

      const auto & to_end = _to_blocks.empty()
                                ? to_mesh.active_local_elements_end()
                                : to_mesh.active_local_subdomain_set_elements_end(_to_blocks);

      ConstElemRange active_local_elem_range(to_begin, to_end);

      set_solution.project(active_local_elem_range);
    }
    else
    {
      for (auto & val_pair : dofobject_to_val)
      {
        auto dof_object_id = val_pair.first;

        const DofObject * dof_object = nullptr;
        if (is_nodal)
          dof_object = to_mesh.node_ptr(dof_object_id);
        else
          dof_object = to_mesh.elem_ptr(dof_object_id);

        auto dof = dof_object->dof_number(sys_num, var_num, 0);

        auto val = val_pair.second.interp;

        // This will happen if meshes are mismatched
        if (_error_on_miss && GeneralFieldTransfer::isBetterOutOfMeshValue(val))
        {
          if (is_nodal)
            mooseError("Node ", dof_object_id, " for app ", problem_id, " could not be located ");
          else
            mooseError(
                "Element ", dof_object_id, " for app ", problem_id, " could not be located ");
        }

        // We should not put garbage into our solution vector
        // but it can be that we want to set it to a different value than what was already there
        // for example: the source app has been displaced and was sending an indicator of its
        // position
        if (GeneralFieldTransfer::isBetterOutOfMeshValue(val))
        {
          if (!GeneralFieldTransfer::isBetterOutOfMeshValue(_default_extrapolation_value))
            to_sys->solution->set(dof, _default_extrapolation_value);
          continue;
        }

        to_sys->solution->set(dof, val);
      }
    }

    to_sys->solution->close();
    // Sync local solutions
    to_sys->update();
  }
}

bool
MultiAppGeneralFieldTransfer::acceptPointInOriginMesh(unsigned int i_from,
                                                      const std::vector<BoundingBox> & local_bboxes,
                                                      const Point & pt) const
{
  if (!local_bboxes[i_from].contains_point(pt))
    return false;
  else
  {
    auto * pl = _from_point_locators[i_from].get();

    // Check block restriction
    if (!_from_blocks.empty() && !inBlocks(_from_blocks, pl, pt - _from_positions[i_from]))
      return false;

    // Check boundary restriction. Passing the block restriction will speed up the search
    if (!_from_boundaries.empty() && !onBoundaries(_from_boundaries,
                                                   _from_blocks,
                                                   *_from_meshes[i_from],
                                                   pl,
                                                   pt - _from_positions[i_from]))
      return false;

    // Check that the app actually contains the origin point
    // We dont need to check if we already found it in a block or a boundary
    if (_from_blocks.empty() && _from_boundaries.empty() && _source_app_must_contain_point &&
        !inMesh(pl, pt - _from_positions[i_from]))
      return false;
  }
  return true;
}

bool
MultiAppGeneralFieldTransfer::inMesh(const PointLocatorBase * const pl, const Point & point) const
{
  // Note: we do not take advantage of a potential block restriction of the mesh here. This is
  // because we can avoid this routine by calling inBlocks() instead
  const Elem * elem = (*pl)(point);
  return (elem != nullptr);
}

bool
MultiAppGeneralFieldTransfer::inBlocks(const std::set<SubdomainID> & blocks,
                                       const Elem * elem) const
{
  return blocks.find(elem->subdomain_id()) != blocks.end();
}

bool
MultiAppGeneralFieldTransfer::inBlocks(const std::set<SubdomainID> & blocks,
                                       const MooseMesh & /* mesh */,
                                       const Elem * elem) const
{
  return inBlocks(blocks, elem);
}

bool
MultiAppGeneralFieldTransfer::inBlocks(const std::set<SubdomainID> & blocks,
                                       const MooseMesh & mesh,
                                       const Node * node) const
{
  const std::set<SubdomainID> & node_blocks = mesh.getNodeBlockIds(*node);
  std::set<SubdomainID> u;
  std::set_intersection(blocks.begin(),
                        blocks.end(),
                        node_blocks.begin(),
                        node_blocks.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

bool
MultiAppGeneralFieldTransfer::inBlocks(const std::set<SubdomainID> & blocks,
                                       const PointLocatorBase * const pl,
                                       const Point & point) const
{
  const Elem * elem = (*pl)(point, &blocks);
  return (elem != nullptr);
}

bool
MultiAppGeneralFieldTransfer::onBoundaries(const std::set<BoundaryID> & boundaries,
                                           const MooseMesh & mesh,
                                           const Node * node) const
{
  const BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> vec_to_fill;
  bnd_info.boundary_ids(node, vec_to_fill);
  std::set<BoundaryID> vec_to_fill_set(vec_to_fill.begin(), vec_to_fill.end());
  std::set<BoundaryID> u;
  std::set_intersection(boundaries.begin(),
                        boundaries.end(),
                        vec_to_fill_set.begin(),
                        vec_to_fill_set.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

bool
MultiAppGeneralFieldTransfer::onBoundaries(const std::set<BoundaryID> & boundaries,
                                           const MooseMesh & mesh,
                                           const Elem * elem) const
{
  // Get all boundaries each side of the element is part of
  const BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> vec_to_fill;
  std::vector<BoundaryID> vec_to_fill_temp;
  if (_elemental_boundary_restriction_on_sides)
    for (auto side : make_range(elem->n_sides()))
    {
      bnd_info.boundary_ids(elem, side, vec_to_fill_temp);
      vec_to_fill.insert(vec_to_fill.end(), vec_to_fill_temp.begin(), vec_to_fill_temp.end());
    }
  else
    for (auto node_index : make_range(elem->n_nodes()))
    {
      bnd_info.boundary_ids(elem->node_ptr(node_index), vec_to_fill_temp);
      vec_to_fill.insert(vec_to_fill.end(), vec_to_fill_temp.begin(), vec_to_fill_temp.end());
    }
  std::set<BoundaryID> vec_to_fill_set(vec_to_fill.begin(), vec_to_fill.end());

  // Look for a match between the boundaries from the restriction and those near the element
  std::set<BoundaryID> u;
  std::set_intersection(boundaries.begin(),
                        boundaries.end(),
                        vec_to_fill_set.begin(),
                        vec_to_fill_set.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

bool
MultiAppGeneralFieldTransfer::onBoundaries(const std::set<BoundaryID> & boundaries,
                                           const std::set<SubdomainID> & block_restriction,
                                           const MooseMesh & mesh,
                                           const PointLocatorBase * const pl,
                                           const Point & point) const
{
  // Find the element containing the point and use the block restriction if known for speed
  const Elem * elem;
  if (block_restriction.empty())
    elem = (*pl)(point);
  else
    elem = (*pl)(point, &block_restriction);

  if (!elem)
    return false;
  return onBoundaries(boundaries, mesh, elem);
}

bool
MultiAppGeneralFieldTransfer::closestToPosition(unsigned int pos_index, const Point & pt) const
{
  mooseAssert(_nearest_positions_obj, "Should not be here without a positions object");
  bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;
  return _nearest_positions_obj->getPosition(pos_index, initial) ==
         _nearest_positions_obj->getNearestPosition(pt, initial);
}

Real
MultiAppGeneralFieldTransfer::bboxMaxDistance(const Point & p, const BoundingBox & bbox)
{
  std::array<Point, 2> source_points = {{bbox.first, bbox.second}};

  std::array<Point, 8> all_points;
  for (unsigned int x = 0; x < 2; x++)
    for (unsigned int y = 0; y < 2; y++)
      for (unsigned int z = 0; z < 2; z++)
        all_points[x + 2 * y + 4 * z] =
            Point(source_points[x](0), source_points[y](1), source_points[z](2));

  Real max_distance = 0.;

  for (unsigned int i = 0; i < 8; i++)
  {
    Real distance = (p - all_points[i]).norm();
    if (distance > max_distance)
      max_distance = distance;
  }

  return max_distance;
}

Real
MultiAppGeneralFieldTransfer::bboxMinDistance(const Point & p, const BoundingBox & bbox)
{
  std::array<Point, 2> source_points = {{bbox.first, bbox.second}};

  std::array<Point, 8> all_points;
  for (unsigned int x = 0; x < 2; x++)
    for (unsigned int y = 0; y < 2; y++)
      for (unsigned int z = 0; z < 2; z++)
        all_points[x + 2 * y + 4 * z] =
            Point(source_points[x](0), source_points[y](1), source_points[z](2));

  Real min_distance = std::numeric_limits<Real>::max();

  for (unsigned int i = 0; i < 8; i++)
  {
    Real distance = (p - all_points[i]).norm();
    if (distance < min_distance)
      min_distance = distance;
  }

  return min_distance;
}

std::vector<BoundingBox>
MultiAppGeneralFieldTransfer::getRestrictedFromBoundingBoxes()
{
  std::vector<std::pair<Point, Point>> bb_points(_from_meshes.size());
  const Real min_r = std::numeric_limits<Real>::lowest();
  const Real max_r = std::numeric_limits<Real>::max();

  for (const auto j : make_range(_from_meshes.size()))
  {
    Point min(max_r, max_r, max_r);
    Point max(min_r, min_r, min_r);
    bool at_least_one = false;
    const auto & from_mesh = _from_problems[j]->mesh(_displaced_source_mesh);

    for (auto & elem : as_range(from_mesh.getMesh().local_elements_begin(),
                                from_mesh.getMesh().local_elements_end()))
    {
      if (!_from_blocks.empty() && !inBlocks(_from_blocks, from_mesh, elem))
        continue;

      for (auto & node : elem->node_ref_range())
      {
        if (!_from_boundaries.empty() && !onBoundaries(_from_boundaries, from_mesh, &node))
          continue;

        at_least_one = true;
        for (const auto i : make_range(LIBMESH_DIM))
        {
          min(i) = std::min(min(i), node(i));
          max(i) = std::max(max(i), node(i));
        }
      }
    }

    // For 2D RZ problems, we need to amend the bounding box to cover the whole XYZ projection
    // - The XYZ-Y axis is assumed aligned with the RZ-Z axis
    // - RZ systems also cover negative coordinates hence the use of the maximum R
    // NOTE: We will only support the case where there is only one coordinate system
    if ((from_mesh.getUniqueCoordSystem() == Moose::COORD_RZ) && (LIBMESH_DIM == 3))
    {
      min(0) = -max(0);
      min(2) = -max(0);
      max(2) = max(0);
    }

    BoundingBox bbox(min, max);
    if (!at_least_one)
      bbox.min() = max; // If we didn't hit any nodes, this will be _the_ minimum bbox
    else
    {
      // Translate the bounding box to the from domain's position.
      bbox.first += _from_positions[j];
      bbox.second += _from_positions[j];
    }

    // Cast the bounding box into a pair of points (so it can be put through
    // MPI communication).
    bb_points[j] = static_cast<std::pair<Point, Point>>(bbox);
  }

  // Serialize the bounding box points.
  _communicator.allgather(bb_points);

  // Recast the points back into bounding boxes and return.
  std::vector<BoundingBox> bboxes(bb_points.size());
  for (const auto i : make_range(bb_points.size()))
    bboxes[i] = static_cast<BoundingBox>(bb_points[i]);

  // Check for a user-set fixed bounding box size and modify the sizes as appropriate
  if (_fixed_bbox_size != std::vector<Real>(3, 0))
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      if (!MooseUtils::absoluteFuzzyEqual(_fixed_bbox_size[i], 0))
        for (const auto j : make_range(bboxes.size()))
        {
          const auto current_width = (bboxes[j].second - bboxes[j].first)(i);
          bboxes[j].first(i) -= (_fixed_bbox_size[i] - current_width) / 2;
          bboxes[j].second(i) += (_fixed_bbox_size[i] - current_width) / 2;
        }

  return bboxes;
}

VariableName
MultiAppGeneralFieldTransfer::getFromVarName(unsigned int var_index)
{
  VariableName var_name = _from_var_names[var_index];
  if (_from_var_components.size())
    var_name += "_" + std::to_string(_from_var_components[var_index]);
  return var_name;
}

VariableName
MultiAppGeneralFieldTransfer::getToVarName(unsigned int var_index)
{
  VariableName var_name = _to_var_names[var_index];
  if (_to_var_components.size())
    var_name += "_" + std::to_string(_to_var_components[var_index]);
  return var_name;
}

Point
MultiAppGeneralFieldTransfer::getMaxToProblemsBBoxDimensions() const
{
  Point max_dimension = {std::numeric_limits<Real>::min(),
                         std::numeric_limits<Real>::min(),
                         std::numeric_limits<Real>::min()};

  for (auto & to_mesh : _to_meshes)
  {
    const auto bbox = to_mesh->getInflatedProcessorBoundingBox();
    for (auto dim : make_range(LIBMESH_DIM))
      max_dimension(dim) = std::max(
          max_dimension(dim), std::max(std::abs(bbox.first(dim)), std::abs(bbox.second(dim))));
  }

  return max_dimension;
}

bool
MultiAppGeneralFieldTransfer::detectConflict(Real current_value,
                                             Real new_value,
                                             Real current_distance,
                                             Real new_distance) const
{
  // No conflict if we're not looking for them
  if (_search_value_conflicts)
    // Only consider conflicts if the values are valid and different
    if (current_value != GeneralFieldTransfer::BetterOutOfMeshValue &&
        new_value != GeneralFieldTransfer::BetterOutOfMeshValue &&
        !MooseUtils::absoluteFuzzyEqual(current_value, new_value))
      // Conflict only occurs if the origin points are equidistant
      if (MooseUtils::absoluteFuzzyEqual(current_distance, new_distance))
        return true;
  return false;
}
