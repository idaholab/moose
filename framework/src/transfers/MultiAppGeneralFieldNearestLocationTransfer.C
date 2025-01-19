//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldNearestLocationTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "Positions.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/system.h"

using namespace libMesh;

registerMooseObject("MooseApp", MultiAppGeneralFieldNearestLocationTransfer);
registerMooseObjectRenamed("MooseApp",
                           MultiAppGeneralFieldNearestNodeTransfer,
                           "12/31/2024 24:00",
                           MultiAppGeneralFieldNearestLocationTransfer);

InputParameters
MultiAppGeneralFieldNearestLocationTransfer::validParams()
{
  InputParameters params = MultiAppGeneralFieldTransfer::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position by finding the value at the nearest "
      "neighbor(s) in the origin application.");
  params.addParam<unsigned int>("num_nearest_points",
                                1,
                                "Number of nearest source (from) points will be chosen to "
                                "construct a value for the target point. All points will be "
                                "selected from the same origin mesh!");

  // Nearest node is historically more an extrapolation transfer
  params.set<Real>("extrapolation_constant") = GeneralFieldTransfer::BetterOutOfMeshValue;
  params.suppressParameter<Real>("extrapolation_constant");
  // We dont keep track of both point distance to app and to nearest node, so we cannot guarantee
  // that the nearest app (among the local apps, not globally) will hold the nearest location.
  // However, if the user knows this is true, we can use this heuristic to reduce the number of apps
  // that are requested to provide a candidate value. If the user is wrong, then the nearest
  // location is used, which can be from the non-nearest app.
  params.renameParam("use_nearest_app", "assume_nearest_app_holds_nearest_location", "");

  // the default of node/centroid switching based on the variable is causing lots of mistakes and
  // bad results
  std::vector<MooseEnum> source_types = {
      MooseEnum("nodes centroids variable_default", "variable_default")};
  params.addParam<std::vector<MooseEnum>>(
      "source_type", source_types, "Where to get the source values from for each source variable");

  // choose whether to include data from multiple apps when performing nearest-position/
  // mesh-divisions based transfers
  params.addParam<bool>("group_subapps",
                        false,
                        "Whether to group source locations and values from all subapps "
                        "when considering a nearest-position algorithm");

  return params;
}

MultiAppGeneralFieldNearestLocationTransfer::MultiAppGeneralFieldNearestLocationTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters),
    SolutionInvalidInterface(this),
    _num_nearest_points(getParam<unsigned int>("num_nearest_points")),
    _group_subapps(getParam<bool>("group_subapps"))
{
  if (_source_app_must_contain_point && _nearest_positions_obj)
    paramError("use_nearest_position",
               "We do not support using both nearest positions matching and checking if target "
               "points are within an app domain because the KDTrees for nearest-positions matching "
               "are (currently) built with data from multiple applications.");
  if (_nearest_positions_obj &&
      (isParamValid("from_mesh_divisions") || isParamValid("to_mesh_divisions")))
    paramError("use_nearest_position", "Cannot use nearest positions with mesh divisions");

  // Parameter checks on grouping subapp values
  if (_group_subapps && _from_mesh_divisions.empty() && !_nearest_positions_obj)
    paramError(
        "group_subapps",
        "This option is only available for using mesh divisions or nearest positions regions");
  else if (_group_subapps &&
           (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
            _from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX))
    paramError("group_subapps",
               "Cannot group subapps when considering nearest-location data as we would lose "
               "track of the division index of the source locations");
  else if (_group_subapps && _use_nearest_app)
    paramError(
        "group_subapps",
        "When using the 'nearest child application' data, the source data (positions and values) "
        "are grouped on a per-application basis, so it cannot be agglomerated over all child "
        "applications.\nNote that the option to use nearest applications for source restrictions, "
        "but further split each child application's domain by regions closest to each position "
        "(here the the child application's centroid), which could be conceived when "
        "'group_subapps' = false, is also not available.");
}

