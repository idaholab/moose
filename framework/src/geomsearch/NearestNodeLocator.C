//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "SecondaryNeighborhoodThread.h"
#include "NearestNodeThread.h"
#include "Moose.h"
#include "KDTree.h"
#include "Conversion.h"
#include "MooseApp.h"

// libMesh
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/plane.h"
#include "libmesh/mesh_tools.h"

NearestNodeLocator::NearestNodeLocator(SubProblem & subproblem,
                                       MooseMesh & mesh,
                                       BoundaryID boundary1,
                                       BoundaryID boundary2)
  : Restartable(subproblem.getMooseApp(),
                Moose::stringify(boundary1) + Moose::stringify(boundary2),
                "NearestNodeLocator",
                0),
    PerfGraphInterface(subproblem.getMooseApp().perfGraph(),
                       "NearestNodeLocator_" + Moose::stringify(boundary1) + "_" +
                           Moose::stringify(boundary2)),
    _subproblem(subproblem),
    _mesh(mesh),
    _boundary1(boundary1),
    _boundary2(boundary2),
    _first(true),
    _patch_update_strategy(_mesh.getPatchUpdateStrategy())
{
  /*
  //sanity check on boundary ids
  const std::set<BoundaryID>& bids=_mesh.getBoundaryIDs();
  std::set<BoundaryID>::const_iterator sit;
  sit=bids.find(_boundary1);
  if (sit == bids.end())
    mooseError("NearestNodeLocator being created for boundaries ", _boundary1, " and ", _boundary2,
  ", but boundary ", _boundary1, " does not exist");
  sit=bids.find(_boundary2);
  if (sit == bids.end())
    mooseError("NearestNodeLocator being created for boundaries ", _boundary1, " and ", _boundary2,
  ", but boundary ", _boundary2, " does not exist");
  */
}

NearestNodeLocator::~NearestNodeLocator() = default;

