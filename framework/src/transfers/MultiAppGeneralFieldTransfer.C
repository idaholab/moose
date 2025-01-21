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
#include "MeshDivision.h"
#include "Positions.h"
#include "Factory.h"
#include "MooseAppCoordTransform.h"

// libmesh includes
#include "libmesh/point_locator_base.h"
#include "libmesh/enum_point_locator_type.h"

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

using namespace libMesh;

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

  // Mesh division restriction
  params.addParam<MeshDivisionName>("from_mesh_division",
                                    "Mesh division object on the origin application");
  params.addParam<MeshDivisionName>("to_mesh_division",
                                    "Mesh division object on the target application");
  MooseEnum mesh_division_uses("spatial_restriction matching_division matching_subapp_index none",
                               "none");
  params.addParam<MooseEnum>("from_mesh_division_usage",
                             mesh_division_uses,
                             "How to use the source mesh division in the transfer. See object "
                             "documentation for description of each option");
  params.addParam<MooseEnum>("to_mesh_division_usage",
                             mesh_division_uses,
                             "How to use the target mesh division in the transfer. See object "
                             "documentation for description of each option");

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
  params.addParam<bool>("use_bounding_boxes",
                        true,
                        "When set to false, bounding boxes will not be used to restrict the source "
                        "of the transfer. Either source applications must be set using the "
                        "from_mesh_division parameter, or a greedy search must be used.");
  params.addParam<bool>(
      "use_nearest_app",
      false,
      "When True, transfers from a child application will work by finding the nearest (using "
      "the `position` + mesh centroid) sub-app and query that app for the value to transfer.");
  params.addParam<PositionsName>(
      "use_nearest_position",
      "Name of the the Positions object (in main app) such that transfers to/from a child "
      "application will work by finding the nearest position to a target and query only the "
      "app / points closer to this position than to any other position for the value to transfer.");
  params.addParam<bool>(
      "from_app_must_contain_point",
      false,
      "Wether on not the origin mesh must contain the point to evaluate data at. If false, this "
      "allows for interpolation between origin app meshes. Origin app bounding boxes are still "
      "considered so you may want to increase them with 'fixed_bounding_box_size'");
  params.addParam<bool>("search_value_conflicts",
                        true,
                        "Whether to look for potential conflicts between two valid and different "
                        "source values for any target point");
  params.addParam<unsigned int>(
      "value_conflicts_output",
      10,
      "Maximum number of conflicts to output if value-conflicts, from equidistant sources to a "
      "given transfer target location, search is turned on");

  params.addParamNamesToGroup(
      "to_blocks from_blocks to_boundaries from_boundaries elemental_boundary_restriction "
      "from_mesh_division from_mesh_division_usage to_mesh_division to_mesh_division_usage",
      "Transfer spatial restriction");
  params.addParamNamesToGroup(
      "greedy_search use_bounding_boxes use_nearest_app use_nearest_position "
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
    _use_bounding_boxes(getParam<bool>("use_bounding_boxes")),
    _use_nearest_app(getParam<bool>("use_nearest_app")),
    _nearest_positions_obj(
        isParamValid("use_nearest_position")
            ? &_fe_problem.getPositionsObject(getParam<PositionsName>("use_nearest_position"))
            : nullptr),
    _source_app_must_contain_point(getParam<bool>("from_app_must_contain_point")),
    _from_mesh_division_behavior(getParam<MooseEnum>("from_mesh_division_usage")),
    _to_mesh_division_behavior(getParam<MooseEnum>("to_mesh_division_usage")),
    _elemental_boundary_restriction_on_sides(
        getParam<MooseEnum>("elemental_boundary_restriction") == "sides"),
    _greedy_search(getParam<bool>("greedy_search")),
    _search_value_conflicts(getParam<bool>("search_value_conflicts")),
    _already_output_search_value_conflicts(false),
    _search_value_conflicts_max_log(getParam<unsigned int>("value_conflicts_output")),
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
      paramError("use_nearest_app",
                 "Should have a 'from_multiapp' when using the nearest-app informed search");
    auto pos_params = _app.getFactory().getValidParams("MultiAppPositions");
    pos_params.set<std::vector<MultiAppName>>("multiapps") = {getFromMultiApp()->name()};
    _fe_problem.addReporter("MultiAppPositions", "_created_for_" + name(), pos_params);
    _nearest_positions_obj = &_fe_problem.getPositionsObject("_created_for_" + name());
  }

  // Dont let users get wrecked by bounding boxes if it looks like they are trying to extrapolate
  if (!_source_app_must_contain_point && _use_bounding_boxes &&
      (_nearest_positions_obj || isParamSetByUser("from_app_must_contain_point")))
    if (!isParamSetByUser("bbox_factor") && !isParamSetByUser("fixed_bounding_box_size"))
      mooseWarning(
          "Extrapolation (nearest-source options, outside-app source) parameters have "
          "been passed, but no subapp bounding box expansion parameters have been passed.");

  if (!_use_bounding_boxes &&
      (isParamValid("fixed_bounding_box_size") || isParamSetByUser("bbox_factor")))
    paramError("use_bounding_boxes",
               "Cannot pass additional bounding box parameters (sizes, expansion, etc) if we are "
               "not using bounding boxes");
}