void
MultiAppGeneralFieldNearestLocationTransfer::initialSetup()
{
  MultiAppGeneralFieldTransfer::initialSetup();

  // Handle the source types ahead of time
  const auto & source_types = getParam<std::vector<MooseEnum>>("source_type");
  _source_is_nodes.resize(_from_var_names.size());
  _use_zero_dof_for_value.resize(_from_var_names.size());
  if (source_types.size() != _from_var_names.size())
    mooseError("Not enough source types specified for this number of variables. Source types must "
               "be specified for transfers with multiple variables");
  for (const auto var_index : index_range(_from_var_names))
  {
    // Local app does not own any of the source problems
    if (_from_problems.empty())
      break;

    // Get some info on the source variable
    FEProblemBase & from_problem = *_from_problems[0];
    MooseVariableFieldBase & from_var =
        from_problem.getVariable(0,
                                 _from_var_names[var_index],
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_ANY);
    System & from_sys = from_var.sys().system();
    const auto & fe_type = from_sys.variable_type(from_var.number());

    // Select where to get the origin values
    if (source_types[var_index] == "nodes")
      _source_is_nodes[var_index] = true;
    else if (source_types[var_index] == "centroids")
      _source_is_nodes[var_index] = false;
    else
    {
      // "Nodal" variables are continuous for sure at nodes
      if (from_var.isNodal())
        _source_is_nodes[var_index] = true;
      // for everyone else, we know they are continuous at centroids
      else
        _source_is_nodes[var_index] = false;
    }

    // Some variables can be sampled directly at their 0 dofs
    // - lagrange at nodes on a first order mesh
    // - anything constant and elemental obviously has the 0-dof value at the centroid (or
    // vertex-average). However, higher order elemental, even monomial, do not hold the centroid
    // value at dof index 0 For example: pyramid has dof 0 at the center of the base, prism has dof
    // 0 on an edge etc
    if ((_source_is_nodes[var_index] && fe_type.family == LAGRANGE &&
         !from_problem.mesh().hasSecondOrderElements()) ||
        (!_source_is_nodes[var_index] && fe_type.order == CONSTANT))
      _use_zero_dof_for_value[var_index] = true;
    else
      _use_zero_dof_for_value[var_index] = false;

    // Check with the source variable that it is not discontinuous at the source
    if (_source_is_nodes[var_index])
      if (from_var.getContinuity() == DISCONTINUOUS ||
          from_var.getContinuity() == SIDE_DISCONTINUOUS)
        mooseError("Source variable cannot be sampled at nodes as it is discontinuous");

    // Local app does not own any of the target problems
    if (_to_problems.empty())
      break;

    // Check with the target variable that we are not doing awful projections
    MooseVariableFieldBase & to_var =
        _to_problems[0]->getVariable(0,
                                     _to_var_names[var_index],
                                     Moose::VarKindType::VAR_ANY,
                                     Moose::VarFieldType::VAR_FIELD_ANY);
    System & to_sys = to_var.sys().system();
    const auto & to_fe_type = to_sys.variable_type(to_var.number());
    if (_source_is_nodes[var_index])
    {
      if (to_fe_type.order == CONSTANT)
        mooseWarning(
            "Transfer is projecting from nearest-nodes to centroids. This is likely causing "
            "floating point indetermination in the results because multiple nodes are 'nearest' to "
            "a centroid. Please consider using a ProjectionAux to build an elemental source "
            "variable (for example constant monomial) before transferring");
    }
    else if (to_var.isNodal())
      mooseWarning(
          "Transfer is projecting from nearest-centroids to nodes. This is likely causing "
          "floating point indetermination in the results because multiple centroids are "
          "'nearest' to a node. Please consider using a ProjectionAux to build a nodal source "
          "variable (for example linear Lagrange) before transferring");
  }

  // We need to improve the indexing if we are to allow this
  if (!_from_mesh_divisions.empty())
    for (const auto mesh_div : _from_mesh_divisions)
      if (mesh_div->getNumDivisions() != _from_mesh_divisions[0]->getNumDivisions())
        paramError("from_mesh_division",
                   "This transfer has only been implemented with a uniform number of source mesh "
                   "divisions across all source applications");
}

