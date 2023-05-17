//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldNearestNodeTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "Positions.h"

#include "libmesh/system.h"

registerMooseObject("MooseApp", MultiAppGeneralFieldNearestNodeTransfer);

InputParameters
MultiAppGeneralFieldNearestNodeTransfer::validParams()
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
  // We dont keep track of both point distance to app and to nearest node
  params.set<bool>("use_nearest_app") = false;
  params.suppressParameter<bool>("use_nearest_app");

  return params;
}

MultiAppGeneralFieldNearestNodeTransfer::MultiAppGeneralFieldNearestNodeTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters),
    _num_nearest_points(getParam<unsigned int>("num_nearest_points"))
{
  if (_source_app_must_contain_point && _nearest_positions_obj)
    paramError("use_nearest_position",
               "We do not support using both nearest positions matching and checking if target "
               "points are within an app domain because the KDTrees for nearest-positions matching "
               "are (currently) built with data from multiple applications.");
}

void
MultiAppGeneralFieldNearestNodeTransfer::prepareEvaluationOfInterpValues(
    const unsigned int var_index)
{
  _local_kdtrees.clear();
  _local_points.clear();
  _local_values.clear();
  buildKDTrees(var_index);
}

void
MultiAppGeneralFieldNearestNodeTransfer::buildKDTrees(const unsigned int var_index)
{
  const unsigned int num_sources =
      _nearest_positions_obj ? _nearest_positions_obj->getPositions(/*initial=*/false).size()
                             : _from_problems.size();

  _local_kdtrees.resize(num_sources);
  _local_points.resize(num_sources);
  _local_values.resize(num_sources);
  unsigned int max_leaf_size = 0;

  // Construct a local KDTree for each source (subapp or positions)
  for (unsigned int i_source = 0; i_source < num_sources; ++i_source)
  {
    // Nest a loop on apps only for the Positions case. Regular case only looks at one app
    const unsigned int num_apps_to_consider = _nearest_positions_obj ? _from_problems.size() : 1;
    for (const auto app_index : make_range(num_apps_to_consider))
    {
      const unsigned int i_from = !_nearest_positions_obj ? i_source : app_index;

      // NOTE: If we decide we do not want to mix apps when using the nearest-positions
      // we could continue here when the positions is not the nearest to the app

      FEProblemBase & from_problem = *_from_problems[i_from];
      auto & from_mesh = from_problem.mesh(_displaced_source_mesh);
      MooseVariableFieldBase & from_var =
          from_problem.getVariable(0,
                                   _from_var_names[var_index],
                                   Moose::VarKindType::VAR_ANY,
                                   Moose::VarFieldType::VAR_FIELD_ANY);

      System & from_sys = from_var.sys().system();
      const unsigned int from_var_num = from_sys.variable_number(getFromVarName(var_index));
      // FEM type info
      const auto & fe_type = from_sys.variable_type(from_var.number());
      const bool is_nodal = fe_type.family == LAGRANGE;

      if (is_nodal)
      {
        for (const auto & node : from_mesh.getMesh().local_node_ptr_range())
        {
          if (node->n_dofs(from_sys.number(), from_var_num) < 1)
            continue;

          if (!_from_blocks.empty() && !inBlocks(_from_blocks, from_mesh, node))
            continue;

          if (!_from_boundaries.empty() && !onBoundaries(_from_boundaries, from_mesh, node))
            continue;

          // Only add to the KDTree nodes that are closest to the 'position'
          // When querying values at a target point, the KDTree associated to the closest
          // position to the target point is queried
          if (_nearest_positions_obj &&
              !closestToPosition(i_source, *node + _from_positions[i_from]))
            continue;

          const auto dof = node->dof_number(from_sys.number(), from_var_num, 0);
          _local_points[i_source].push_back(*node + _from_positions[i_from]);
          _local_values[i_source].push_back((*from_sys.solution)(dof));
        }
      }
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

          if (_nearest_positions_obj &&
              !closestToPosition(i_source, elem->vertex_average() + _from_positions[i_from]))
            continue;

          const auto dof = elem->dof_number(from_sys.number(), from_var_num, 0);
          _local_points[i_source].push_back(elem->vertex_average() + _from_positions[i_from]);
          _local_values[i_source].push_back((*from_sys.solution)(dof));
        }
      }
      max_leaf_size = std::max(max_leaf_size, from_mesh.getMaxLeafSize());
    }

    // Make a KDTree, from a single app in regular case, from all points from all apps nearest
    // a position in the nearest_position case
    std::shared_ptr<KDTree> _kd_tree =
        std::make_shared<KDTree>(_local_points[i_source], max_leaf_size);
    _local_kdtrees[i_source] = _kd_tree;
  }
}