void
MultiAppGeneralFieldTransfer::initialSetup()
{
  MultiAppConservativeTransfer::initialSetup();

  // Use IDs for block and boundary restriction
  // Loop over all source problems
  for (const auto i_from : index_range(_from_problems))
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

    if (isParamValid("from_mesh_division"))
    {
      const auto & mesh_div_name = getParam<MeshDivisionName>("from_mesh_division");
      _from_mesh_divisions.push_back(&_from_problems[i_from]->getMeshDivision(mesh_div_name));
      // Check that the behavior set makes sense
      if (_from_mesh_division_behavior == MeshDivisionTransferUse::RESTRICTION)
      {
        if (_from_mesh_divisions[i_from]->coversEntireMesh())
          mooseInfo("'from_mesh_division_usage' is set to use a spatial restriction but the "
                    "'from_mesh_division' for source app of global index " +
                    std::to_string(getGlobalSourceAppIndex(i_from)) +
                    " covers the entire mesh. Do not expect any restriction from a mesh "
                    "division that covers the entire mesh");
      }
      else if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX &&
               !isParamValid("to_mesh_division"))
        paramError("to_mesh_division_usage",
                   "Source mesh division cannot match target mesh division if no target mesh "
                   "division is specified");
      else if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX)
      {
        if (!hasToMultiApp())
          paramError("from_mesh_division_usage",
                     "Cannot match source mesh division index to target subapp index if there is "
                     "only one target: the parent app (not a subapp)");
        else if (getToMultiApp()->numGlobalApps() !=
                 _from_mesh_divisions[i_from]->getNumDivisions())
          mooseWarning("Attempting to match target subapp index with the number of source mesh "
                       "divisions, which is " +
                       std::to_string(_from_mesh_divisions[i_from]->getNumDivisions()) +
                       " while there are " + std::to_string(getToMultiApp()->numGlobalApps()) +
                       " target subapps");
        if (_to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX)
          // We do not support it because it would require sending the point + target app index +
          // target app division index, and we only send the Point + one number
          paramError("from_mesh_division_usage",
                     "We do not support using target subapp index for source division behavior and "
                     "matching the division index for the target mesh division behavior.");
      }
      else if (_from_mesh_division_behavior == "none")
        paramError("from_mesh_division_usage", "User must specify a 'from_mesh_division_usage'");
    }
    else if (_from_mesh_division_behavior != "none")
      paramError("from_mesh_division",
                 "'from_mesh_division' must be specified if the usage method is specified");
  }

  // Loop over all target problems
  for (const auto i_to : index_range(_to_problems))
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

    if (isParamValid("to_mesh_division"))
    {
      const auto & mesh_div_name = getParam<MeshDivisionName>("to_mesh_division");
      _to_mesh_divisions.push_back(&_to_problems[i_to]->getMeshDivision(mesh_div_name));
      // Check that the behavior set makes sense
      if (_to_mesh_division_behavior == MeshDivisionTransferUse::RESTRICTION)
      {
        if (_to_mesh_divisions[i_to]->coversEntireMesh())
          mooseInfo("'to_mesh_division_usage' is set to use a spatial restriction but the "
                    "'to_mesh_division' for target application of global index " +
                    std::to_string(getGlobalSourceAppIndex(i_to)) +
                    " covers the entire mesh. Do not expect any restriction from a mesh "
                    "division that covers the entire mesh");
      }
      else if (_to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX)
      {
        if (!isParamValid("from_mesh_division"))
          paramError("to_mesh_division_usage",
                     "Target mesh division cannot match source mesh division if no source mesh "
                     "division is specified");
        else if ((*_from_mesh_divisions.begin())->getNumDivisions() !=
                 _to_mesh_divisions[i_to]->getNumDivisions())
          mooseWarning("Source and target mesh divisions do not have the same number of bins. If "
                       "this is what you expect, please reach out to a MOOSE or app developer to "
                       "ensure appropriate use");
      }
      else if (_to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX)
      {
        if (!hasFromMultiApp())
          paramError(
              "to_mesh_division_usage",
              "Cannot match target mesh division index to source subapp index if there is only one "
              "source: the parent app (not a subapp)");
        else if (getFromMultiApp()->numGlobalApps() != _to_mesh_divisions[i_to]->getNumDivisions())
          mooseWarning("Attempting to match source subapp index with the number of target mesh "
                       "divisions, which is " +
                       std::to_string(_to_mesh_divisions[i_to]->getNumDivisions()) +
                       " while there are " + std::to_string(getFromMultiApp()->numGlobalApps()) +
                       " source subapps");
        if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX)
          paramError(
              "from_mesh_division_usage",
              "We do not support using source subapp index for the target division behavior and "
              "matching the division index for the source mesh division behavior.");
      }
      else if (_to_mesh_division_behavior == "none")
        paramError("to_mesh_division_usage", "User must specify a 'to_mesh_division_usage'");
    }
    else if (_to_mesh_division_behavior != "none")
      paramError("to_mesh_division",
                 "'to_mesh_division' must be specified if usage method '" +
                     Moose::stringify(_to_mesh_division_behavior) + "' is specified");
  }

  // Check if components are set correctly if using an array variable
  for (const auto i_from : index_range(_from_problems))
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
  for (const auto i_to : index_range(_to_problems))
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
    for (const auto i_var : index_range(_to_var_names))
      _to_variables[i_var] = &_to_problems[0]->getVariable(
          0, _to_var_names[i_var], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  }
}

void
MultiAppGeneralFieldTransfer::getAppInfo()
{
  MultiAppFieldTransfer::getAppInfo();

  // Create the point locators to locate evaluation points in the origin mesh(es)
  _from_point_locators.resize(_from_problems.size());
  for (const auto i_from : index_range(_from_problems))
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
  TIME_SECTION(
      "MultiAppGeneralFieldTransfer::execute()_" + name(), 5, "Transfer execution " + name());
  getAppInfo();

  // Set up bounding boxes, etc
  prepareToTransfer();

  // loop over the vector of variables and make the transfer one by one
  for (const auto i : make_range(_var_size))
    transferVariable(i);

  postExecute();
}

void
MultiAppGeneralFieldTransfer::prepareToTransfer()
{
  // Get the bounding boxes for the "from" domains.
  // Clean up _from_bboxes from the previous transfer execution
  _from_bboxes.clear();

  // NOTE: This ignores the app's bounding box inflation and padding
  _from_bboxes = getRestrictedFromBoundingBoxes();

  // Expand bounding boxes. Some desired points might be excluded
  // without an expansion
  extendBoundingBoxes(_bbox_factor, _from_bboxes);

  // Figure out how many "from" domains each processor owns.
  _froms_per_proc.clear();
  _froms_per_proc = getFromsPerProc();

  // Get the index for the first source app every processor owns
  _global_app_start_per_proc = getGlobalStartAppPerProc();

  // No need to keep searching for conflicts if the mesh has not changed
  if (_already_output_search_value_conflicts && !_displaced_source_mesh && !_displaced_target_mesh)
    _search_value_conflicts = false;
}

void
MultiAppGeneralFieldTransfer::postExecute()
{
  MultiAppConservativeTransfer::postExecute();
  if (_search_value_conflicts)
    _already_output_search_value_conflicts = true;
}