void
MultiAppGeneralFieldNearestLocationTransfer::prepareEvaluationOfInterpValues(
    const unsigned int var_index)
{
  _local_kdtrees.clear();
  _local_points.clear();
  _local_values.clear();
  buildKDTrees(var_index);
}

void
MultiAppGeneralFieldNearestLocationTransfer::buildKDTrees(const unsigned int var_index)
{
  computeNumSources();
  const auto num_apps_per_tree = getNumAppsPerTree();
  _local_kdtrees.resize(_num_sources);
  _local_points.resize(_num_sources);
  _local_values.resize(_num_sources);
  unsigned int max_leaf_size = 0;

  // Construct a local KDTree for each source. A source can be a single app or multiple apps
  // combined (option for nearest-position / mesh-divisions)
  for (const auto i_source : make_range(_num_sources))
  {
    // Nest a loop on apps in case multiple apps contribute to the same KD-Tree source
    for (const auto app_i : make_range(num_apps_per_tree))
    {
      // Get the current app index
      const auto i_from = getAppIndex(i_source, app_i);
      // Current position index, if using nearest positions (not used for use_nearest_app)
      const auto i_pos = _group_subapps ? i_source : (i_source % getNumDivisions());

      // Get access to the variable and some variable information
      FEProblemBase & from_problem = *_from_problems[i_from];
      auto & from_mesh = from_problem.mesh(_displaced_source_mesh);
      MooseVariableFieldBase & from_var =
          from_problem.getVariable(0,
                                   _from_var_names[var_index],
                                   Moose::VarKindType::VAR_ANY,
                                   Moose::VarFieldType::VAR_FIELD_ANY);
      System & from_sys = from_var.sys().system();
      const unsigned int from_var_num = from_sys.variable_number(getFromVarName(var_index));

      // Build KDTree from nodes and nodal values
      if (_source_is_nodes[var_index])
      {
        for (const auto & node : from_mesh.getMesh().local_node_ptr_range())
        {
          if (node->n_dofs(from_sys.number(), from_var_num) < 1)
            continue;

          if (!_from_blocks.empty() && !inBlocks(_from_blocks, from_mesh, node))
            continue;

          if (!_from_boundaries.empty() && !onBoundaries(_from_boundaries, from_mesh, node))
            continue;

          // Handle the various source mesh divisions behaviors
          // NOTE: This could be more efficient, as instead of rejecting points in the
          // wrong division, we could just be adding them to the tree for the right division
          if (!_from_mesh_divisions.empty())
          {
            const auto tree_division_index = i_source % getNumDivisions();
            const auto node_div_index = _from_mesh_divisions[i_from]->divisionIndex(*node);

            // Spatial restriction is always active
            if (node_div_index == MooseMeshDivision::INVALID_DIVISION_INDEX)
              continue;
            // We fill one tree per division index for matching subapp index or division index. We
            // only accept source data from the division index
            else if ((_from_mesh_division_behavior ==
                          MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
                      _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
                      _from_mesh_division_behavior ==
                          MeshDivisionTransferUse::MATCH_SUBAPP_INDEX) &&
                     tree_division_index != node_div_index)
              continue;
          }

          // Transformed node is in the reference space, as is the _nearest_positions_obj
          const auto transformed_node = (*_from_transforms[getGlobalSourceAppIndex(i_from)])(*node);

          // Only add to the KDTree nodes that are closest to the 'position'
          // When querying values at a target point, the KDTree associated to the closest
          // position to the target point is queried
          // We do not need to check the positions when using nearest app as we will assume
          // (somewhat incorrectly) that all the points in each subapp are closer to that subapp
          // than to any other
          if (!_use_nearest_app && _nearest_positions_obj &&
              !closestToPosition(i_pos, transformed_node))
            continue;

          _local_points[i_source].push_back(transformed_node);
          const auto dof = node->dof_number(from_sys.number(), from_var_num, 0);
          _local_values[i_source].push_back((*from_sys.solution)(dof));
          if (!_use_zero_dof_for_value[var_index])
            flagInvalidSolution(
                "Nearest-location is not implemented for this source variable type on "
                "this mesh. Returning value at dof 0");
        }
      }
      // Build KDTree from centroids and value at centroids
      else
      {
        for (auto & elem : as_range(from_mesh.getMesh().local_elements_begin(),
                                    from_mesh.getMesh().local_elements_end()))
        {
          if (elem->n_dofs(from_sys.number(), from_var_num) < 1)
            continue;

          if (!_from_blocks.empty() && !inBlocks(_from_blocks, elem))
            continue;

          if (!_from_boundaries.empty() && !onBoundaries(_from_boundaries, from_mesh, elem))
            continue;

          // Handle the various source mesh divisions behaviors
          if (!_from_mesh_divisions.empty())
          {
            const auto tree_division_index = i_source % getNumDivisions();
            const auto elem_div_index = _from_mesh_divisions[i_from]->divisionIndex(*elem);

            // Spatial restriction is always active
            if (elem_div_index == MooseMeshDivision::INVALID_DIVISION_INDEX)
              continue;
            // We fill one tree per division index for matching subapp index or division index. We
            // only accept source data from the division index
            else if ((_from_mesh_division_behavior ==
                          MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
                      _from_mesh_division_behavior ==
                          MeshDivisionTransferUse::MATCH_SUBAPP_INDEX) &&
                     tree_division_index != elem_div_index)
              continue;
          }

          const auto vertex_average = elem->vertex_average();
          // Transformed element vertex average is in the reference space, as is the
          // _nearest_positions_obj
          const auto transformed_vertex_average =
              (*_from_transforms[getGlobalSourceAppIndex(i_from)])(vertex_average);

          // We do not need to check the positions when using nearest app as we will assume
          // (somewhat incorrectly) that all the points in each subapp are closer to that subapp
          if (!_use_nearest_app && _nearest_positions_obj &&
              !closestToPosition(i_pos, transformed_vertex_average))
            continue;

          _local_points[i_source].push_back(transformed_vertex_average);
          if (_use_zero_dof_for_value[var_index])
          {
            auto dof = elem->dof_number(from_sys.number(), from_var_num, 0);
            _local_values[i_source].push_back((*from_sys.solution)(dof));
          }
          else
            _local_values[i_source].push_back(
                from_sys.point_value(from_var_num, vertex_average, elem));
        }
      }
      max_leaf_size = std::max(max_leaf_size, from_mesh.getMaxLeafSize());
    }

    // Make a KDTree from the accumulated points data
    std::shared_ptr<KDTree> _kd_tree =
        std::make_shared<KDTree>(_local_points[i_source], max_leaf_size);
    _local_kdtrees[i_source] = _kd_tree;
  }
}

