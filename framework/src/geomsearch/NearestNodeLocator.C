#include "NearestNodeLocator.h"
#include "Moose.h"
#include "Mesh.h"
#include "SubProblem.h"

#include <queue>

#include "boundary_info.h"
#include "elem.h"
#include "plane.h"

namespace Moose {

const unsigned int NearestNodeLocator::_patch_size = 20;

NearestNodeLocator::NearestNodeLocator(Mesh & mesh, unsigned int boundary1, unsigned int boundary2) :
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

  _nearest_node_info.clear();

  if(_first)
  {
    _first=false;
    
    // Data strcutres to hold the Nodal Boundary conditions
    std::vector< unsigned int > node_list;
    std::vector< short int > node_boundary_list;
//  _mesh.boundary_info->build_node_list_from_side_list();
    _mesh.build_node_list(node_list, node_boundary_list);

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

    unsigned int n_slave_nodes = _slave_nodes.size();
    unsigned int n_master_nodes = _master_nodes.size();

    /**
     * Save a patch of nodes that are close to each of the slave nodes to speed the search algorithm
     * TODO: This needs to be updated at some point in time.  If the hits into this data structure approach "the end"
     * then it may be time to update
     */
    for(unsigned int i=0; i<n_slave_nodes; i++)
    {
      unsigned int node_id = _slave_nodes[i];

      std::priority_queue<std::pair<unsigned int, Real>, std::vector<std::pair<unsigned int, Real> >, ComparePair> neighbors;

      Node & node = _mesh.node(node_id);

      for(unsigned int k=0; k<n_master_nodes; k++)
      {
        unsigned int master_id = _master_nodes[k];
        Node * cur_node = &_mesh.node(master_id);
        Real distance = ((*cur_node) - node).size();

        neighbors.push(std::make_pair(master_id, distance));
      }

      std::vector<unsigned int> & neighbor_nodes = _neighbor_nodes[node_id];

      unsigned int patch_size = std::min(_patch_size, static_cast<unsigned int>(neighbors.size()));
      neighbor_nodes.resize(patch_size);
      
      for(unsigned int t=0; t<patch_size; t++)
      {
        std::pair<unsigned int, Real> neighbor_info = neighbors.top();
        neighbors.pop();

        neighbor_nodes[t] = neighbor_info.first;
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

}
