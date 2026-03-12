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
  InputParameters params = MultiAppGeneralFieldTransfer::validParams();
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

  MooseEnum extrapolation("flat nearest-node linear", "linear");
  params.addParam<MooseEnum>("extrapolation_behavior",
                             extrapolation,
                             "How to extrapolate the functors when a target point for the transfer "
                             "is outside the domain of evaluation");

  // TODO: decide if this option really makes sense
  // choose whether to include data from multiple apps when performing nearest-position/
  // mesh-divisions based transfers
  params.addParam<bool>("group_subapps",
                        false,
                        "Whether to group source locations and values from all subapps "
                        "when considering algorithm");
  // This is a convenient heuristic to limit communication
  params.renameParam("use_nearest_app", "assume_nearest_app_holds_nearest_location", "");

  return params;
}

MultiAppGeneralFieldFunctorTransfer::MultiAppGeneralFieldFunctorTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters),
    NonADFunctorInterface(this),
    _functor_names(getParam<std::vector<MooseFunctorName>>("source_functors")),
    _extrapolation_behavior(getParam<MooseEnum>("extrapolation_behavior")),
    _group_subapps(getParam<bool>("group_subapps"))
{
  // Same as for NL transfer
  if (_source_app_must_contain_point && _nearest_positions_obj)
    paramError("use_nearest_position",
               "We do not support using both nearest positions matching and checking if target "
               "points are within an app domain because the KDTrees for nearest-positions matching "
               "are (currently) built with data from multiple applications.");
  if (_nearest_positions_obj &&
      (isParamValid("from_mesh_divisions") || isParamValid("to_mesh_divisions")))
    paramError("use_nearest_position", "Cannot use nearest positions with mesh divisions");

  // Check extrapolation options
  if (isParamSetByUser("extrapolation_constant") && _extrapolation_behavior != "flat")
    paramError("extrapolation_behavior",
               "Flat (single-constant) extrapolation must be selected if an extrapolation constant "
               "is specified");

  // Check size
  if (_functor_names.size() != _to_var_names.size())
    paramError("source_functors", "Should be the same size as target 'variable'");

  // Parameter checks on grouping subapp values
  // if (_group_subapps && _from_mesh_divisions.empty() && !_nearest_positions_obj)
  //   paramError(
  //       "group_subapps",
  //       "This option is only available for using mesh divisions or nearest positions regions");
  // else if (_group_subapps &&
  //          (_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
  //           _from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX))
  //   paramError("group_subapps",
  //              "Cannot group subapps when considering nearest-location data as we would lose "
  //              "track of the division index of the source locations");
  // else if (_group_subapps && _use_nearest_app)
  //   paramError(
  //       "group_subapps",
  //       "When using the 'nearest child application' data, the source data (positions and values)
  //       " "are grouped on a per-application basis, so it cannot be agglomerated over all child "
  //       "applications.\nNote that the option to use nearest applications for source restrictions,
  //       " "but further split each child application's domain by regions closest to each position
  //       "
  //       "(here the the child application's centroid), which could be conceived when "
  //       "'group_subapps' = false, is also not available.");
}

void
MultiAppGeneralFieldFunctorTransfer::initialSetup()
{
  MultiAppGeneralFieldTransfer::initialSetup();

  // Retrieve the functors
  for (const auto & fname : _functor_names)
    _functors.push_back(&getFunctor<Real>(fname));

  // Same as NL transfer
  // We need to improve the indexing if we are to allow this
  if (!_from_mesh_divisions.empty())
    for (const auto mesh_div : _from_mesh_divisions)
      if (mesh_div->getNumDivisions() != _from_mesh_divisions[0]->getNumDivisions())
        paramError("from_mesh_division",
                   "This transfer has only been implemented with a uniform number of source mesh "
                   "divisions across all source applications");
}