void
MultiAppGeneralFieldNearestLocationTransfer::evaluateInterpValues(
    const std::vector<std::pair<Point, unsigned int>> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateInterpValuesNearestNode(incoming_points, outgoing_vals);
}

void
MultiAppGeneralFieldNearestLocationTransfer::evaluateInterpValuesNearestNode(
    const std::vector<std::pair<Point, unsigned int>> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (const auto & [pt, mesh_div] : incoming_points)
  {
    // Reset distance
    outgoing_vals[i_pt].second = std::numeric_limits<Real>::max();
    bool point_found = false;

    // Loop on all sources
    for (const auto i_from : make_range(_num_sources))
    {
      // Examine all restrictions for the point. This source (KDTree+values) could be ruled out
      if (!checkRestrictionsForSource(pt, mesh_div, i_from))
        continue;

      // TODO: Pre-allocate these two work arrays. They will be regularly resized by the
      // searches
      std::vector<std::size_t> return_index(_num_nearest_points);
      std::vector<Real> return_dist_sqr(_num_nearest_points);

      // KD Tree can be empty if no points are within block/boundary/bounding box restrictions
      if (_local_kdtrees[i_from]->numberCandidatePoints())
      {
        point_found = true;
        // Note that we do not need to use the transformed_pt (in the source app frame)
        // because the KDTree has been created in the reference frame
        _local_kdtrees[i_from]->neighborSearch(
            pt, _num_nearest_points, return_index, return_dist_sqr);
        Real val_sum = 0, dist_sum = 0;
        for (const auto index : return_index)
        {
          val_sum += _local_values[i_from][index];
          dist_sum += (_local_points[i_from][index] - pt).norm();
        }

        // If the new value found is closer than for other sources, use it
        const auto new_distance = dist_sum / return_dist_sqr.size();
        if (new_distance < outgoing_vals[i_pt].second)
          outgoing_vals[i_pt] = {val_sum / return_index.size(), new_distance};
      }
    }

    // none of the source problem meshes were within the restrictions set
    if (!point_found)
      outgoing_vals[i_pt] = {GeneralFieldTransfer::BetterOutOfMeshValue,
                             GeneralFieldTransfer::BetterOutOfMeshValue};
    else if (_search_value_conflicts)
    {
      // Local source meshes conflicts can happen two ways:
      // - two (or more) problems' nearest nodes are equidistant to the target point
      // - within a problem, the num_nearest_points'th closest node is as close to the target
      //   point as the num_nearest_points + 1'th closest node
      unsigned int num_equidistant_problems = 0;

      for (const auto i_from : make_range(_num_sources))
      {
        // Examine all restrictions for the point. This source (KDTree+values) could be ruled out
        if (!checkRestrictionsForSource(pt, mesh_div, i_from))
          continue;

        // TODO: Pre-allocate these two work arrays. They will be regularly resized by the searches
        std::vector<std::size_t> return_index(_num_nearest_points + 1);
        std::vector<Real> return_dist_sqr(_num_nearest_points + 1);

        // NOTE: app_index is not valid if _group_subapps = true
        const auto app_index = i_from / getNumDivisions();
        const auto num_search = _num_nearest_points + 1;

        if (_local_kdtrees[i_from]->numberCandidatePoints())
        {
          _local_kdtrees[i_from]->neighborSearch(pt, num_search, return_index, return_dist_sqr);
          auto num_found = return_dist_sqr.size();

          // Local coordinates only accessible when not using nearest-position
          // as we did not keep the index of the source app, only the position index
          const Point local_pt = getPointInLocalSourceFrame(app_index, pt);

          if (!_nearest_positions_obj &&
              _from_transforms[getGlobalSourceAppIndex(app_index)]->hasCoordinateSystemTypeChange())
            if (!_skip_coordinate_collapsing)
              mooseInfo("Search value conflict cannot find the origin point due to the "
                        "non-uniqueness of the coordinate collapsing reverse mapping");

          // Look for too many equidistant nodes within a problem. First zip then sort by distance
          std::vector<std::pair<Real, std::size_t>> zipped_nearest_points;
          for (const auto i : make_range(num_found))
            zipped_nearest_points.push_back(std::make_pair(return_dist_sqr[i], return_index[i]));
          std::sort(zipped_nearest_points.begin(), zipped_nearest_points.end());

          // If two furthest are equally far from target point, then we have an indetermination in
          // what is sent in this communication round from this process. However, it may not
          // materialize to an actual conflict, as values sent from another process for the
          // desired target point could be closer (nearest). There is no way to know at this point
          // in the communication that a closer value exists somewhere else
          if (num_found > 1 && num_found == num_search &&
              MooseUtils::absoluteFuzzyEqual(zipped_nearest_points[num_found - 1].first,
                                             zipped_nearest_points[num_found - 2].first))
          {
            if (_nearest_positions_obj)
              registerConflict(app_index, 0, pt, outgoing_vals[i_pt].second, true);
            else
              registerConflict(app_index, 0, local_pt, outgoing_vals[i_pt].second, true);
          }

          // Recompute the distance for this problem. If it matches the cached value more than
          // once it means multiple problems provide equidistant values for this point
          Real dist_sum = 0;
          for (const auto i : make_range(num_search - 1))
          {
            auto index = zipped_nearest_points[i].second;
            dist_sum += (_local_points[i_from][index] - pt).norm();
          }

          // Compare to the selected value found after looking at all the problems
          if (MooseUtils::absoluteFuzzyEqual(dist_sum / return_dist_sqr.size(),
                                             outgoing_vals[i_pt].second))
          {
            num_equidistant_problems++;
            if (num_equidistant_problems > 1)
            {
              if (_nearest_positions_obj)
                registerConflict(app_index, 0, pt, outgoing_vals[i_pt].second, true);
              else
                registerConflict(app_index, 0, local_pt, outgoing_vals[i_pt].second, true);
            }
          }
        }
      }
    }

    // Move to next point
    i_pt++;
  }
}

