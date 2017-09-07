/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "SlaveNeighborhoodThread.h"
#include "NearestNodeThread.h"
#include "Moose.h"
#include "KDTree.h"

// libMesh
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/plane.h"
#include "libmesh/mesh_tools.h"

std::string
_boundaryFuser(BoundaryID boundary1, BoundaryID boundary2)
{
  std::stringstream ss;

  ss << boundary1 << "to" << boundary2;

  return ss.str();
}

NearestNodeLocator::NearestNodeLocator(SubProblem & subproblem,
                                       MooseMesh & mesh,
                                       BoundaryID boundary1,
                                       BoundaryID boundary2)
  : Restartable(_boundaryFuser(boundary1, boundary2), "NearestNodeLocator", subproblem, 0),
    _subproblem(subproblem),
    _mesh(mesh),
    _slave_node_range(NULL),
    _boundary1(boundary1),
    _boundary2(boundary2),
    _first(true),
    _ghost_elements(true),
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

NearestNodeLocator::~NearestNodeLocator() { delete _slave_node_range; }

void
NearestNodeLocator::findNodes()
{
  Moose::perf_log.push("NearestNodeLocator::findNodes()", "Execution");
  /**
   * If this is the first time through we're going to build up a "neighborhood" of nodes
   * surrounding each of the slave nodes.  This will speed searching later.
   */

  if (_first)
  {
    _first = false;

    // Trial slave nodes are all the nodes on the slave side
    // We only keep the ones that are either on this processor or are likely
    // to interact with elements on this processor (ie nodes owned by this processor
    // are in the "neighborhood" of the slave node
    std::vector<dof_id_type> trial_slave_nodes;
    std::vector<dof_id_type> trial_master_nodes;

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
          libmesh_make_unique<BoundingBox>(my_box.first - distance, my_box.second + distance);
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
          trial_master_nodes.push_back(node_id);
        else if (boundary_id == _boundary2)
          trial_slave_nodes.push_back(node_id);
      }
    }

    const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map =
        _mesh.nodeToElemMap();

    // Ghost the entire master and slave boundary once when nonliner_iter patch update strategy is
    // used
    if (_ghost_elements && _patch_update_strategy == 3)
    {
      // The ghosting needs to be done only once during the simulation as the master
      // and slave boundaries do not change during the simulation.
      _ghost_elements = false;

      // Ghost the elements connected to master boundary and slave boundary_id
      for (unsigned int i = 0; i < trial_master_nodes.size(); ++i)
      {
        auto node_to_elem_pair = node_to_elem_map.find(trial_master_nodes[i]);

        if (node_to_elem_pair != node_to_elem_map.end())
        {
          const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;
          for (const auto & dof : elems_connected_to_node)
            _subproblem.addGhostedElem(dof);
        }
      }

      for (unsigned int i = 0; i < trial_slave_nodes.size(); ++i)
      {
        auto node_to_elem_pair = node_to_elem_map.find(trial_slave_nodes[i]);

        if (node_to_elem_pair != node_to_elem_map.end())
        {
          const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;
          for (const auto & dof : elems_connected_to_node)
            _subproblem.addGhostedElem(dof);
        }
      }
    }

    // Convert trial master nodes to a vector of Points. This will be used to
    // construct the Kdtree.
    std::vector<Point> master_points(trial_master_nodes.size());
    for (unsigned int i = 0; i < trial_master_nodes.size(); ++i)
    {
      const Node & node = _mesh.nodeRef(trial_master_nodes[i]);
      master_points[i] = node;
    }

    // Create object kd_tree of class KDTree using the coordinates of trial
    // master nodes.
    KDTree kd_tree(master_points, _mesh.getMaxLeafSize());

    NodeIdRange trial_slave_node_range(trial_slave_nodes.begin(), trial_slave_nodes.end(), 1);

    SlaveNeighborhoodThread snt(_mesh,
                                trial_master_nodes,
                                node_to_elem_map,
                                _mesh.getPatchSize(),
                                _patch_update_strategy,
                                kd_tree);

    Threads::parallel_reduce(trial_slave_node_range, snt);

    if (_patch_update_strategy == 3)
    {
      _slave_nodes = trial_slave_nodes;
      _neighbor_nodes = snt._neighbor_nodes;
    }
    else
    {
      _slave_nodes = snt._slave_nodes;
      _neighbor_nodes = snt._neighbor_nodes;

      for (const auto & dof : snt._ghosted_elems)
        _subproblem.addGhostedElem(dof);
    }

    // Cache the slave_node_range so we don't have to build it each time
    _slave_node_range = new NodeIdRange(_slave_nodes.begin(), _slave_nodes.end(), 1);
  }

  _nearest_node_info.clear();

  NearestNodeThread nnt(_mesh, _neighbor_nodes);

  Threads::parallel_reduce(*_slave_node_range, nnt);

  _max_patch_percentage = nnt._max_patch_percentage;

  _nearest_node_info = nnt._nearest_node_info;

  Moose::perf_log.pop("NearestNodeLocator::findNodes()", "Execution");
}

void
NearestNodeLocator::reinit()
{
  // Reset all data
  delete _slave_node_range;
  _slave_node_range = NULL;
  _nearest_node_info.clear();

  _first = true;

  _slave_nodes.clear();
  _neighbor_nodes.clear();

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
NearestNodeLocator::updatePatch(std::vector<dof_id_type> & slave_nodes)
{
  Moose::perf_log.push("NearestNodeLocator::updatePatch()", "Execution");

  std::vector<dof_id_type> trial_master_nodes;

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
        libmesh_make_unique<BoundingBox>(my_box.first - distance, my_box.second + distance);
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
        trial_master_nodes.push_back(node_id);
    }
  }

  // Convert trial master nodes to a vector of Points. This will be used to construct the KDTree.
  std::vector<Point> master_points(trial_master_nodes.size());
  for (unsigned int i = 0; i < trial_master_nodes.size(); ++i)
  {
    const Node & node = _mesh.nodeRef(trial_master_nodes[i]);
    master_points[i] = node;
  }

  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();

  // Create object kd_tree of class KDTree using the coordinates of trial
  // master nodes.
  KDTree kd_tree(master_points, _mesh.getMaxLeafSize());

  NodeIdRange slave_node_range(slave_nodes.begin(), slave_nodes.end(), 1);

  SlaveNeighborhoodThread snt(_mesh,
                              trial_master_nodes,
                              node_to_elem_map,
                              _mesh.getPatchSize(),
                              _patch_update_strategy,
                              kd_tree);

  Threads::parallel_reduce(slave_node_range, snt);

  // Update the neighbor nodes (patch) for these slave nodes
  for (const auto & node_id : slave_node_range)
    _neighbor_nodes[node_id] = snt._neighbor_nodes[node_id];

  NearestNodeThread nnt(_mesh, _neighbor_nodes);

  Threads::parallel_reduce(slave_node_range, nnt);

  _max_patch_percentage = nnt._max_patch_percentage;

  _nearest_node_info = nnt._nearest_node_info;

  // Update the nearest node information corresponding to these slave nodes
  for (const auto & node_id : slave_node_range)
    _nearest_node_info[node_id] = nnt._nearest_node_info[node_id];

  Moose::perf_log.pop("NearestNodeLocator::updatePatch()", "Execution");
}
//===================================================================
NearestNodeLocator::NearestNodeInfo::NearestNodeInfo()
  : _nearest_node(NULL), _distance(std::numeric_limits<Real>::max())
{
}
