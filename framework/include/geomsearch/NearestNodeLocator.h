#ifndef NEARESTNODELOCATOR_H_
#define NEARESTNODELOCATOR_H_

#include "libmesh_common.h"

#include <vector>
#include <map>

#include "mesh.h"
#include "vector_value.h"

namespace Moose {

class Mesh;
class SubProblem;

/**
 * Finds the nearest node to each node in boundary1 to each node in boundary2 and the other way around.
 */
class NearestNodeLocator
{
public:
  NearestNodeLocator(Mesh & mesh, unsigned int boundary1, unsigned int boundary2);

  /**
   * This is the main method that is going to start the search.
   */
  void findNodes();

  /**
   * Valid to call this after findNodes() has been called to get the distance to the nearest node.
   */
  Real distance(unsigned int node_id);

  /**
   * Valid to call this after findNodes() has been called to get a pointer to the nearest node.
   */
  Node * nearestNode(unsigned int node_id);

protected:
  /**
   * Data structure used to hold nearest node info.
   */
  class NearestNodeInfo
  {
  public:
    NearestNodeInfo();

    Node * _nearest_node;
    Real _distance;
  };

  Mesh & _mesh;

public:
  std::map<unsigned int, NearestNodeInfo> _nearest_node_info;

  unsigned int _boundary1;
  unsigned int _boundary2;

  bool _first;
  std::vector<unsigned int> _slave_nodes;
  std::vector<unsigned int> _master_nodes;

  std::map<unsigned int, std::vector<unsigned int> > _neighbor_nodes;

  // The following parameter controls the patch size that is searched for each nearest neighbor
  static const unsigned int _patch_size;
};

} // namespace

#endif //NEARESTNODELOCATOR_H_