bool
MultiAppGeneralFieldNearestLocationTransfer::inBlocks(const std::set<SubdomainID> & blocks,
                                                      const MooseMesh & mesh,
                                                      const Elem * elem) const
{
  // We need to override the definition of block restriction for an element
  // because we have to consider whether each node of an element is adjacent to a block
  for (const auto & i_node : make_range(elem->n_nodes()))
  {
    const auto & node = elem->node_ptr(i_node);
    const std::set<SubdomainID> & node_blocks = mesh.getNodeBlockIds(*node);
    std::set<SubdomainID> u;
    std::set_intersection(blocks.begin(),
                          blocks.end(),
                          node_blocks.begin(),
                          node_blocks.end(),
                          std::inserter(u, u.begin()));
    if (!u.empty())
      return true;
  }
  return false;
}

void
MultiAppGeneralFieldNearestLocationTransfer::computeNumSources()
{
  // Number of source = number of KDTrees.
  // Using mesh divisions or nearest-positions, for every app we use 1 tree per division
  if (!_from_mesh_divisions.empty() ||
      (!_use_nearest_app && _nearest_positions_obj && !_group_subapps))
    _num_sources = _from_problems.size() * getNumDivisions();
  // If we group apps, then we only use one tree per division (nearest-position region)
  else if (_nearest_positions_obj && _group_subapps)
    _num_sources = _nearest_positions_obj->getNumPositions(_fe_problem.getCurrentExecuteOnFlag() ==
                                                           EXEC_INITIAL);
  // Regular case: 1 KDTree per app
  // Also if use_nearest_app = true, the number of problems is better than the number of positions,
  // because some of the positions are positions of child applications that are not local
  else
    _num_sources = _from_problems.size();
}

