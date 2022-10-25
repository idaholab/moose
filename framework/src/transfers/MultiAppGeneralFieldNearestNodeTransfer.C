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
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"

#include "libmesh/generic_projector.h"
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

registerMooseObject("MooseApp", MultiAppGeneralFieldNearestNodeTransfer);

InputParameters
MultiAppGeneralFieldNearestNodeTransfer::validParams()
{
  InputParameters params = MultiAppGeneralFieldTransfer::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position by finding the value at the nearest neighbor "
      "in the origin application.");
  params.addParam<unsigned int>("num_nearest_points",
                                1,
                                "Number of nearest source (from) points will be chosen to "
                                "construct a value for the target point.");
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

    std::set<SubdomainID> from_blocks;
    // Take users' input block names
    // Change them to ids
    // Store then in a member variables
    if (isParamValid("from_blocks"))
    {
      // User input block names
      auto & blocks = getParam<std::vector<SubdomainName>>("from_blocks");
      // Subdomain ids
      std::vector<SubdomainID> ids = from_mesh.getSubdomainIDs(blocks);
      // We might have more than one problems
      from_blocks.clear();
      // Store these ids
      from_blocks.insert(ids.begin(), ids.end());
    }

    std::set<BoundaryID> from_boundaries;
    if (isParamValid("from_boundary"))
    {
      // User input block names
      auto & boundary_names = getParam<std::vector<BoundaryName>>("from_boundary");
      std::vector<BoundaryID> boundary_ids = from_mesh.getBoundaryIDs(boundary_names);
      // Store these ids
      from_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
    }

    if (is_nodal)
    {
      for (const auto & node : from_mesh.getMesh().local_node_ptr_range())
      {
        if (node->n_dofs(from_sys.number(), from_var_num) < 1)
          continue;

        if (!from_blocks.empty() && !hasBlocks(from_blocks, from_mesh, node))
          continue;

        if (!from_boundaries.empty() && !hasBoundaries(from_boundaries, from_mesh, node))
          continue;

        points[i_from].push_back(*node + _from_positions[i_from]);
        auto dof = node->dof_number(from_sys.number(), from_var_num, 0);

        local_values[i_from].push_back((*from_sys.solution)(dof));
      }
    }
    else
    {
      if (!from_boundaries.empty())
      {
        mooseError("You can not restrict an elemental variable to boundary");
      }
      for (auto & elem : as_range(from_mesh.getMesh().local_elements_begin(),
                                  from_mesh.getMesh().local_elements_end()))
      {
        if (elem->n_dofs(from_sys.number(), from_var_num) < 1)
          continue;

        if (!from_blocks.empty() && !hasBlocks(from_blocks, elem))
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
    // Loop until we've found the lowest-ranked app that actually contains
    // the quadrature point.
    for (MooseIndex(_from_problems.size()) i_from = 0;
         i_from < _from_problems.size() &&
         outgoing_vals[i_pt].first == GeneralFieldTransfer::BetterOutOfMeshValue;
         ++i_from)
    {

      std::vector<std::size_t> return_index(_num_nearest_points);
      std::vector<Real> return_dist_sqr(_num_nearest_points);
      if (local_kdtrees[i_from]->numberCandidatePoints())
      {
        local_kdtrees[i_from]->neighborSearch(
            pt, _num_nearest_points, return_index, return_dist_sqr);
        Real val_sum = 0, dist_sum = 0;
        for (auto index : return_index)
        {
          val_sum += local_values[i_from][index];
          dist_sum += (local_points[i_from][index] - pt).norm();
        }
        // Use mesh function to compute interpolation values
        // Assign value
        outgoing_vals[i_pt] = {val_sum / return_index.size(), dist_sum / return_dist_sqr.size()};
      }
      else
      {
        outgoing_vals[i_pt] = {GeneralFieldTransfer::BetterOutOfMeshValue,
                               GeneralFieldTransfer::BetterOutOfMeshValue};
      }
    }

    // Move to next point
    i_pt++;
  }
}
