//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldFunctorTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "Positions.h"
#include "MooseAppCoordTransform.h"
#include "MooseFunctorArguments.h"

using namespace libMesh;

registerMooseObject("MooseApp", MultiAppGeneralFieldFunctorTransfer);

InputParameters
MultiAppGeneralFieldFunctorTransfer::validParams()
{
  InputParameters params = MultiAppGeneralFieldKDTreeTransferBase::validParams();
  params += NonADFunctorInterface::validParams();
  params.addClassDescription(
      "Transfers functor data at the MultiApp position by evaluating the functor inside its domain "
      "of definition and extrapolating with a user-selected behavior outside");

  // Input variables as functors instead
  params.suppressParameter<std::vector<VariableName>>("source_variable");
  // NOTE: could rename this instead once we support array or vector functor component transfer
  params.suppressParameter<std::vector<unsigned int>>("source_variable_components");

  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "source_functors", "Functors providing the values to transfer to the target variables");

  // Potential additional parameters:
  // - functor evaluation spatial argument type
  // - functor evaluation time argument type
  // - number of points to use when creating extrapolation 'patches'
  // - other 'nearest' locations: node, element, side or a general 'location'
  // - options for a radius based search and build patches instead of 'nearest'

  MooseEnum extrapolation("flat evaluate_oob nearest-node nearest-elem", "nearest-node");
  params.addParam<MooseEnum>("extrapolation_behavior",
                             extrapolation,
                             "How to extrapolate the functors when a target point for the transfer "
                             "is outside the domain of evaluation");

  // This is a convenient heuristic to limit communication
  params.renameParam("use_nearest_app", "assume_nearest_app_holds_evaluation_location", "");

  return params;
}

MultiAppGeneralFieldFunctorTransfer::MultiAppGeneralFieldFunctorTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldKDTreeTransferBase(parameters),
    NonADFunctorInterface(this),
    _functor_names(getParam<std::vector<MooseFunctorName>>("source_functors")),
    _extrapolation_behavior(getParam<MooseEnum>("extrapolation_behavior"))
{
  // Check extrapolation options
  if (isParamSetByUser("extrapolation_constant") && _extrapolation_behavior != "flat")
    paramError("extrapolation_behavior",
               "Flat (single-constant) extrapolation must be selected if an extrapolation constant "
               "is specified");
  if (_post_transfer_extrapolation != "none" && _extrapolation_behavior != "flat")
    paramError("extrapolation_behavior",
               "Flat (single-constant) extrapolation must be selected if an extrapolation post-"
               "treatment is specified");

  // Check size
  if (_functor_names.size() != _to_var_names.size())
    paramError("source_functors", "Should be the same size as target 'variable'");
}

void
MultiAppGeneralFieldFunctorTransfer::initialSetup()
{
  MultiAppGeneralFieldKDTreeTransferBase::initialSetup();

  // Retrieve the functors
  _functors.resize(_from_problems.size());
  _functor_is_variable.resize(_functor_names.size());
  for (const auto i_functor : index_range(_functor_names))
  {
    const auto & fname = _functor_names[i_functor];

    // Different functors for every source
    for (const auto i_from : index_range(_from_problems))
      _functors[i_from].push_back(
          &_from_problems[i_from]->getFunctor<Real>(fname, /*thread*/ 0, name(), false));

    // Need to keep track of variables because of ghosting needs
    // NOTE: we don't really expect the functor type to vary between problems
    for (const auto i_from : index_range(_from_problems))
      _functor_is_variable[i_functor] =
          _functor_is_variable[i_from] || _from_problems[i_from]->hasVariable(fname);
  }
}

void
MultiAppGeneralFieldFunctorTransfer::execute()
{
  // Execute the user object if it was specified to execute on TRANSFER
  for (const auto & fname : _functor_names)
    switch (_current_direction)
    {
      case TO_MULTIAPP:
      {
        if (!_fe_problem.hasUserObject(fname))
          continue;
        checkParentAppUserObjectExecuteOn(fname);
        _fe_problem.computeUserObjectByName(EXEC_TRANSFER, Moose::PRE_AUX, fname);
        _fe_problem.computeUserObjectByName(EXEC_TRANSFER, Moose::POST_AUX, fname);
        break;
      }
      case FROM_MULTIAPP:
        errorIfObjectExecutesOnTransferInSourceApp(fname);
    }

  // Perfom the actual transfer
  MultiAppGeneralFieldKDTreeTransferBase::execute();
}

