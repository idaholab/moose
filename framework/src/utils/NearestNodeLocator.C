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

#include "boundary_info.h"
#include "elem.h"
#include "plane.h"

NearestNodeLocator::NearestNodeLocator(MooseSystem & moose_system, Mesh & mesh, unsigned int boundary1, unsigned int boundary2)
  :_moose_system(moose_system),
   _mesh(mesh),
   _boundary1(boundary1),
   _boundary2(boundary2),
   _first(true)
{}


void
NearestNodeLocator::findNodes()
{
  Moose::perf_log.push("NearestNodeLocator::findNodes()","Solve");

  _nearest_node_info.clear();

  if(_first)
  {
    _first=false;
    
    // Data strcutres to hold the Nodal Boundary conditions
    std::vector< unsigned int > node_list;
    std::vector< short int > node_boundary_list;
//  _mesh.boundary_info->build_node_list_from_side_list();
    _mesh.boundary_info->build_node_list(node_list, node_boundary_list);

    const unsigned int n_nodes = node_list.size();

    for(unsigned int i=0; i<n_nodes; i++)
    {
      unsigned int boundary_id = node_boundary_list[i];
      unsigned int node_id = node_list[i];

      if(boundary_id == _boundary1)
        _master_nodes.push_back(node_id);
      else if(boundary_id == _boundary2)
        _slave_nodes.push_back(node_id);
    }
  }

  unsigned int n_slave_nodes = _slave_nodes.size();
  unsigned int n_master_nodes = _master_nodes.size();

  for(unsigned int i=0; i<n_slave_nodes; i++)
  {
    unsigned int node_id = _slave_nodes[i];
    
    Node & node = _mesh.node(node_id);

    Node * closest_node = NULL;
    Real closest_distance = 999999999;

    for(unsigned int k=0; k<n_master_nodes; k++)
    {
      Node * cur_node = _mesh.node_ptr(_master_nodes[k]);
      Real distance = ((*cur_node) - node).size();

      if(distance < closest_distance)
      {
        closest_distance = distance;
        closest_node = cur_node;
      }
    }

    if(closest_distance == 999999999)
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
NearestNodeLocator::NearestNodeInfo::NearestNodeInfo()
  : _nearest_node(NULL),
    _distance(9999999999)
{}
