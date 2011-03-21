#ifndef GEOMETRICSEARCHINTERFACE_H_
#define GEOMETRICSEARCHINTERFACE_H_

#include "InputParameters.h"

namespace Moose {

// Forward Declarations
class GeometricSearchData;
class PenetrationLocator;
class NearestNodeLocator;


class GeometricSearchInterface
{
public:
  GeometricSearchInterface(InputParameters & params);

  /**
   * Retrieve the PentrationLocator associated with the two sides.
   */
  PenetrationLocator & getPenetrationLocator(unsigned int master, unsigned int slave);

  /**
   * Retrieve the PentrationLocator associated with the two sides.
   */
  NearestNodeLocator & getNearestNodeLocator(unsigned int master, unsigned int slave);

protected:
  GeometricSearchData & _geometric_search_data;
};

} // namespace

#endif //GEOMETRICSEARCHINTERFACE_H_