unsigned int
MultiAppGeneralFieldNearestLocationTransfer::getAppIndex(
    unsigned int kdtree_index, unsigned int nested_loop_on_app_index) const
{
  // Each app is mapped to a single KD Tree
  if (_use_nearest_app)
    return kdtree_index;
  // We are looping over all the apps that are grouped together
  else if (_group_subapps)
    return nested_loop_on_app_index;
  // There are num_divisions trees for each app, inner ordering is divisions, so dividing by the
  // number of divisions gets us the index of the application
  else
    return kdtree_index / getNumDivisions();
}

unsigned int
MultiAppGeneralFieldNearestLocationTransfer::getNumAppsPerTree() const
{
  if (_use_nearest_app)
    return 1;
  else if (_group_subapps)
    return _from_meshes.size();
  else
    return 1;
}

unsigned int
MultiAppGeneralFieldNearestLocationTransfer::getNumDivisions() const
{
  // This is not used currently, but conceptually it is better to only divide the domain with the
  // local of local applications rather than the global number of positions (# global applications
  // here)
  if (_use_nearest_app)
    return _from_meshes.size();
  // Each nearest-position region is a division
  else if (_nearest_positions_obj && !_group_subapps)
    return _nearest_positions_obj->getNumPositions(_fe_problem.getCurrentExecuteOnFlag() ==
                                                   EXEC_INITIAL);
  // Assume all mesh divisions (on each sub-app) has the same number of divisions. This is checked
  else if (!_from_mesh_divisions.empty())
    return _from_mesh_divisions[0]->getNumDivisions();
  // Grouping subapps or no special mode, we do not subdivide
  else
    return 1;
}