void
NearestNodeLocator::findNodes()
{
  TIME_SECTION("findNodes", 3, "Finding Nearest Nodes");

  /**
   * If this is the first time through we're going to build up a "neighborhood" of nodes
   * surrounding each of the secondary nodes.  This will speed searching later.
   */
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();

  if (_first)
  {
    _first = false;

    // Trial secondary nodes are all the nodes on the secondary side
    // We only keep the ones that are either on this processor or are likely
    // to interact with elements on this processor (ie nodes owned by this processor
    // are in the "neighborhood" of the secondary node
    std::vector<dof_id_type> trial_secondary_nodes;
    std::vector<dof_id_type> trial_primary_nodes;

    // Build a bounding box.  No reason to consider nodes outside of our inflated BB
    std::unique_ptr<BoundingBox> my_inflated_box = nullptr;

    const std::vector<Real> & inflation = _mesh.getGhostedBoundaryInflation();

    // This means there was a user specified inflation... so we can build a BB
    if (inflation.size() > 0)
    {
      BoundingBox my_box = MeshTools::create_local_bounding_box(_mesh);

      Point distance;
      for (unsigned int i = 0; i < inflation.size(); ++i)
        distance(i) = inflation[i];

      my_inflated_box =
          std::make_unique<BoundingBox>(my_box.first - distance, my_box.second + distance);
    }

    // Data structures to hold the boundary nodes
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      BoundaryID boundary_id = bnode->_bnd_id;
      dof_id_type node_id = bnode->_node->id();

      // If we have a BB only consider saving this node if it's in our inflated BB
      if (!my_inflated_box || (my_inflated_box->contains_point(*bnode->_node)))
      {
        if (boundary_id == _boundary1)
          trial_primary_nodes.push_back(node_id);
        else if (boundary_id == _boundary2)
          trial_secondary_nodes.push_back(node_id);
      }
    }

    // Convert trial primary nodes to a vector of Points. This will be used to
    // construct the Kdtree.
    std::vector<Point> primary_points(trial_primary_nodes.size());
    for (unsigned int i = 0; i < trial_primary_nodes.size(); ++i)
    {
      const Node & node = _mesh.nodeRef(trial_primary_nodes[i]);
      primary_points[i] = node;
    }

    // Create object kd_tree of class KDTree using the coordinates of trial
    // primary nodes.
    KDTree kd_tree(primary_points, _mesh.getMaxLeafSize());

    NodeIdRange trial_secondary_node_range(
        trial_secondary_nodes.begin(), trial_secondary_nodes.end(), 1);

    SecondaryNeighborhoodThread snt(
        _mesh, trial_primary_nodes, node_to_elem_map, _mesh.getPatchSize(), kd_tree);

    Threads::parallel_reduce(trial_secondary_node_range, snt);

    _secondary_nodes = snt._secondary_nodes;
    _neighbor_nodes = snt._neighbor_nodes;

    // If 'iteration' patch update strategy is used, a second neighborhood
    // search using the ghosting_patch_size, which is larger than the regular
    // patch_size used for contact search, is conducted. The ghosted element set
    // given by this search is used for ghosting the elements connected to the
    // secondary and neighboring primary nodes.
    if (_patch_update_strategy == Moose::Iteration)
    {
      SecondaryNeighborhoodThread snt_ghosting(
          _mesh, trial_primary_nodes, node_to_elem_map, _mesh.getGhostingPatchSize(), kd_tree);

      Threads::parallel_reduce(trial_secondary_node_range, snt_ghosting);

      for (const auto & dof : snt_ghosting._ghosted_elems)
        _subproblem.addGhostedElem(dof);
    }
    else
    {
      for (const auto & dof : snt._ghosted_elems)
        _subproblem.addGhostedElem(dof);
    }

    // Cache the secondary_node_range so we don't have to build it each time
    _secondary_node_range =
        std::make_unique<NodeIdRange>(_secondary_nodes.begin(), _secondary_nodes.end(), 1);
  }

  _nearest_node_info.clear();

  NearestNodeThread nnt(_mesh, _neighbor_nodes);

  Threads::parallel_reduce(*_secondary_node_range, nnt);

  _max_patch_percentage = nnt._max_patch_percentage;

  _nearest_node_info = nnt._nearest_node_info;

  if (_patch_update_strategy == Moose::Iteration)
  {
    // Get the set of elements that are currently being ghosted
    std::set<dof_id_type> ghost = _subproblem.ghostedElems();

    for (const auto & node_id : *_secondary_node_range)
    {
      const Node * nearest_node = _nearest_node_info[node_id]._nearest_node;

      // Check if the elements attached to the nearest node are within the ghosted
      // set of elements. If not produce an error.
      auto node_to_elem_pair = node_to_elem_map.find(nearest_node->id());

      if (node_to_elem_pair != node_to_elem_map.end())
      {
        const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;
        for (const auto & dof : elems_connected_to_node)
          if (std::find(ghost.begin(), ghost.end(), dof) == ghost.end() &&
              _mesh.elemPtr(dof)->processor_id() != _mesh.processor_id())
            mooseError("Error in NearestNodeLocator : The nearest neighbor lies outside the "
                       "ghosted set of elements. Increase the ghosting_patch_size parameter in the "
                       "mesh block and try again.");
      }
    }
  }
}

void
NearestNodeLocator::reinit()
{
  TIME_SECTION("reinit", 3, "Reinitializing Nearest Node Search");

  // Reset all data
  _secondary_node_range.reset();
  _nearest_node_info.clear();

  _first = true;

  _secondary_nodes.clear();
  _neighbor_nodes.clear();

  _new_ghosted_elems.clear();

  // Redo the search
  findNodes();
}

Real
NearestNodeLocator::distance(dof_id_type node_id)
{
  return _nearest_node_info[node_id]._distance;
}

const Node *
NearestNodeLocator::nearestNode(dof_id_type node_id)
{
  return _nearest_node_info[node_id]._nearest_node;
}

