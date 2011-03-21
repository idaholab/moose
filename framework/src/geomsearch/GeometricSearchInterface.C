#include "GeometricSearchInterface.h"
#include "GeometricSearchData.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"

GeometricSearchInterface::GeometricSearchInterface(InputParameters & params) :
    _geometric_search_data(*params.get<GeometricSearchData *>("_geometric_search_data"))
{
}

PenetrationLocator &
GeometricSearchInterface::getPenetrationLocator(unsigned int master, unsigned int slave)
{
  return _geometric_search_data.getPenetrationLocator(master, slave);
}

NearestNodeLocator &
GeometricSearchInterface::getNearestNodeLocator(unsigned int master, unsigned int slave)
{
  return _geometric_search_data.getNearestNodeLocator(master, slave);
}