void
MultiAppGeneralFieldTransfer::transferVariable(unsigned int i)
{
  mooseAssert(i < _var_size, "The variable of index " << i << " does not exist");

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
  auto gather_functor =
      [this](processor_id_type /*pid*/,
             const std::vector<std::pair<Point, unsigned int>> & incoming_locations,
             std::vector<std::pair<Real, Real>> & outgoing_vals)
  {
    outgoing_vals.resize(
        incoming_locations.size(),
        {GeneralFieldTransfer::BetterOutOfMeshValue, GeneralFieldTransfer::BetterOutOfMeshValue});
    // Evaluate interpolation values for these incoming points
    evaluateInterpValues(incoming_locations, outgoing_vals);
  };

  DofobjectToInterpValVec dofobject_to_valsvec(_to_problems.size());
  InterpCaches interp_caches(_to_problems.size(), getMaxToProblemsBBoxDimensions());
  InterpCaches distance_caches(_to_problems.size(), getMaxToProblemsBBoxDimensions());

  // Copy data out to incoming_vals_ids
  auto action_functor = [this, &i, &dofobject_to_valsvec, &interp_caches, &distance_caches](
                            processor_id_type pid,
                            const std::vector<std::pair<Point, unsigned int>> & my_outgoing_points,
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
    outputValueConflicts(i, dofobject_to_valsvec, distance_caches);

  // Set cached values into solution vector
  setSolutionVectorValues(i, dofobject_to_valsvec, interp_caches);
}

void
MultiAppGeneralFieldTransfer::locatePointReceivers(const Point point,
                                                   std::set<processor_id_type> & processors)
{
  // Check which processors have apps that may include or be near this point
  // A point may be close enough to several problems, hosted on several processes
  bool found = false;

  // Additional process-restriction techniques we could use (TODOs):
  // - create a heuristic for using nearest-positions
  // - from_mesh_divisions could be polled for which divisions they possess on each
  //   process, depending on the behavior chosen. This could limit potential senders.
  //   This should be done ahead of this function call, for all points at once

  // Determine the apps which will be receiving points (then sending values) using various
  // heuristics
  if (_use_nearest_app)
  {
    // Find the nearest position for the point
    const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;
    // The apps form the nearest positions here, this is the index of the nearest app
    const auto nearest_index = _nearest_positions_obj->getNearestPositionIndex(point, initial);

    // Find the apps that are nearest to the same position
    // Global search over all applications
    for (processor_id_type i_proc = 0; i_proc < n_processors(); ++i_proc)
    {
      // We need i_from to correspond to the global app index
      unsigned int from0 = _global_app_start_per_proc[i_proc];
      for (unsigned int i_from = from0; i_from < from0 + _froms_per_proc[i_proc]; ++i_from)
      {
        if (_greedy_search || _search_value_conflicts || i_from == nearest_index)
        {
          processors.insert(i_proc);
          found = true;
        }
        mooseAssert(i_from < getFromMultiApp()->numGlobalApps(), "We should not reach this");
      }
    }
    mooseAssert((getFromMultiApp()->numGlobalApps() < n_processors() || processors.size() == 1) ||
                    _greedy_search || _search_value_conflicts,
                "Should only be one source processor when using more processors than source apps");
  }
  else if (_use_bounding_boxes)
  {
    // We examine all (global) bounding boxes and find the minimum of the maximum distances within a
    // bounding box from the point. This creates a sphere around the point of interest. Any app
    // with a bounding box that intersects this sphere (with a bboxMinDistance <
    // nearest_max_distance) will be considered a potential source
    // NOTE: This is a heuristic. We could try others
    // NOTE: from_bboxes are in the reference space, as is the point.
    Real nearest_max_distance = std::numeric_limits<Real>::max();
    for (const auto & bbox : _from_bboxes)
    {
      Real distance = bboxMaxDistance(point, bbox);
      if (distance < nearest_max_distance)
        nearest_max_distance = distance;
    }

    unsigned int from0 = 0;
    for (processor_id_type i_proc = 0; i_proc < n_processors();
         from0 += _froms_per_proc[i_proc], ++i_proc)
      // i_from here is a hybrid index based on the cumulative sum of the apps per processor
      for (unsigned int i_from = from0; i_from < from0 + _froms_per_proc[i_proc]; ++i_from)
      {
        Real distance = bboxMinDistance(point, _from_bboxes[i_from]);
        // We will not break here because we want to send a point to all possible source domains
        if (_greedy_search || distance <= nearest_max_distance ||
            _from_bboxes[i_from].contains_point(point))
        {
          processors.insert(i_proc);
          found = true;
        }
      }
  }
  // Greedy search will contact every single processor. It's not scalable, but if there's valid data
  // on any subapp on any process, it will find it
  else if (_greedy_search)
  {
    found = true;
    for (const auto i_proc : make_range(n_processors()))
      processors.insert(i_proc);
  }
  // Since we indicated that we only wanted values from a subapp with the same global index as the
  // target mesh division, we might as well only communicate with the process that owns this app
  else if (!_to_mesh_divisions.empty() &&
           _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX)
  {
    // The target point could have a different index in each target mesh division. So on paper, we
    // would need to check all of them.
    auto saved_target_div = MooseMeshDivision::INVALID_DIVISION_INDEX;
    for (const auto i_to : index_range(_to_meshes))
    {
      const auto target_div = _to_mesh_divisions[i_to]->divisionIndex(
          _to_transforms[getGlobalTargetAppIndex(i_to)]->mapBack(point));
      // If it's the same division index, do not redo the search
      if (target_div == saved_target_div)
        continue;
      else
        saved_target_div = target_div;

      // Look for the processors owning a source-app with an index equal to the target mesh division
      for (const auto i_proc : make_range(n_processors()))
        for (const auto i_from : make_range(_froms_per_proc[i_proc]))
          if (target_div == _global_app_start_per_proc[i_proc] + i_from)
          {
            processors.insert(i_proc);
            found = true;
          }
    }
  }
  else
    mooseError("No algorithm were selected to find which processes may send value data "
               "for a each target point. Please either specify using bounding boxes, "
               "greedy search, or to_mesh_division-based parameters");

  // Error out if we could not find this point when ask us to do so
  if (!found && _error_on_miss)
    mooseError(
        "Cannot find a source application to provide a value at point: ",
        point,
        " \n ",
        "It must be that mismatched meshes, between the source and target application, are being "
        "used.\nIf you are using bounding boxes, nearest-app or mesh-divisions, please consider "
        "using the greedy_search to confirm. Then consider choosing a different transfer type.");
}

void
MultiAppGeneralFieldTransfer::cacheOutgoingPointInfo(const Point point,
                                                     const dof_id_type dof_object_id,
                                                     const unsigned int problem_id,
                                                     ProcessorToPointVec & outgoing_points)
{
  std::set<processor_id_type> processors;
  // Find which processors will receive point data so they can send back value data
  // The list can be larger than needed, depending on the heuristic / algorithm used to make
  // the call on whether a processor (and the apps it runs) should be involved
  processors.clear();
  locatePointReceivers(point, processors);

  // We need to send this location data to these processors so they can send back values
  for (const auto pid : processors)
  {
    // Select which from_mesh_division the source data must come from for this point
    unsigned int required_source_division = 0;
    if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX)
      required_source_division = getGlobalTargetAppIndex(problem_id);
    else if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
             _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
             _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX)
      required_source_division = _to_mesh_divisions[problem_id]->divisionIndex(
          _to_transforms[getGlobalTargetAppIndex(problem_id)]->mapBack(point));

    // Skip if we already know we don't want the point
    if (required_source_division == MooseMeshDivision::INVALID_DIVISION_INDEX)
      continue;

    // Store outgoing information for every source process
    outgoing_points[pid].push_back(std::pair<Point, unsigned int>(point, required_source_division));

    // Store point information locally for processing received data
    // We can use these information when inserting values into the solution vector
    PointInfo pointinfo;
    pointinfo.problem_id = problem_id;
    pointinfo.dof_object_id = dof_object_id;
    _processor_to_pointInfoVec[pid].push_back(pointinfo);
  }
}