void
NearestNodeLocator::updatePatch(std::vector<dof_id_type> & secondary_nodes)
{
  TIME_SECTION("updatePatch", 3, "Updating Nearest Node Search Patch");

  std::vector<dof_id_type> trial_primary_nodes;

  // Build a bounding box.  No reason to consider nodes outside of our inflated BB
  std::unique_ptr<BoundingBox> my_inflated_box = nullptr;

  const std::vector<Real> & inflation = _mesh.getGhostedBoundaryInflation();

  // This means there was a user specified inflation... so we can build a BB
  if (inflation.size() > 0)
  {
    BoundingBox my_box = MeshTools::create_local_bounding_box(_mesh);

    Point distance;
    for (unsigned int i = 0; i < inflation.size(); ++i)
      distance(i) = inflation[i];

    my_inflated_box =
        std::make_unique<BoundingBox>(my_box.first - distance, my_box.second + distance);
  }

  // Data structures to hold the boundary nodes
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (const auto & bnode : bnd_nodes)
  {
    BoundaryID boundary_id = bnode->_bnd_id;
    dof_id_type node_id = bnode->_node->id();

    // If we have a BB only consider saving this node if it's in our inflated BB
    if (!my_inflated_box || (my_inflated_box->contains_point(*bnode->_node)))
    {
      if (boundary_id == _boundary1)
        trial_primary_nodes.push_back(node_id);
    }
  }

  // Convert trial primary nodes to a vector of Points. This will be used to construct the KDTree.
  std::vector<Point> primary_points(trial_primary_nodes.size());
  for (unsigned int i = 0; i < trial_primary_nodes.size(); ++i)
  {
    const Node & node = _mesh.nodeRef(trial_primary_nodes[i]);
    primary_points[i] = node;
  }

  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();

  // Create object kd_tree of class KDTree using the coordinates of trial
  // primary nodes.
  KDTree kd_tree(primary_points, _mesh.getMaxLeafSize());

  NodeIdRange secondary_node_range(secondary_nodes.begin(), secondary_nodes.end(), 1);

  SecondaryNeighborhoodThread snt(
      _mesh, trial_primary_nodes, node_to_elem_map, _mesh.getPatchSize(), kd_tree);

  Threads::parallel_reduce(secondary_node_range, snt);

  // Calculate new ghosting patch for the secondary_node_range
  SecondaryNeighborhoodThread snt_ghosting(
      _mesh, trial_primary_nodes, node_to_elem_map, _mesh.getGhostingPatchSize(), kd_tree);

  Threads::parallel_reduce(secondary_node_range, snt_ghosting);

  // Add the new set of elements that need to be ghosted into _new_ghosted_elems
  for (const auto & dof : snt_ghosting._ghosted_elems)
    _new_ghosted_elems.push_back(dof);

  std::vector<dof_id_type> tracked_secondary_nodes = snt._secondary_nodes;

  // Update the neighbor nodes (patch) for these tracked secondary nodes
  for (const auto & node_id : tracked_secondary_nodes)
    _neighbor_nodes[node_id] = snt._neighbor_nodes[node_id];

  NodeIdRange tracked_secondary_node_range(
      tracked_secondary_nodes.begin(), tracked_secondary_nodes.end(), 1);

  NearestNodeThread nnt(_mesh, snt._neighbor_nodes);

  Threads::parallel_reduce(tracked_secondary_node_range, nnt);

  _max_patch_percentage = nnt._max_patch_percentage;

  // Get the set of elements that are currently being ghosted
  std::set<dof_id_type> ghost = _subproblem.ghostedElems();

  // Update the nearest node information corresponding to these tracked secondary nodes
  for (const auto & node_id : tracked_secondary_node_range)
  {
    _nearest_node_info[node_id] = nnt._nearest_node_info[node_id];

    // Check if the elements attached to the nearest node are within the ghosted
    // set of elements. If not produce an error.
    const Node * nearest_node = nnt._nearest_node_info[node_id]._nearest_node;

    auto node_to_elem_pair = node_to_elem_map.find(nearest_node->id());

    if (node_to_elem_pair != node_to_elem_map.end())
    {
      const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;
      for (const auto & dof : elems_connected_to_node)
        if (std::find(ghost.begin(), ghost.end(), dof) == ghost.end() &&
            _mesh.elemPtr(dof)->processor_id() != _mesh.processor_id())
          mooseError("Error in NearestNodeLocator : The nearest neighbor lies outside the ghosted "
                     "set of elements. Increase the ghosting_patch_size parameter in the mesh "
                     "block and try again.");
    }
  }
}

void
NearestNodeLocator::updateGhostedElems()
{
  TIME_SECTION("updateGhostedElems", 5, "Updating Nearest Node Search Because of Ghosting");

  // When 'iteration' patch update strategy is used, add the elements in
  // _new_ghosted_elems, which were accumulated in the nonlinear iterations
  // during the previous time step, to the list of ghosted elements. Also clear
  // the _new_ghosted_elems array for storing the ghosted elements from the
  // nonlinear iterations in the current time step.

  for (const auto & dof : _new_ghosted_elems)
    _subproblem.addGhostedElem(dof);

  _new_ghosted_elems.clear();
}
//===================================================================
NearestNodeLocator::NearestNodeInfo::NearestNodeInfo()
  : _nearest_node(nullptr), _distance(std::numeric_limits<Real>::max())
{
}
