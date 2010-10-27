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

#include "NearestNode.h"

#include "Moose.h"

#include "boundary_info.h"
#include "elem.h"
#include "plane.h"

NearestNode::NearestNode(MooseSystem & moose_system, Mesh & mesh, unsigned int boundary1, unsigned int boundary2)
  :_moose_system(moose_system),
  _mesh(mesh),
  _boundary1(boundary1),
  _boundary2(boundary2)
{}


void
NearestNode::findNodes()
{
  Moose::perf_log.push("NearestNode::findNodes()","Solve");

  _nearest_node_info.clear();

  // Data strcutres to hold the Nodal Boundary conditions
  std::vector< unsigned int > node_list;
  std::vector< short int > node_boundary_list;
  _mesh.boundary_info->build_node_list_from_side_list();
  _mesh.boundary_info->build_node_list(node_list, node_boundary_list);

  const unsigned int n_nodes = node_list.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = node_boundary_list[i];

    unsigned int node_id = node_list[i];

    // If this node is on one of the boundaries we're interested in.
    if(boundary_id == _boundary1 || boundary_id == _boundary2)
    {
      Node & node = _mesh.node(node_id);
      
      if(node.processor_id() == libMesh::processor_id())
      {          
        Node * closest_node = NULL;
        
        Real closest_distance = 999999999;
        
        for(unsigned int k=0; k<n_nodes; k++)  
        {
          short int other_boundary_id = node_boundary_list[k];

          if(other_boundary_id != boundary_id && (other_boundary_id == _boundary1 || other_boundary_id == _boundary2))
          {
            Node * cur_node = _mesh.node_ptr(node_list[k]);
                                             
            Real distance = ((*cur_node) - node).size();

            if(distance < closest_distance)
            {
              closest_distance = distance;
              closest_node = cur_node;
            }
          }
        }

        if(closest_distance == 999999999)
          mooseError("Unable to find nearest node!");

        NearestNodeInfo * info = &_nearest_node_info[node.id()];

        if(closest_distance < info->_distance)
        {
          info->_nearest_node = closest_node;
          info->_distance = closest_distance;
        }
      }
    }
  }        

  Moose::perf_log.pop("NearestNode::findNodes()","Solve");
}

Real
NearestNode::distance(unsigned int node_id)
{
  return _nearest_node_info[node_id]._distance;
}


Node *
NearestNode::nearestNode(unsigned int node_id)
{
  return _nearest_node_info[node_id]._nearest_node;
}

//===================================================================
NearestNode::NearestNodeInfo::NearestNodeInfo()
  : _nearest_node(NULL),
    _distance(9999999999)
{}