void
MultiAppGeneralFieldFunctorTransfer::prepareEvaluationOfInterpValues(const unsigned int var_index)
{
  MultiAppGeneralFieldKDTreeTransferBase::prepareEvaluationOfInterpValues(var_index);

  // Get the point locators
  // Should be done after we know the number of sources
  _point_locators.resize(_num_sources);
  for (const auto i_from : make_range(_num_sources))
    _point_locators[i_from] =
        _from_problems[i_from]->mesh(_displaced_source_mesh).getPointLocator();
}

void
MultiAppGeneralFieldFunctorTransfer::buildKDTrees(const unsigned int var_index)
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
      // No need for displaced mesh for checking domain of definition
      const auto & node_to_elem_map = from_problem.mesh().nodeToActiveSemilocalElemMap();

      // Get functor for that app
      const auto & functor = _functors[i_from][var_index];

      // Form the block restriction for evaluation. We need to prevent evaluation outside the
      // domain of evaluation of the functor (could crash) or the transfer (disobeys user)
      // Note: the functor subdomains of evaluation are checked below as well, so we
      // should be fairly safe
      std::set<SubdomainID> from_blocks;
      if (_from_blocks.size())
      {
        for (const auto bl : _from_blocks)
          if (functor->hasBlocks(bl))
            from_blocks.insert(bl);
      }
      else
        from_blocks = Moose::NodeArg::undefined_subdomain_connection;

      // We need to loop on the nodes on the node at the edge of the domain of definition the
      // current functor
      if (_extrapolation_behavior == 2) // nearest-node
        for (const auto & node : from_mesh.getMesh().local_node_ptr_range())
        {
          // No way to check number of dofs for a functor
          // Functor should be defined on at least one block by the block to have a value
          // Node should be either on a functor or a mesh boundary to be relevant for extrapolation
          bool on_at_least_one_block = false;
          bool on_boundary = false;
          for (const auto eid : libmesh_map_find(node_to_elem_map, node->id()))
          {
            bool has_block = functor->hasBlocks(from_mesh.elemPtr(eid)->subdomain_id());
            if (has_block)
              on_at_least_one_block = true;
            else
              on_boundary = true;
            // Detect a mesh boundary
            const auto elem = from_mesh.elemPtr(eid);
            for (const auto side : elem->side_index_range())
              if (!elem->neighbor_ptr(side) &&
                  elem->is_node_on_side(elem->get_node_index(node), side))
                on_boundary = true;
          }

          // Not on a boundary
          if (!on_at_least_one_block || !on_boundary)
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

          // Evaluate the functor on the boundary node
          Moose::NodeArg node_arg = {node, &from_blocks};
          Moose::StateArg time_arg(0, Moose::SolutionIterationType::Time);
          _local_values[i_source].push_back((*functor)(node_arg, time_arg));
        }
      // Nearest-element option
      // We also use this for 'evaluate_oob', which will influence the distance found and used to
      // select the nearest value from out of bounds evaluations. It will not influence the result
      // of the evaluation
      else
        for (const auto & elem : from_mesh.getMesh().local_element_ptr_range())
        {
          // No way to check number of dofs for a functor
          if (!functor->hasBlocks(elem->subdomain_id()))
            continue;

          // Make sure sure it is at a boundary, for either the mesh or a functor
          bool at_a_boundary = false;
          for (const auto side : elem->side_index_range())
          {
            // Boundary of the mesh
            if (!elem->neighbor_ptr(side))
            {
              at_a_boundary = true;
              break;
            }
            // Boundary of the domain of definition of the functor
            else if (!functor->hasBlocks(elem->neighbor_ptr(side)->subdomain_id()))
            {
              at_a_boundary = true;
              break;
            }
          }
          // Non boundary elements are not relevant as we can just evaluate the functor
          if (!at_a_boundary)
            continue;

          if (!_from_blocks.empty() && !inBlocks(_from_blocks, from_mesh, elem))
            continue;

          if (!_from_boundaries.empty() && !onBoundaries(_from_boundaries, from_mesh, elem))
            continue;

          // Handle the various source mesh divisions behaviors
          // NOTE: This could be more efficient, as instead of rejecting points in the
          // wrong division, we could just be adding them to the tree for the right division
          if (!_from_mesh_divisions.empty())
          {
            const auto tree_division_index = i_source % getNumDivisions();
            const auto node_div_index =
                _from_mesh_divisions[i_from]->divisionIndex(elem->vertex_average());

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

          // Transformed centroid is in the reference space, as is the _nearest_positions_obj
          const auto transformed_centroid =
              (*_from_transforms[getGlobalSourceAppIndex(i_from)])(elem->vertex_average());

          // Only add to the KDTree nodes that are closest to the 'position'
          // When querying values at a target point, the KDTree associated to the closest
          // position to the target point is queried
          // We do not need to check the positions when using nearest app as we will assume
          // (somewhat incorrectly) that all the points in each subapp are closer to that subapp
          // than to any other
          if (!_use_nearest_app && _nearest_positions_obj &&
              !closestToPosition(i_pos, transformed_centroid))
            continue;

          _local_points[i_source].push_back(transformed_centroid);

          // Evaluate the functor on the boundary node
          Moose::ElemArg elem_arg = {elem, /*sknewness*/ false};
          Moose::StateArg time_arg(0, Moose::SolutionIterationType::Time);
          _local_values[i_source].push_back((*functor)(elem_arg, time_arg));
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
MultiAppGeneralFieldFunctorTransfer::evaluateInterpValues(
    const unsigned int var_index,
    const std::vector<std::pair<Point, unsigned int>> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateValues(var_index, incoming_points, outgoing_vals);
}

void
MultiAppGeneralFieldFunctorTransfer::evaluateValues(
    const unsigned int var_index,
    const std::vector<std::pair<Point, unsigned int>> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  std::set<const Elem *> elem_candidates;

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
      // Note: because this transfer is intended for extrapolation,
      // this will usually not restrict the source. The distance comparisons will be crucial

      // Retrieve the functor
      const auto & functor = *_functors[i_from][var_index];

      // Get the intersection of the functor and transfer source block restrictions
      std::set<SubdomainID> from_blocks;
      for (const auto bl : _from_blocks.size()
                               ? _from_blocks
                               : _from_problems[i_from]->mesh().getMesh().get_mesh_subdomains())
        if (functor.hasBlocks(bl))
          from_blocks.insert(bl);

      // If in domain, use the functor evaluation at the point
      // Locate the point and an element
      // Clear the nearest candidates
      elem_candidates.clear();
      const auto transformed_pt = getPointInLocalSourceFrame(i_from, pt);
      if (from_blocks.size())
        (*_point_locators[i_from])(transformed_pt, elem_candidates, &from_blocks);
      else
        (*_point_locators[i_from])(transformed_pt, elem_candidates);
      if (elem_candidates.size())
      {
        // Register conflict if any
        if (point_found && _search_value_conflicts)
        {
          // In the nearest-position/app mode, we save conflicts in the reference frame
          if (_nearest_positions_obj)
            registerConflict(i_from, /*dof*/ 0, pt, 0, true);
          else
            registerConflict(i_from, 0, transformed_pt, 0, true);
        }

        // Average the result for now
        // TODO: if we knew the functor were continuous, we could return earlier
        Real value = 0;
        unsigned int num_values = 0;
        for (const auto elem : elem_candidates)
        {
          // Variables would hit a ghosting error
          if (_functor_is_variable[var_index] && elem->processor_id() != processor_id())
            continue;
          Moose::ElemPointArg elem_pt_arg = {elem, transformed_pt, /*correct skewness*/ false};
          Moose::StateArg time_arg(0, Moose::SolutionIterationType::Time);
          value += functor(elem_pt_arg, time_arg);
          num_values++;
        }
        if (num_values == 0)
          continue;

        value /= num_values;
        point_found = true;
        outgoing_vals[i_pt] = {value, 0};
        // Skip nearest node KD-Tree evaluation
        continue;
      }

      // Check if we have already found a point to avoid over-writing a valid value
      // No need to check the KD-Tree if the extrapolation behavior is flat
      if (!point_found && _extrapolation_behavior == 0) /*flat*/
      {
        // The base class will do take care of replacing the value
        outgoing_vals[i_pt] = {GeneralFieldTransfer::BetterOutOfMeshValue,
                               GeneralFieldTransfer::BetterOutOfMeshValue};
        continue;
      }

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
        {
          if (_extrapolation_behavior == 1) /*evaluate oob*/
          {
            Moose::ElemPointArg elem_pt_arg = {nullptr, transformed_pt, /*correct skewness*/ false};
            Moose::StateArg time_arg(0, Moose::SolutionIterationType::Time);
            outgoing_vals[i_pt] = {functor(elem_pt_arg, time_arg), new_distance};
          }
          // /* nearest-node and nearest-elem */
          else
            outgoing_vals[i_pt] = {val_sum / return_index.size(), new_distance};
        }
      }
    }

    // none of the source problem meshes were within the restrictions set
    if (!point_found)
      outgoing_vals[i_pt] = {GeneralFieldTransfer::BetterOutOfMeshValue,
                             GeneralFieldTransfer::BetterOutOfMeshValue};
    else if (_search_value_conflicts)
    {
      // Within the domain: implemented above
      // Outside the domain, it depends on the extrapolation option
      // Flat: no conflict
      // Evaluate out-of-bounds: same issue as nearest location
      // Nearest-node: same issue as nearest location
      // Linear: TBD
    }

    // Move to next point
    i_pt++;
  }
}
