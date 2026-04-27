//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldKDTreeTransferBase.h"

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

InputParameters
MultiAppGeneralFieldKDTreeTransferBase::validParams()
{
  InputParameters params = MultiAppGeneralFieldTransfer::validParams();

  params.addParam<unsigned int>("num_nearest_points",
                                1,
                                "Number of nearest source (from) points will be chosen to "
                                "construct a value for the target point. All points will be "
                                "selected from the same origin mesh!");

  // choose whether to include data from multiple apps when performing nearest-position/
  // mesh-divisions based transfers
  params.addParam<bool>("group_subapps",
                        false,
                        "Whether to group source locations and values from all subapps "
                        "when working with a nearest-position or source mesh-division");

  return params;
}

MultiAppGeneralFieldKDTreeTransferBase::MultiAppGeneralFieldKDTreeTransferBase(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters),
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
MultiAppGeneralFieldKDTreeTransferBase::initialSetup()
{
  MultiAppGeneralFieldTransfer::initialSetup();

  // We need to improve the indexing if we are to allow this
  if (!_from_mesh_divisions.empty())
    for (const auto mesh_div : _from_mesh_divisions)
      if (mesh_div->getNumDivisions() != _from_mesh_divisions[0]->getNumDivisions())
        paramError("from_mesh_division",
                   "This transfer has only been implemented with a uniform number of source mesh "
                   "divisions across all source applications");
}

void
MultiAppGeneralFieldKDTreeTransferBase::prepareEvaluationOfInterpValues(
    const unsigned int var_index)
{
  _local_kdtrees.clear();
  _local_points.clear();
  _local_values.clear();
  buildKDTrees(var_index);
}

bool
MultiAppGeneralFieldKDTreeTransferBase::inBlocks(const std::set<SubdomainID> & blocks,
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
MultiAppGeneralFieldKDTreeTransferBase::computeNumSources()
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
MultiAppGeneralFieldKDTreeTransferBase::getAppIndex(unsigned int kdtree_index,
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
MultiAppGeneralFieldKDTreeTransferBase::getNumAppsPerTree() const
{
  if (_use_nearest_app)
    return 1;
  else if (_group_subapps)
    return _from_meshes.size();
  else
    return 1;
}

unsigned int
MultiAppGeneralFieldKDTreeTransferBase::getNumDivisions() const
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
MultiAppGeneralFieldKDTreeTransferBase::getPointInSourceKDTreeFrame(unsigned int i_source,
                                                                    const Point & pt) const
{
  if (_nearest_positions_obj && _group_subapps)
    // KD-trees are built in global coords when grouping subapps with nearest-positions
    return pt;
  else
    return getPointInSourceAppFrame(pt, i_source, "KD-tree neighbor search");
}

bool
MultiAppGeneralFieldKDTreeTransferBase::checkRestrictionsForSource(const Point & pt,
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
    if (_nearest_positions_obj)
      mooseError("Nearest-positions + source_app_must_contain_point not implemented");
    // Transform the point to place it in the local coordinate system
    const auto local_pt = getPointInSourceAppFrame(pt, app_index, "Source mesh containment check");
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

void
MultiAppGeneralFieldKDTreeTransferBase::evaluateNearestNodeFromKDTrees(
    const Point & pt,
    unsigned int source_index,
    std::pair<Real, Real> & outgoing_val,
    bool & point_found)
{
  // First pass: find the nearest neighbor value across all local sources
  for (const auto i_from : make_range(_num_sources))
  {
    if (!checkRestrictionsForSource(pt, source_index, i_from))
      continue;

    // TODO: Pre-allocate these two work arrays. They will be regularly resized by the searches
    std::vector<std::size_t> return_index(_num_nearest_points);
    std::vector<Real> return_dist_sqr(_num_nearest_points);

    // KD-Tree can be empty if no points are within block/boundary/bounding box restrictions
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
      if (new_distance < outgoing_val.second)
        outgoing_val = {val_sum / return_index.size(), new_distance};
    }
  }

  // Second pass: search-value-conflict detection
  if (point_found && _search_value_conflicts)
  {
    unsigned int num_equidistant_problems = 0;

    for (const auto i_from : make_range(_num_sources))
    {
      if (!checkRestrictionsForSource(pt, source_index, i_from))
        continue;

      // TODO: Pre-allocate these two work arrays. They will be regularly resized by the searches
      std::vector<std::size_t> return_index(_num_nearest_points + 1);
      std::vector<Real> return_dist_sqr(_num_nearest_points + 1);

      const auto app_index = getAppIndex(i_from, i_from / getNumDivisions());
      const auto num_search = _num_nearest_points + 1;

      if (_local_kdtrees[i_from]->numberCandidatePoints())
      {
        _local_kdtrees[i_from]->neighborSearch(pt, num_search, return_index, return_dist_sqr);
        auto num_found = return_dist_sqr.size();

        // Local coordinates only accessible when not using nearest-position,
        // as we did not keep the index of the source app, only the position index
        const Point local_pt = getPointInSourceKDTreeFrame(app_index, pt);

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
            registerConflict(app_index, 0, pt, outgoing_val.second, true);
          else
            registerConflict(app_index, 0, local_pt, outgoing_val.second, true);
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
        if (MooseUtils::absoluteFuzzyEqual(dist_sum / return_dist_sqr.size(), outgoing_val.second))
        {
          num_equidistant_problems++;
          if (num_equidistant_problems > 1)
          {
            if (_nearest_positions_obj)
              registerConflict(app_index, 0, pt, outgoing_val.second, true);
            else
              registerConflict(app_index, 0, local_pt, outgoing_val.second, true);
          }
        }
      }
    }
  }
}