Point
MultiAppGeneralFieldNearestLocationTransfer::getPointInLocalSourceFrame(unsigned int i_from,
                                                                        const Point & pt) const
{

  if (!_nearest_positions_obj &&
      (!_from_transforms[getGlobalSourceAppIndex(i_from)]->hasCoordinateSystemTypeChange() ||
       _skip_coordinate_collapsing))
    return _from_transforms[getGlobalSourceAppIndex(i_from)]->mapBack(pt);
  else if (!_nearest_positions_obj || !_group_subapps)
    return pt - _from_positions[i_from];
  else
    return pt;
}

bool
MultiAppGeneralFieldNearestLocationTransfer::checkRestrictionsForSource(
    const Point & pt, const unsigned int mesh_div, const unsigned int i_from) const
{
  // Only use the KDTree from the closest position if in "nearest-position" mode
  if (_nearest_positions_obj)
  {
    // See computeNumSources for the number of sources. i_from is the index in the source loop
    // i_from is local if looping on _from_problems as sources, positions are indexed globally
    // i_from is already indexing in positions if using group_subapps
    auto position_index = i_from; // if _group_subapps
    if (_use_nearest_app)
      position_index = getGlobalSourceAppIndex(i_from);
    else if (!_group_subapps)
      position_index = i_from % getNumDivisions();

    // NOTE: if two positions are equi-distant to the point, this will chose one
    // This problem is detected if using search_value_conflicts in this call
    if (!closestToPosition(position_index, pt))
      return false;
  }

  // Application index depends on which source/grouping mode we are using
  const unsigned int app_index = getAppIndex(i_from, i_from / getNumDivisions());

  // Check mesh restriction before anything
  if (_source_app_must_contain_point)
  {
    // We have to be careful that getPointInLocalSourceFrame returns in the reference frame
    if (_nearest_positions_obj)
      mooseError("Nearest-positions + source_app_must_contain_point not implemented");
    // Transform the point to place it in the local coordinate system
    const auto local_pt = getPointInLocalSourceFrame(app_index, pt);
    if (!inMesh(_from_point_locators[app_index].get(), local_pt))
      return false;
  }

  // Check the mesh division. We have handled the restriction of the source locations when
  // building the nearest-neighbor trees. We only need to check that we meet the required
  // source division index.
  if (!_from_mesh_divisions.empty())
  {
    mooseAssert(mesh_div != MooseMeshDivision::INVALID_DIVISION_INDEX,
                "We should not be receiving point requests with an invalid "
                "source mesh division index");
    const unsigned int kd_div_index = i_from % getNumDivisions();

    // If matching source mesh divisions to target apps, we check that the index of the target
    // application, which was passed in the point request, is equal to the current mesh division
    if (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX &&
        mesh_div != kd_div_index)
      return false;
    // If matching source mesh divisions to target mesh divisions, we check that the index of the
    // target mesh division, which was passed in the point request, is equal to the current mesh
    // division
    else if ((_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
              _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX) &&
             mesh_div != kd_div_index)
      return false;
  }

  // If matching target apps to source mesh divisions, we check that the global index of the
  // application is equal to the target mesh division index, which was passed in the point request
  if (_to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX &&
      mesh_div != getGlobalSourceAppIndex(app_index))
    return false;

  return true;
}