void
MultiAppGeneralFieldFunctorTransfer::prepareEvaluationOfInterpValues(const unsigned int var_index)
{
  _local_kdtrees.clear();
  _local_points.clear();
  _local_values.clear();
  buildKDTrees(var_index);

  // Get the point locators
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

  const auto & functor = _functors[var_index];
  const auto from_blocks =
      _from_blocks.size() ? _from_blocks : Moose::NodeArg::undefined_subdomain_connection;

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

      // We need to loop on the nodes on the node at the edge of the domain of definition the current functor
      for (const auto & node : from_mesh.getMesh().local_node_ptr_range())
      {
        // No way to check number of dofs for a functor
        bool on_at_least_one_block = false;
        bool not_on_a_block = false;
        for (const auto eid : libmesh_map_find(node_to_elem_map, node->id()))
        {
          bool has_block = functor->hasBlocks(from_mesh.elemPtr(eid)->subdomain_id());
          if (has_block)
            on_at_least_one_block = true;
          else
            not_on_a_block = true;
        }

        // Not on a boundary
        if (!on_at_least_one_block || !not_on_a_block)
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
          else if ((_from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
                    _to_mesh_division_behavior == MeshDivisionTransferUse::MATCH_DIVISION_INDEX ||
                    _from_mesh_division_behavior == MeshDivisionTransferUse::MATCH_SUBAPP_INDEX) &&
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

  const auto & functor = *_functors[var_index];
  const auto from_blocks =
      _from_blocks.size() ? _from_blocks : Moose::NodeArg::undefined_subdomain_connection;

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

      // If in domain, use the functor evaluation at the point
      // Locate the point and an element
      (*_point_locators[i_from])(pt, elem_candidates, &from_blocks);
      if (elem_candidates.size())
      {
        // Average the result for now
        // TODO: if we knew the functor was continuous, could return earlier
        Real value = 0;
        for (const auto elem : elem_candidates)
        {
          Moose::ElemPointArg elem_pt_arg = {elem, pt, /*correct skewness*/ false};
          Moose::StateArg time_arg(0, Moose::SolutionIterationType::Time);
          value += functor(elem_pt_arg, time_arg);
        }
        value /= elem_candidates.size();

        outgoing_vals[i_pt] = {value, 0};
        // Skip nearest node KD-Tree evaluation
        continue;
      }

      // TODO: Pre-allocate these two work arrays. They will be regularly resized by the
      // searches
      const auto _num_nearest_points = 1;
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
      // There are no search value conflicts within the domain.
      // Outside the domain, it depends on the extrapolation option
      // Flat: no conflict
      // Nearest-node: same issue as nearest location
      // Linear: TBD
    }

    // Move to next point
    i_pt++;
  }
}

bool
MultiAppGeneralFieldFunctorTransfer::inBlocks(const std::set<SubdomainID> & blocks,
                                              const MooseMesh & mesh,
                                              const Elem * elem) const
{
  // We need to override the definition of block restriction for an element
  // because we have to consider whether each node of an element is adjacent to a block
  for (const auto & i_node : make_range(elem->n_nodes()))
  {
    const auto & node = elem->node_ptr(i_node);
    const auto & node_blocks = mesh.getNodeBlockIds(*node);
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
MultiAppGeneralFieldFunctorTransfer::computeNumSources()
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
MultiAppGeneralFieldFunctorTransfer::getAppIndex(unsigned int kdtree_index,
                                                 unsigned int nested_loop_on_app_index) const
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
MultiAppGeneralFieldFunctorTransfer::getNumAppsPerTree() const
{
  if (_use_nearest_app)
    return 1;
  else if (_group_subapps)
    return _from_meshes.size();
  else
    return 1;
}

unsigned int
MultiAppGeneralFieldFunctorTransfer::getNumDivisions() const
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
MultiAppGeneralFieldFunctorTransfer::getPointInLocalSourceFrame(unsigned int i_from,
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
MultiAppGeneralFieldFunctorTransfer::checkRestrictionsForSource(const Point & pt,
                                                                const unsigned int mesh_div,
                                                                const unsigned int i_from) const
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