void
MultiAppGeneralFieldNearestNodeTransfer::evaluateInterpValues(
    const std::vector<Point> & incoming_points, std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateInterpValuesNearestNode(incoming_points, outgoing_vals);
}

void
MultiAppGeneralFieldNearestNodeTransfer::evaluateInterpValuesNearestNode(
    const std::vector<Point> & incoming_points, std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (auto & pt : incoming_points)
  {
    // Reset distance
    outgoing_vals[i_pt].second = std::numeric_limits<Real>::max();
    bool point_found = false;

    const unsigned int num_sources =
        !_nearest_positions_obj ? _from_problems.size()
                                : _nearest_positions_obj->getPositions(/*initial=*/false).size();

    // Loop on all sources
    for (const auto i_from : make_range(num_sources))
    {
      // Only use the KDTree from the closest position if in "nearest-position" mode
      if (_nearest_positions_obj && !closestToPosition(i_from, pt))
        continue;

      std::vector<std::size_t> return_index(_num_nearest_points);
      std::vector<Real> return_dist_sqr(_num_nearest_points);

      // Check mesh restriction before anything
      if (_source_app_must_contain_point && !inMesh(_from_point_locators[i_from].get(), pt))
        continue;

      // KD Tree can be empty if no points are within block/boundary/bounding box restrictions
      if (_local_kdtrees[i_from]->numberCandidatePoints())
      {
        point_found = true;
        _local_kdtrees[i_from]->neighborSearch(
            pt, _num_nearest_points, return_index, return_dist_sqr);
        Real val_sum = 0, dist_sum = 0;
        for (auto index : return_index)
        {
          val_sum += _local_values[i_from][index];
          dist_sum += (_local_points[i_from][index] - pt).norm();
        }
        // Pick the closest
        if (dist_sum / return_dist_sqr.size() < outgoing_vals[i_pt].second)
          outgoing_vals[i_pt] = {val_sum / return_index.size(), dist_sum / return_dist_sqr.size()};
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

      for (const auto i_from : make_range(num_sources))
      {
        // Only use the KDTree from the closest position if in "nearest-position" mode
        if (_nearest_positions_obj && !closestToPosition(i_from, pt))
          continue;

        unsigned int num_search = _num_nearest_points + 1;
        std::vector<std::size_t> return_index(num_search);
        std::vector<Real> return_dist_sqr(num_search);

        if (_local_kdtrees[i_from]->numberCandidatePoints())
        {
          _local_kdtrees[i_from]->neighborSearch(pt, num_search, return_index, return_dist_sqr);
          auto num_found = return_dist_sqr.size();

          // Local coordinates only accessible when not using nearest-position
          Point local_pt = pt;
          if (!_nearest_positions_obj)
            local_pt -= _from_positions[i_from];

          // Look for too many equidistant nodes within a problem. First zip then sort by distance
          std::vector<std::pair<Real, std::size_t>> zipped_nearest_points;
          for (auto i : make_range(num_found))
            zipped_nearest_points.push_back(std::make_pair(return_dist_sqr[i], return_index[i]));
          std::sort(zipped_nearest_points.begin(), zipped_nearest_points.end());

          // If two furthest are equally far from target point, then we have an indetermination
          if (num_found > 1 &&
              MooseUtils::absoluteFuzzyEqual(zipped_nearest_points[num_found - 1].first,
                                             zipped_nearest_points[num_found - 2].first))
            registerConflict(i_from, 0, local_pt, outgoing_vals[i_pt].second, true);

          Real val_sum = 0, dist_sum = 0;
          for (auto i : make_range(num_search - 1))
          {
            auto index = zipped_nearest_points[i].second;
            val_sum += _local_values[i_from][index];
            dist_sum += (_local_points[i_from][index] - pt).norm();
          }
          // Compare to the selected value found after looking at all the problems
          if (MooseUtils::absoluteFuzzyEqual(dist_sum / return_dist_sqr.size(),
                                             outgoing_vals[i_pt].second))
          {
            num_equidistant_problems++;
            if (num_equidistant_problems > 1)
              registerConflict(i_from, 0, local_pt, outgoing_vals[i_pt].second, true);
          }
        }
      }
    }

    // Move to next point
    i_pt++;
  }
}

bool
MultiAppGeneralFieldNearestNodeTransfer::inBlocks(const std::set<SubdomainID> & blocks,
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
