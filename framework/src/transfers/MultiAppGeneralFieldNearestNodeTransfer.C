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

  // Suppress all the options that are not considered
  params.suppressParameter<bool>("from_multiapp_must_contain_point");
  // Bounding boxes should still be used to locate the potential source problems

  return params;
}

MultiAppGeneralFieldNearestNodeTransfer::MultiAppGeneralFieldNearestNodeTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters),
    _num_nearest_points(getParam<unsigned int>("num_nearest_points"))
{
}

void
MultiAppGeneralFieldNearestNodeTransfer::prepareEvaluationOfInterpValues(
    const VariableName & var_name)
{
  _local_kdtrees.clear();
  _local_points.clear();
  _local_values.clear();
  buildKDTrees(var_name, _local_kdtrees, _local_points, _local_values);
}

void
MultiAppGeneralFieldNearestNodeTransfer::buildKDTrees(
    const VariableName & var_name,
    std::vector<std::shared_ptr<KDTree>> & kdtrees,
    std::vector<std::vector<Point>> & points,
    std::vector<std::vector<Real>> & local_values)
{
  kdtrees.resize(_from_problems.size());
  points.resize(_from_problems.size());
  local_values.resize(_from_problems.size());

  // Construct a local mesh for each problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    auto & from_mesh = from_problem.mesh(_displaced_source_mesh);
    MooseVariableFEBase & from_var = from_problem.getVariable(
        0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());
    // FEM type info
    auto & fe_type = from_sys.variable_type(from_var.number());
    bool is_nodal = fe_type.family == LAGRANGE;

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

        points[i_from].push_back(*node + _from_positions[i_from]);
        auto dof = node->dof_number(from_sys.number(), from_var_num, 0);

        local_values[i_from].push_back((*from_sys.solution)(dof));
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

        auto dof = elem->dof_number(from_sys.number(), from_var_num, 0);
        points[i_from].push_back(elem->vertex_average() + _from_positions[i_from]);
        local_values[i_from].push_back((*from_sys.solution)(dof));
      }
    }

    std::shared_ptr<KDTree> _kd_tree =
        std::make_shared<KDTree>(points[i_from], from_mesh.getMaxLeafSize());
    kdtrees[i_from] = _kd_tree;
  }
}

void
MultiAppGeneralFieldNearestNodeTransfer::evaluateInterpValues(
    const std::vector<Point> & incoming_points, std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateInterpValuesNearestNode(
      _local_kdtrees, _local_points, _local_values, incoming_points, outgoing_vals);
}

void
MultiAppGeneralFieldNearestNodeTransfer::evaluateInterpValuesNearestNode(
    const std::vector<std::shared_ptr<KDTree>> & local_kdtrees,
    const std::vector<std::vector<Point>> & local_points,
    const std::vector<std::vector<Real>> & local_values,
    const std::vector<Point> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (auto & pt : incoming_points)
  {
    // Reset distance
    outgoing_vals[i_pt].second = std::numeric_limits<Real>::max();
    bool value_found = false;

    // Loop on all source problems
    for (MooseIndex(_from_problems.size()) i_from = 0; i_from < _from_problems.size(); ++i_from)
    {
      std::vector<std::size_t> return_index(_num_nearest_points);
      std::vector<Real> return_dist_sqr(_num_nearest_points);

      // KD Tree can be empty if no points are within block/boundary/bounding box restrictions
      if (local_kdtrees[i_from]->numberCandidatePoints())
      {
        value_found = true;
        local_kdtrees[i_from]->neighborSearch(
            pt, _num_nearest_points, return_index, return_dist_sqr);
        Real val_sum = 0, dist_sum = 0;
        for (auto index : return_index)
        {
          val_sum += local_values[i_from][index];
          dist_sum += (local_points[i_from][index] - pt).norm();
        }
        // Pick the closest
        if (dist_sum / return_dist_sqr.size() < outgoing_vals[i_pt].second)
          outgoing_vals[i_pt] = {val_sum / return_index.size(), dist_sum / return_dist_sqr.size()};
      }
    }

    // none of the source problem meshes were within the restrictions set
    if (!value_found)
      outgoing_vals[i_pt] = {GeneralFieldTransfer::BetterOutOfMeshValue,
                             GeneralFieldTransfer::BetterOutOfMeshValue};
    else if (_search_value_conflicts)
    {
      // Local source meshes conflicts can happen two ways:
      // - two (or more) problems' nearest nodes are equidistant to the target point
      // - within a problem, the num_nearest_points'th closest node is as close to the target point
      //   as the num_nearest_points + 1'th closest node
      unsigned int num_equidistant_problems = 0;

      for (MooseIndex(_from_problems.size()) i_from = 0; i_from < _from_problems.size(); ++i_from)
      {
        unsigned int num_search = _num_nearest_points + 1;
        std::vector<std::size_t> return_index(num_search);
        std::vector<Real> return_dist_sqr(num_search);

        if (local_kdtrees[i_from]->numberCandidatePoints())
        {
          local_kdtrees[i_from]->neighborSearch(pt, num_search, return_index, return_dist_sqr);
          auto num_found = return_dist_sqr.size();

          // Look for too many equidistant nodes within a problem. First zip then sort by distance
          auto local_pt = pt - _from_positions[i_from];
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
            val_sum += local_values[i_from][index];
            dist_sum += (local_points[i_from][index] - pt).norm();
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
