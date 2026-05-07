//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = MultiAppGeneralFieldKDTreeTransferBase::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position by finding the value at the nearest "
      "neighbor(s) in the origin application.");

  // Nearest node is historically more an extrapolation transfer
  params.set<Real>("extrapolation_constant") = GeneralFieldTransfer::OutOfMeshValue;
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

  return params;
}

MultiAppGeneralFieldNearestLocationTransfer::MultiAppGeneralFieldNearestLocationTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldKDTreeTransferBase(parameters)
{
}

void
MultiAppGeneralFieldNearestLocationTransfer::initialSetup()
{
  MultiAppGeneralFieldKDTreeTransferBase::initialSetup();

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
    // Higher order mesh is also fine incidentally because on the 'other' (mid-face for example)
    // nodes, the 1st order lagrange variable has 0 dofs, and the second order lagrange
    // use the mid-edge nodes as lagrange points (and 0 dofs on extra ones).
    // Third order is different (except on edge4, but not used much)
    // - anything constant and elemental obviously has the 0-dof value at the centroid (or
    // vertex-average). However, higher order elemental, even monomial, do not hold the centroid
    // value at dof index 0. For example: pyramid has dof 0 at the center of the base, prism has dof
    // 0 on an edge etc
    if ((_source_is_nodes[var_index] && fe_type.family == LAGRANGE && fe_type.order <= SECOND) ||
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
          // This notably excludes first order Lagrange variables on the mid-nodes
          // e.g. the mid-nodes are not added to the KD tree
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
            flagSolutionWarning(
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
    const unsigned int /*var_index*/,
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
    outgoing_vals[i_pt].second = std::numeric_limits<Real>::max();
    bool point_found = false;
    evaluateNearestNodeFromKDTrees(pt, mesh_div, outgoing_vals[i_pt], point_found);
    if (!point_found)
      outgoing_vals[i_pt] = {GeneralFieldTransfer::OutOfMeshValue,
                             GeneralFieldTransfer::OutOfMeshValue};
    i_pt++;
  }
}
