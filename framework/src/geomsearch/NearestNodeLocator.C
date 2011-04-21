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
#include "Moose.h"
#include "MooseMesh.h"
#include "SubProblem.h"

#include <queue>

// libMesh
#include "boundary_info.h"
#include "elem.h"
#include "plane.h"
#include "mesh_tools.h"

const unsigned int NearestNodeLocator::_patch_size = 20;

NearestNodeLocator::NearestNodeLocator(SubProblem & subproblem, MooseMesh & mesh, unsigned int boundary1, unsigned int boundary2) :
    _subproblem(subproblem),
    _mesh(mesh),
    _boundary1(boundary1),
    _boundary2(boundary2),
    _first(true)
{}

class ComparePair
{
public:
  bool operator()(std::pair<unsigned int, Real> & p1, std::pair<unsigned int, Real> & p2)
  {
    if(p1.second > p2.second)
      return true;

    return false;
  }
};

void
NearestNodeLocator::findNodes()
{
  Moose::perf_log.push("NearestNodeLocator::findNodes()","Solve");

  Mesh & mesh = _mesh._mesh;

  _nearest_node_info.clear();

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
      MeshTools::BoundingBox my_box = MeshTools::processor_bounding_box(mesh, libMesh::processor_id());

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
    
    // Data strcutres to hold the Nodal Boundary conditions
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      unsigned int boundary_id = bnode->_bnd_id;
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

    unsigned int n_slave_nodes = trial_slave_nodes.size();
    unsigned int n_master_nodes = trial_master_nodes.size();

    std::vector<std::vector<unsigned int> > & node_to_elem_map = _mesh.nodeToElemMap();

    unsigned int processor_id = libMesh::processor_id();

    /**
     * Save a patch of nodes that are close to each of the slave nodes to speed the search algorithm
     * TODO: This needs to be updated at some point in time.  If the hits into this data structure approach "the end"
     * then it may be time to update
     */
    for(unsigned int i=0; i<n_slave_nodes; i++)
    {
      unsigned int node_id = trial_slave_nodes[i];

      std::priority_queue<std::pair<unsigned int, Real>, std::vector<std::pair<unsigned int, Real> >, ComparePair> neighbors;

      Node & node = _mesh.node(node_id);

      for(unsigned int k=0; k<n_master_nodes; k++)
      {
        unsigned int master_id = trial_master_nodes[k];
        Node * cur_node = &_mesh.node(master_id);
        Real distance = ((*cur_node) - node).size();

        neighbors.push(std::make_pair(master_id, distance));
      }

      std::vector<unsigned int> neighbor_nodes;// = _neighbor_nodes[node_id];

      unsigned int patch_size = std::min(_patch_size, static_cast<unsigned int>(neighbors.size()));
      neighbor_nodes.resize(patch_size);
      
      for(unsigned int t=0; t<patch_size; t++)
      {
        std::pair<unsigned int, Real> neighbor_info = neighbors.top();
        neighbors.pop();

        neighbor_nodes[t] = neighbor_info.first;
      }

      /**
       * Now see if _this_ processor needs to keep track of this slave and it's neighbors
       * We're going to see if this processor owns the slave, any of the neighborhood nodes
       * or any of the elements connected to either set.  If it does then we're going to ghost
       * all of the elements connected to the slave node and the neighborhood nodes to this processor.
       * This is a very conservative approach that we might revisit later.
       */

      bool need_to_track = false;

      if(_mesh.node(node_id).processor_id() == processor_id)
        need_to_track = true;
      else
      {
        { // See if we own any of the elements connected to the slave node
          std::vector<unsigned int> & elems_connected_to_node = node_to_elem_map[node_id];
          
          for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
            if(_mesh.elem(elems_connected_to_node[elem_id_it])->processor_id() == processor_id)
            {
              need_to_track = true;
              break; // Break out of element loop
            }
        }

        if(!need_to_track)
        { // Now check the neighbor nodes to see if we own any of them
          for(unsigned int neighbor_it=0; neighbor_it < neighbor_nodes.size(); neighbor_it++)
          {
            unsigned int neighbor_node_id = neighbor_nodes[neighbor_it];
            
            if(_mesh.node(neighbor_node_id).processor_id() == processor_id)
              need_to_track = true;
            else // Now see if we own any of the elements connected to the neighbor nodes
            {
              std::vector<unsigned int> & elems_connected_to_node = node_to_elem_map[neighbor_node_id];
              
              for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
                if(_mesh.elem(elems_connected_to_node[elem_id_it])->processor_id() == processor_id)
                {
                  need_to_track = true;
                  break; // Break out of element loop
                }
            }
            
            if(need_to_track)
              break; // Breaking out of neighbor loop
          }
        }
      }

      if(need_to_track)
      {
        // Add this node as a slave node to search in the future
        _slave_nodes.push_back(node_id);

        // Set it's neighbors
        _neighbor_nodes[node_id] = neighbor_nodes;
        
        { // Add the elements connected to the slave node to the ghosted list
          std::vector<unsigned int> & elems_connected_to_node = node_to_elem_map[node_id];

          for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
            _subproblem.addGhostedElem(elems_connected_to_node[elem_id_it]);
        }

        // Now add elements connected to the neighbor nodes to the ghosted list
        for(unsigned int neighbor_it=0; neighbor_it < neighbor_nodes.size(); neighbor_it++)
        {
          std::vector<unsigned int> & elems_connected_to_node = node_to_elem_map[neighbor_nodes[neighbor_it]];

          for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
            _subproblem.addGhostedElem(elems_connected_to_node[elem_id_it]);
        }
      }
    }
  }    

  unsigned int n_slave_nodes = _slave_nodes.size();

  for(unsigned int i=0; i<n_slave_nodes; i++)
  {
    unsigned int node_id = _slave_nodes[i];
    
    Node & node = _mesh.node(node_id);

    Node * closest_node = NULL;
    Real closest_distance = std::numeric_limits<Real>::max();

    std::vector<unsigned int> & neighbor_nodes = _neighbor_nodes[node_id];

    unsigned int n_neighbor_nodes = neighbor_nodes.size();

    for(unsigned int k=0; k<n_neighbor_nodes; k++)
    {
      Node * cur_node = &_mesh.node(neighbor_nodes[k]);
      Real distance = ((*cur_node) - node).size();

      if(distance < closest_distance)
      {
        closest_distance = distance;
        closest_node = cur_node;
      }
    }

    if(closest_distance == std::numeric_limits<Real>::max())
      mooseError("Unable to find nearest node!");

    NearestNodeInfo & info = _nearest_node_info[node.id()];

    if(closest_distance < info._distance)
    {
      info._nearest_node = closest_node;
      info._distance = closest_distance;
    }
  }  

  Moose::perf_log.pop("NearestNodeLocator::findNodes()","Solve");
}

Real
NearestNodeLocator::distance(unsigned int node_id)
{
  return _nearest_node_info[node_id]._distance;
}


Node *
NearestNodeLocator::nearestNode(unsigned int node_id)
{
  return _nearest_node_info[node_id]._nearest_node;
}

//===================================================================
NearestNodeLocator::NearestNodeInfo::NearestNodeInfo() :
    _nearest_node(NULL),
    _distance(std::numeric_limits<Real>::max())
{}
