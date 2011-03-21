#ifndef GEOMETRICSEARCHDATA_H_
#define GEOMETRICSEARCHDATA_H_

//libmesh includes
#include "mesh.h"

#include <map>

namespace Moose {

//Forward Declarations
class Mesh;
class SubProblem;
class PenetrationLocator;
class NearestNodeLocator;

class GeometricSearchData
{
public:
  GeometricSearchData(SubProblem & subproblem, Mesh & mesh);

  PenetrationLocator & getPenetrationLocator(unsigned int master, unsigned int slave);

  NearestNodeLocator & getNearestNodeLocator(unsigned int master, unsigned int slave);

  /**
   * Update all of the search objects.
   *
   * This is probably getting called because the mesh changed in some way.
   */
  void update();

protected:
  SubProblem & _subproblem;
  Mesh & _mesh;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> _penetration_locators;
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> _nearest_node_locators;
};

} // namespace

#endif //GEOMETRICSEARCHDATA_H