void
MultiAppGeneralFieldTransfer::extractOutgoingPoints(const unsigned int var_index,
                                                    ProcessorToPointVec & outgoing_points)
{
  // Get the variable name, with the accommodation for array/vector names
  const auto & var_name = getToVarName(var_index);

  // Clean up the map from processor to pointInfo vector
  // This map should be consistent with outgoing_points
  _processor_to_pointInfoVec.clear();

  // Loop over all problems
  for (const auto i_to : index_range(_to_problems))
  {
    const auto global_i_to = getGlobalTargetAppIndex(i_to);

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

      // Defining only boundary values will not be enough to describe the variable, disallow it
      if (_to_boundaries.size() && (_to_variables[var_index]->getContinuity() == DISCONTINUOUS))
        mooseError("Higher order discontinuous elemental variables are not supported for "
                   "target-boundary "
                   "restricted transfers");

      // Not implemented as the target mesh division could similarly be cutting elements in an
      // arbitrary way with not enough requested points to describe the target variable
      if (!_to_mesh_divisions.empty() && !_to_mesh_divisions[i_to]->coversEntireMesh())
        mooseError("Higher order variable support not implemented for target mesh division "
                   "unless the mesh is fully covered / indexed in the mesh division. This must be "
                   "set programmatically in the MeshDivision object used.");

      // We dont look at boundary restriction, not supported for higher order target variables
      // Same for mesh divisions
      const auto & to_begin = _to_blocks.empty()
                                  ? to_mesh.active_local_elements_begin()
                                  : to_mesh.active_local_subdomain_set_elements_begin(_to_blocks);

      const auto & to_end = _to_blocks.empty()
                                ? to_mesh.active_local_elements_end()
                                : to_mesh.active_local_subdomain_set_elements_end(_to_blocks);

      ConstElemRange to_elem_range(to_begin, to_end);

      request_gather.project(to_elem_range);

      dof_id_type point_id = 0;
      for (const Point & p : f.points_requested())
        // using the point number as a "dof_object_id" will serve to identify the point if we ever
        // rework interp/distance_cache into the dof_id_to_value maps
        this->cacheOutgoingPointInfo(
            (*_to_transforms[global_i_to])(p), point_id++, i_to, outgoing_points);

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

        // Skip if the node does not meet the target mesh division behavior
        // We cannot know from which app the data will come from so we cannot know
        // the source mesh division index and the source app global index
        if (!_to_mesh_divisions.empty() && _to_mesh_divisions[i_to]->divisionIndex(*node) ==
                                               MooseMeshDivision::INVALID_DIVISION_INDEX)
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfo(
            (*_to_transforms[global_i_to])(*node), node->id(), i_to, outgoing_points);
      }
    }
    else // Elemental, constant monomial
    {
      for (const auto & elem :
           as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if the element is not inside the block restriction
        if (!_to_blocks.empty() && !inBlocks(_to_blocks, elem))
          continue;

        // Skip if the element does not have a side on the boundary
        if (!_to_boundaries.empty() && !onBoundaries(_to_boundaries, to_moose_mesh, elem))
          continue;

        // Skip if the element is not indexed within the mesh division
        if (!_to_mesh_divisions.empty() && _to_mesh_divisions[i_to]->divisionIndex(*elem) ==
                                               MooseMeshDivision::INVALID_DIVISION_INDEX)
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfo((*_to_transforms[global_i_to])(elem->vertex_average()),
                               elem->id(),
                               i_to,
                               outgoing_points);
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
  for (const auto i_from : make_range(_froms_per_proc[processor_id()]))
    local_bboxes[i_from] = _from_bboxes[local_start + i_from];
}

void
MultiAppGeneralFieldTransfer::cacheIncomingInterpVals(
    processor_id_type pid,
    const unsigned int var_index,
    std::vector<PointInfo> & pointInfoVec,
    const std::vector<std::pair<Point, unsigned int>> & point_requests,
    const std::vector<std::pair<Real, Real>> & incoming_vals,
    DofobjectToInterpValVec & dofobject_to_valsvec,
    InterpCaches & interp_caches,
    InterpCaches & distance_caches)
{
  mooseAssert(pointInfoVec.size() == incoming_vals.size(),
              "Number of dof objects does not equal to the number of incoming values");

  dof_id_type val_offset = 0;
  for (const auto & pointinfo : pointInfoVec)
  {
    // Retrieve target information from cached point infos
    const auto problem_id = pointinfo.problem_id;
    const auto dof_object_id = pointinfo.dof_object_id;

    auto & fe_type = _to_variables[var_index]->feType();
    bool is_nodal = _to_variables[var_index]->isNodal();

    // In the higher order elemental variable case, we receive point values, not nodal or
    // elemental. We use an InterpCache to store the values. The distance_cache is necessary to
    // choose between multiple origin problems sending values. This code could be unified with the
    // lower order order case by using the dofobject_to_valsvec
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      // Cache solution on target mesh in its local frame of reference
      InterpCache & value_cache = interp_caches[problem_id];
      InterpCache & distance_cache = distance_caches[problem_id];
      Point p = _to_transforms[getGlobalTargetAppIndex(problem_id)]->mapBack(
          point_requests[val_offset].first);
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

      // if we use the nearest app, even if the value is bad we want to save the distance because
      // it's the distance to the app, if that's the closest app then so be it with the bad value
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
      // Using the dof object pointer, so we can handle
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

        // Look for value conflicts
        if (detectConflict(val.interp,
                           incoming_vals[val_offset].first,
                           val.distance,
                           incoming_vals[val_offset].second))
        {
          // Keep track of distance and value
          const Point p =
              getPointInTargetAppFrame(point_requests[val_offset].first,
                                       problem_id,
                                       "Registration of received equi-distant value conflict");
          registerConflict(problem_id, dof_object_id, p, incoming_vals[val_offset].second, false);
        }

        // We adopt values that are, in order of priority
        // - valid (or from nearest app)
        // - closest distance
        // - the smallest rank with the same distance
        // It is debatable whether we want invalid values from the nearest app. It could just be
        // that the app position was closer but the extent of another child app was large enough
        if ((!GeneralFieldTransfer::isBetterOutOfMeshValue(incoming_vals[val_offset].first) ||
             _use_nearest_app) &&
            (MooseUtils::absoluteFuzzyGreaterThan(val.distance, incoming_vals[val_offset].second) ||
             ((val.pid > pid) &&
              MooseUtils::absoluteFuzzyEqual(val.distance, incoming_vals[val_offset].second))))
        {
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
MultiAppGeneralFieldTransfer::examineReceivedValueConflicts(
    const unsigned int var_index,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & distance_caches)
{
  const auto var_name = getToVarName(var_index);
  // We must check a posteriori because we could have two
  // equidistant points with different values from two different problems, but a third point from
  // another problem is actually closer, so there is no conflict because only that last one
  // matters We check here whether the potential conflicts actually were the nearest points Loop
  // over potential conflicts
  for (auto conflict_it = _received_conflicts.begin(); conflict_it != _received_conflicts.end();)
  {
    const auto potential_conflict = *conflict_it;
    bool overlap_found = false;

    // Extract info for the potential conflict
    const unsigned int problem_id = std::get<0>(potential_conflict);
    const dof_id_type dof_object_id = std::get<1>(potential_conflict);
    const Point p = std::get<2>(potential_conflict);
    const Real distance = std::get<3>(potential_conflict);

    // Extract target variable info
    auto & es = getEquationSystem(*_to_problems[problem_id], _displaced_target_mesh);
    System * to_sys = find_sys(es, var_name);
    auto var_num = to_sys->variable_number(var_name);
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = _to_variables[var_index]->isNodal();

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
      _received_conflicts.erase(conflict_it);
    else
      ++conflict_it;
  }
}

void
MultiAppGeneralFieldTransfer::examineLocalValueConflicts(
    const unsigned int var_index,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & distance_caches)
{
  const auto var_name = getToVarName(var_index);
  // We must check a posteriori because we could have:
  // - two equidistant points with different values from two different problems
  // - two (or more) equidistant points with different values from the same problem
  // but a third point/value couple from another problem is actually closer, so there is no
  // conflict because only that last one matters. We check here whether the potential conflicts
  // actually were the nearest points. We use several global reductions. If there are not too many
  // potential conflicts (and there should not be in a well-posed problem) it should be manageably
  // expensive

  // Move relevant conflict info (location, distance) to a smaller data structure
  std::vector<std::tuple<Point, Real>> potential_conflicts;
  potential_conflicts.reserve(_local_conflicts.size());

  // Loop over potential conflicts to broadcast all the conflicts
  for (auto conflict_it = _local_conflicts.begin(); conflict_it != _local_conflicts.end();
       ++conflict_it)
  {
    // Extract info for the potential conflict
    const auto potential_conflict = *conflict_it;
    const unsigned int i_from = std::get<0>(potential_conflict);
    Point p = std::get<2>(potential_conflict);
    const Real distance = std::get<3>(potential_conflict);
    // If not using nearest-positions: potential conflict was saved in the source frame
    // If using nearest-positions: potential conflict was saved in the reference frame
    if (!_nearest_positions_obj)
    {
      const auto from_global_num = getGlobalSourceAppIndex(i_from);
      p = (*_from_transforms[from_global_num])(p);
    }

    // Send data in the global frame of reference
    potential_conflicts.push_back(std::make_tuple(p, distance));
  }
  _communicator.allgather(potential_conflicts, false);
  // conflicts could have been reported multiple times within a tolerance
  std::sort(potential_conflicts.begin(), potential_conflicts.end());
  potential_conflicts.erase(unique(potential_conflicts.begin(),
                                   potential_conflicts.end(),
                                   [](auto l, auto r)
                                   {
                                     return std::get<0>(l).absolute_fuzzy_equals(std::get<0>(r)) &&
                                            std::abs(std::get<1>(l) - std::get<1>(r)) < TOLERANCE;
                                   }),
                            potential_conflicts.end());

  std::vector<std::tuple<Point, Real>> real_conflicts;
  real_conflicts.reserve(potential_conflicts.size());

  // For each potential conflict, we need to identify what problem asked for that value
  for (auto conflict_it = potential_conflicts.begin(); conflict_it != potential_conflicts.end();
       ++conflict_it)
  {
    // Extract info for the potential conflict
    auto potential_conflict = *conflict_it;
    const Point p = std::get<0>(potential_conflict);
    const Real distance = std::get<1>(potential_conflict);

    // Check all the problems to try to find this requested point in the data structures filled
    // with the received information
    bool target_found = false;
    bool conflict_real = false;
    for (const auto i_to : index_range(_to_problems))
    {
      // Extract variable info
      auto & es = getEquationSystem(*_to_problems[i_to], _displaced_target_mesh);
      System * to_sys = find_sys(es, var_name);
      auto var_num = to_sys->variable_number(var_name);
      auto & fe_type = to_sys->variable_type(var_num);
      bool is_nodal = _to_variables[var_index]->isNodal();

      // Move to the local frame of reference for the target problem
      Point local_p =
          getPointInTargetAppFrame(p, i_to, "Resolution of local value conflicts detected");

      // Higher order elemental
      if (fe_type.order > CONSTANT && !is_nodal)
      {
        // distance_caches finds use a binned floating point search
        auto cached_distance = distance_caches[i_to].find(local_p);
        if (cached_distance != distance_caches[i_to].end())
        {
          target_found = true;
          // Distance between source & target is still the distance we found in the sending
          // process when we detected a potential overlap while gathering values to send
          if (MooseUtils::absoluteFuzzyEqual(cached_distance->second, distance))
            conflict_real = true;
        }
      }
      // Nodal-value-dof-only and const monomial variable
      else
      {
        // Find the dof id for the variable to be set
        dof_id_type dof_object_id = std::numeric_limits<dof_id_type>::max();
        auto pl = _to_problems[i_to]->mesh().getPointLocator();
        pl->enable_out_of_mesh_mode();
        if (is_nodal)
        {
          auto node = pl->locate_node(local_p);
          if (node)
            // this is not the dof_id for the variable, but the dof_object_id
            dof_object_id = node->id();
        }
        else
        {
          auto elem = (*pl)(local_p);
          if (elem)
            dof_object_id = elem->id();
        }
        pl->disable_out_of_mesh_mode();

        // point isn't even in mesh
        if (dof_object_id == std::numeric_limits<dof_id_type>::max())
          continue;

        // this dof was not requested by this problem on this process
        if (dofobject_to_valsvec[i_to].find(dof_object_id) == dofobject_to_valsvec[i_to].end())
          continue;

        target_found = true;
        // Check the saved distance in the vector of saved results. If the same, then the local
        // conflict we detected with that distance is still an issue after receiving all values
        if (MooseUtils::absoluteFuzzyEqual(
                dofobject_to_valsvec[i_to].find(dof_object_id)->second.distance, distance))
          conflict_real = true;
      }
    }
    // Only keep the actual conflicts / overlaps
    if (target_found && conflict_real)
      real_conflicts.push_back(potential_conflict);
  }

  // Communicate real conflicts to all so they can be checked by every process
  _communicator.allgather(real_conflicts, false);

  // Delete potential conflicts that were resolved
  // Each local list of conflicts will now be updated. It's important to keep conflict lists local
  // so we can give more context like the sending processor id (the domain of which can be
  // inspected by the user)
  for (auto conflict_it = _local_conflicts.begin(); conflict_it != _local_conflicts.end();)
  {
    // Extract info for the potential conflict
    const auto potential_conflict = *conflict_it;
    const unsigned int i_from = std::get<0>(potential_conflict);
    Point p = std::get<2>(potential_conflict);
    const Real distance = std::get<3>(potential_conflict);
    if (!_nearest_positions_obj)
    {
      const auto from_global_num = getGlobalSourceAppIndex(i_from);
      p = (*_from_transforms[from_global_num])(p);
    }

    // If not in the vector of real conflicts, was not real so delete it
    if (std::find_if(real_conflicts.begin(),
                     real_conflicts.end(),
                     [p, distance](const auto & item)
                     {
                       return std::get<0>(item).absolute_fuzzy_equals(p) &&
                              std::abs(std::get<1>(item) - distance) < TOLERANCE;
                     }) == real_conflicts.end())
      _local_conflicts.erase(conflict_it);
    else
      ++conflict_it;
  }
}

void
MultiAppGeneralFieldTransfer::outputValueConflicts(
    const unsigned int var_index,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & distance_caches)
{
  // Remove potential conflicts that did not materialize, the value did not end up being used
  examineReceivedValueConflicts(var_index, dofobject_to_valsvec, distance_caches);
  examineLocalValueConflicts(var_index, dofobject_to_valsvec, distance_caches);

  // Output the conflicts from the selection of local values (evaluateInterpValues-type routines)
  // to send in response to value requests at target points
  const std::string rank_str = std::to_string(_communicator.rank());
  if (_local_conflicts.size())
  {
    unsigned int num_outputs = 0;
    std::string local_conflicts_string = "";
    std::string potential_reasons =
        "Are some points in target mesh equidistant from the sources "
        "(nodes/centroids/apps/positions, depending on transfer) in origin mesh(es)?\n";
    if (hasFromMultiApp() && _from_problems.size() > 1)
      potential_reasons += "Are multiple subapps overlapping?\n";
    for (const auto & conflict : _local_conflicts)
    {
      const unsigned int problem_id = std::get<0>(conflict);
      Point p = std::get<2>(conflict);
      num_outputs++;

      std::string origin_domain_message;
      if (hasFromMultiApp() && !_nearest_positions_obj)
      {
        // NOTES:
        // - The origin app for a conflict may not be unique.
        // - The conflicts vectors only store the conflictual points, not the original one
        //   The original value found with a given distance could be retrieved from the main
        //   caches
        const auto app_id = _from_local2global_map[problem_id];
        origin_domain_message = "In source child app " + std::to_string(app_id) + " mesh,";
      }
      // We can't locate the source app when considering nearest positions, so we saved the data
      // in the reference space. So we return the conflict location in the target app (parent or
      // sibling) instead
      else if (hasFromMultiApp() && _nearest_positions_obj)
      {
        if (_to_problems.size() == 1 || _skip_coordinate_collapsing)
        {
          p = (*_to_transforms[0])(p);
          origin_domain_message = "In target app mesh,";
        }
        else
          origin_domain_message = "In reference (post-coordinate collapse) mesh,";
      }
      else
        origin_domain_message = "In source parent app mesh,";

      if (num_outputs < _search_value_conflicts_max_log)
        local_conflicts_string += origin_domain_message + " point: (" + std::to_string(p(0)) +
                                  ", " + std::to_string(p(1)) + ", " + std::to_string(p(2)) +
                                  "), equi-distance: " + std::to_string(std::get<3>(conflict)) +
                                  "\n";
      else if (num_outputs == _search_value_conflicts_max_log)
        local_conflicts_string +=
            "Maximum output of the search for value conflicts has been reached. Further conflicts "
            "will not be output.\nIncrease 'search_value_conflicts_max_log' to output more.";
    }
    // Explicitly name source to give more context
    std::string source_str = "unknown";
    if (_from_var_names.size())
      source_str = "variable '" + getFromVarName(var_index) + "'";
    else if (isParamValid("source_user_object"))
      source_str = "user object '" + getParam<UserObjectName>("source_user_object") + "'";

    mooseWarning("On rank " + rank_str +
                 ", multiple valid values from equidistant points were "
                 "found in the origin mesh for source " +
                 source_str + " for " + std::to_string(_local_conflicts.size()) +
                 " target points.\n" + potential_reasons + "Conflicts detected at :\n" +
                 local_conflicts_string);
  }

  // Output the conflicts discovered when receiving values from multiple origin problems
  if (_received_conflicts.size())
  {
    unsigned int num_outputs = 0;
    std::string received_conflicts_string = "";
    std::string potential_reasons =
        "Are some points in target mesh equidistant from the sources "
        "(nodes/centroids/apps/positions, depending on transfer) in origin mesh(es)?\n";
    if (hasToMultiApp() && _to_problems.size() > 1)
      potential_reasons += "Are multiple subapps overlapping?\n";
    for (const auto & conflict : _received_conflicts)
    {
      // Extract info for the potential overlap
      const unsigned int problem_id = std::get<0>(conflict);
      const Point p = std::get<2>(conflict);
      num_outputs++;

      std::string target_domain_message;
      if (hasToMultiApp())
      {
        const auto app_id = _to_local2global_map[problem_id];
        target_domain_message = "In target child app " + std::to_string(app_id) + " mesh,";
      }
      else
        target_domain_message = "In target parent app mesh,";

      if (num_outputs < _search_value_conflicts_max_log)
        received_conflicts_string += target_domain_message + " point: (" + std::to_string(p(0)) +
                                     ", " + std::to_string(p(1)) + ", " + std::to_string(p(2)) +
                                     "), equi-distance: " + std::to_string(std::get<3>(conflict)) +
                                     "\n";
      else if (num_outputs == _search_value_conflicts_max_log)
        received_conflicts_string +=
            "Maximum output of the search for value conflicts has been reached. Further conflicts "
            "will not be output.\nIncrease 'search_value_conflicts_max_log' to output more.";
    }
    mooseWarning("On rank " + rank_str +
                 ", multiple valid values from equidistant points were "
                 "received for target variable '" +
                 getToVarName(var_index) + "' for " + std::to_string(_received_conflicts.size()) +
                 " target points.\n" + potential_reasons + "Conflicts detected at :\n" +
                 received_conflicts_string);
  }

  if (_local_conflicts.empty() && _received_conflicts.empty())
  {
    if (isParamSetByUser("search_value_conflict"))
      mooseInfo("Automated diagnosis did not detect floating point indetermination in transfer");
    else if (_to_problems.size() > 10 || _from_problems.size() > 10 || _communicator.size() > 10)
      mooseInfo(
          "Automated diagnosis did not detect any floating point indetermination in "
          "the transfer. You may consider turning it off using `search_value_conflicts=false` "
          "to improve performance/scalability.");
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
  // Get the variable name, with the accommodation for array/vector names
  const auto & var_name = getToVarName(var_index);

  for (const auto problem_id : index_range(_to_problems))
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
      for (const auto & val_pair : dofobject_to_val)
      {
        const auto dof_object_id = val_pair.first;

        const DofObject * dof_object = nullptr;
        if (is_nodal)
          dof_object = to_mesh.node_ptr(dof_object_id);
        else
          dof_object = to_mesh.elem_ptr(dof_object_id);

        const auto dof = dof_object->dof_number(sys_num, var_num, 0);
        const auto val = val_pair.second.interp;

        // This will happen if meshes are mismatched
        if (_error_on_miss && GeneralFieldTransfer::isBetterOutOfMeshValue(val))
        {
          const auto target_location =
              hasToMultiApp()
                  ? " on target app " + std::to_string(getGlobalTargetAppIndex(problem_id))
                  : " on parent app ";
          if (is_nodal)
            mooseError("No source value could be found for node ",
                       dof_object_id,
                       target_location,
                       "could not be located. Node details:\n",
                       _to_meshes[problem_id]->nodePtr(dof_object_id)->get_info());
          else
            mooseError("No source value could be found for element ",
                       dof_object_id,
                       target_location,
                       "could not be located. Element details:\n",
                       _to_meshes[problem_id]->elemPtr(dof_object_id)->get_info());
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
                                                      const Point & pt,
                                                      const unsigned int only_from_mesh_div,
                                                      Real & distance) const
{
  if (_use_bounding_boxes && !local_bboxes[i_from].contains_point(pt))
    return false;
  else
  {
    auto * pl = _from_point_locators[i_from].get();
    const auto from_global_num = getGlobalSourceAppIndex(i_from);
    const auto transformed_pt = _from_transforms[from_global_num]->mapBack(pt);

    // Check point against source block restriction
    if (!_from_blocks.empty() && !inBlocks(_from_blocks, pl, transformed_pt))
      return false;

    // Check point against source boundary restriction. Block restriction will speed up the search
    if (!_from_boundaries.empty() &&
        !onBoundaries(_from_boundaries, _from_blocks, *_from_meshes[i_from], pl, transformed_pt))
      return false;

    // Check point against the source mesh division
    if ((!_from_mesh_divisions.empty() || !_to_mesh_divisions.empty()) &&
        !acceptPointMeshDivision(transformed_pt, i_from, only_from_mesh_div))
      return false;

    // Get nearest position (often a subapp position) for the target point
    // We want values from the child app that is closest to the same position as the target
    Point nearest_position_source;
    if (_nearest_positions_obj)
    {
      const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;
      // The search for the nearest position is done in the reference frame
      const Point nearest_position = _nearest_positions_obj->getNearestPosition(pt, initial);
      nearest_position_source = _nearest_positions_obj->getNearestPosition(
          (*_from_transforms[from_global_num])(Point(0, 0, 0)), initial);

      if (!_skip_coordinate_collapsing &&
          _from_transforms[from_global_num]->hasNonTranslationTransformation())
        mooseError("Rotation and scaling currently unsupported with nearest positions transfer.");

      // Compute distance to nearest position and nearest position source
      const Real distance_to_position_nearest_source = (pt - nearest_position_source).norm();
      const Real distance_to_nearest_position = (pt - nearest_position).norm();

      // Source (usually app position) is not closest to the same positions as the target, dont
      // send values. We check the distance instead of the positions because if they are the same
      // that means there's two equidistant positions and we would want to capture that as a "value
      // conflict"
      if (!MooseUtils::absoluteFuzzyEqual(distance_to_position_nearest_source,
                                          distance_to_nearest_position))
        return false;

      // Set the distance as the distance from the nearest position to the target point
      distance = distance_to_position_nearest_source;
    }

    // Check that the app actually contains the origin point
    // We dont need to check if we already found it in a block or a boundary
    if (_from_blocks.empty() && _from_boundaries.empty() && _source_app_must_contain_point &&
        !inMesh(pl, transformed_pt))
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
    for (const auto side : make_range(elem->n_sides()))
    {
      bnd_info.boundary_ids(elem, side, vec_to_fill_temp);
      vec_to_fill.insert(vec_to_fill.end(), vec_to_fill_temp.begin(), vec_to_fill_temp.end());
    }
  else
    for (const auto node_index : make_range(elem->n_nodes()))
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
MultiAppGeneralFieldTransfer::acceptPointMeshDivision(
    const Point & pt, const unsigned int i_local, const unsigned int only_from_this_mesh_div) const
{
  // This routine can also be called to examine if the to_mesh_division index matches the current
  // source subapp index
  unsigned int source_mesh_div = MooseMeshDivision::INVALID_DIVISION_INDEX - 1;
  if (!_from_mesh_divisions.empty())
    source_mesh_div = _from_mesh_divisions[i_local]->divisionIndex(pt);

  // If the point is not indexed in the source division
  if (!_from_mesh_divisions.empty() && source_mesh_div == MooseMeshDivision::INVALID_DIVISION_INDEX)
    return false;
  // If the point is not the at the same index in the target and the origin meshes, reject
  else if ((_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
            _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX) &&
           source_mesh_div != only_from_this_mesh_div)
    return false;
  // If the point is at a certain division index that is not the same as the index of the subapp
  // we wanted the information to be from for that point, reject
  else if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX &&
           source_mesh_div != only_from_this_mesh_div)
    return false;
  else if (_to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX &&
           only_from_this_mesh_div != getGlobalSourceAppIndex(i_local))
    return false;
  else
    return true;
}

bool
MultiAppGeneralFieldTransfer::closestToPosition(unsigned int pos_index, const Point & pt) const
{
  mooseAssert(_nearest_positions_obj, "Should not be here without a positions object");
  if (!_skip_coordinate_collapsing)
    paramError("skip_coordinate_collapsing", "Coordinate collapsing not implemented");
  bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;
  if (!_search_value_conflicts)
    // Faster to just compare the index
    return pos_index == _nearest_positions_obj->getNearestPositionIndex(pt, initial);
  else
  {
    // Get the distance to the position and see if we are missing a value just because the position
    // is not officially the closest, but it is actually at the same distance
    const auto nearest_position = _nearest_positions_obj->getNearestPosition(pt, initial);
    const auto nearest_position_at_index = _nearest_positions_obj->getPosition(pos_index, initial);
    Real distance_to_position_at_index = (pt - nearest_position_at_index).norm();
    const Real distance_to_nearest_position = (pt - nearest_position).norm();

    if (!MooseUtils::absoluteFuzzyEqual(distance_to_position_at_index,
                                        distance_to_nearest_position))
      return false;
    // Actually the same position (point)
    else if (nearest_position == nearest_position_at_index)
      return true;
    else
    {
      mooseWarning("Two equidistant positions ",
                   nearest_position,
                   " and ",
                   nearest_position_at_index,
                   " detected near point ",
                   pt);
      return true;
    }
  }
}

Real
MultiAppGeneralFieldTransfer::bboxMaxDistance(const Point & p, const BoundingBox & bbox) const
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
MultiAppGeneralFieldTransfer::bboxMinDistance(const Point & p, const BoundingBox & bbox) const
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
MultiAppGeneralFieldTransfer::getRestrictedFromBoundingBoxes() const
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

    for (const auto & elem : as_range(from_mesh.getMesh().local_elements_begin(),
                                      from_mesh.getMesh().local_elements_end()))
    {
      if (!_from_blocks.empty() && !inBlocks(_from_blocks, from_mesh, elem))
        continue;

      for (const auto & node : elem->node_ref_range())
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
      // Translate the bounding box to the from domain's position. We may have rotations so we
      // must be careful in constructing the new min and max (first and second)
      const auto from_global_num = getGlobalSourceAppIndex(j);
      transformBoundingBox(bbox, *_from_transforms[from_global_num]);
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

  // TODO move up
  // Check for a user-set fixed bounding box size and modify the sizes as appropriate
  if (_fixed_bbox_size != std::vector<Real>(3, 0))
    for (const auto i : make_range(LIBMESH_DIM))
      if (!MooseUtils::absoluteFuzzyEqual(_fixed_bbox_size[i], 0))
        for (const auto j : make_range(bboxes.size()))
        {
          const auto current_width = (bboxes[j].second - bboxes[j].first)(i);
          bboxes[j].first(i) -= (_fixed_bbox_size[i] - current_width) / 2;
          bboxes[j].second(i) += (_fixed_bbox_size[i] - current_width) / 2;
        }

  return bboxes;
}

std::vector<unsigned int>
MultiAppGeneralFieldTransfer::getGlobalStartAppPerProc() const
{
  std::vector<unsigned int> global_app_start_per_proc(1, -1);
  if (_from_local2global_map.size())
    global_app_start_per_proc[0] = _from_local2global_map[0];
  _communicator.allgather(global_app_start_per_proc, true);
  return global_app_start_per_proc;
}

VariableName
MultiAppGeneralFieldTransfer::getFromVarName(unsigned int var_index)
{
  mooseAssert(var_index < _from_var_names.size(), "No source variable at this index");
  VariableName var_name = _from_var_names[var_index];
  if (_from_var_components.size())
    var_name += "_" + std::to_string(_from_var_components[var_index]);
  return var_name;
}

VariableName
MultiAppGeneralFieldTransfer::getToVarName(unsigned int var_index)
{
  mooseAssert(var_index < _to_var_names.size(), "No target variable at this index");
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

  for (const auto & to_mesh : _to_meshes)
  {
    const auto bbox = to_mesh->getInflatedProcessorBoundingBox();
    for (const auto dim : make_range(LIBMESH_DIM))
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
