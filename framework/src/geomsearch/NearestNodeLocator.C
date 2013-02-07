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
// libMesh
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/plane.h"
#include "libmesh/mesh_tools.h"


NearestNodeLocator::NearestNodeLocator(SubProblem & subproblem, MooseMesh & mesh, BoundaryID boundary1, BoundaryID boundary2) :
    _subproblem(subproblem),
    _mesh(mesh),
    _slave_node_range(NULL),
    _boundary1(boundary1),
    _boundary2(boundary2),
    _first(true)
{
  /*
  //sanity check on boundary ids
  const std::set<BoundaryID>& bids=_mesh.get_boundary_ids();
  std::set<BoundaryID>::const_iterator sit;
  sit=bids.find(_boundary1);
  if (sit == bids.end())
    mooseError("NearestNodeLocator being created for boundaries "<<_boundary1<<" and "<<_boundary2<<", but boundary "<<_boundary1<<" does not exist");
  sit=bids.find(_boundary2);
  if (sit == bids.end())
    mooseError("NearestNodeLocator being created for boundaries "<<_boundary1<<" and "<<_boundary2<<", but boundary "<<_boundary2<<" does not exist");
  */
}

NearestNodeLocator::~NearestNodeLocator()
{
  delete _slave_node_range;
}

void
NearestNodeLocator::findNodes()
{
  Moose::perf_log.push("NearestNodeLocator::findNodes()","Solve");

  /**
   * If this is the first time through we're going to build up a "neighborhood" of nodes
   * surrounding each of the slave nodes.  This will speed searching later.
   */
  if(_first)
  {
    _first=false;

    // Trial slave nodes are all the nodes on the slave side
    // We only keep the ones that are either on this processor or are likely
    // to interact with elements on this processor (ie nodes owned by this processor
    // are in the "neighborhood" of the slave node
    std::vector<unsigned int> trial_slave_nodes;
    std::vector<unsigned int> trial_master_nodes;


    // Build a bounding box.  No reason to consider nodes outside of our inflated BB
    MeshTools::BoundingBox * my_inflated_box = NULL;

    std::vector<Real> & inflation = _mesh.getGhostedBoundaryInflation();

    // This means there was a user specified inflation... so we can build a BB
    if(inflation.size() > 0)
    {
      MeshTools::BoundingBox my_box = MeshTools::processor_bounding_box(_mesh, libMesh::processor_id());

      Real distance_x = 0;
      Real distance_y = 0;
      Real distance_z = 0;

      distance_x = inflation[0];

      if(inflation.size() > 1)
        distance_y = inflation[1];

      if(inflation.size() > 2)
        distance_z = inflation[2];

      my_inflated_box = new MeshTools::BoundingBox(Point(my_box.first(0)-distance_x,
                                                         my_box.first(1)-distance_y,
                                                         my_box.first(2)-distance_z),
                                                   Point(my_box.second(0)+distance_x,
                                                         my_box.second(1)+distance_y,
                                                         my_box.second(2)+distance_z));
    }

    // Data structures to hold the Nodal Boundary conditions
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;
      unsigned int node_id = bnode->_node->id();

      // If we have a BB only consider saving this node if it's in our inflated BB
      if(!my_inflated_box || (my_inflated_box->contains_point(*bnode->_node)))
      {
        if(boundary_id == _boundary1)
          trial_master_nodes.push_back(node_id);
        else if(boundary_id == _boundary2)
          trial_slave_nodes.push_back(node_id);
      }
    }

    // don't need the BB anymore
    delete my_inflated_box;

    std::map<unsigned int, std::vector<unsigned int> > & node_to_elem_map = _mesh.nodeToElemMap();

    NodeIdRange trial_slave_node_range(trial_slave_nodes.begin(), trial_slave_nodes.end(), 1);

    SlaveNeighborhoodThread snt(_mesh, trial_master_nodes, node_to_elem_map, _mesh.getPatchSize());

    Threads::parallel_reduce(trial_slave_node_range, snt);

    _slave_nodes = snt._slave_nodes;
    _neighbor_nodes = snt._neighbor_nodes;

    for(std::set<unsigned int>::iterator it = snt._ghosted_elems.begin();
        it != snt._ghosted_elems.end();
        ++it)
      _subproblem.addGhostedElem(*it);

    // Cache the slave_node_range so we don't have to build it each time
    _slave_node_range = new NodeIdRange(_slave_nodes.begin(), _slave_nodes.end(), 1);
  }

  _nearest_node_info.clear();

  NearestNodeThread nnt(_mesh, _neighbor_nodes);

  Threads::parallel_reduce(*_slave_node_range, nnt);

  _nearest_node_info = nnt._nearest_node_info;

  Moose::perf_log.pop("NearestNodeLocator::findNodes()","Solve");
}

Real
NearestNodeLocator::distance(unsigned int node_id)
{
  return _nearest_node_info[node_id]._distance;
}


const Node *
NearestNodeLocator::nearestNode(unsigned int node_id)
{
  return _nearest_node_info[node_id]._nearest_node;
}

//===================================================================
NearestNodeLocator::NearestNodeInfo::NearestNodeInfo() :
    _nearest_node(NULL),
    _distance(std::numeric_limits<Real>::max())
{}
