#ifndef PENETRATIONLOCATOR_H
#define PENETRATIONLOCATOR_H

#include "libmesh_common.h"

#include <vector>
#include <map>

#include "mesh.h"

class PenetrationLocator
{
public:

  PenetrationLocator(Mesh & mesh, std::vector<unsigned int> master, unsigned int slave);
  void detectPenetration();

  Real penetrationDistance(unsigned int node_id) const;
  
private:

  Real normDistance(const Elem & side, const Point & p0);
  
  Mesh & _mesh;
  std::vector<unsigned int> _master_boundary;
  unsigned int _slave_boundary;

  std::map<unsigned int, std::pair<unsigned int, Real> > _penetrated_elems;
};


#endif //PENETRATIONLOCATOR_H
