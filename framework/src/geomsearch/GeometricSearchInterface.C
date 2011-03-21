#include "GeometricSearchInterface.h"
#include "GeometricSearchData.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "SystemBase.h"

GeometricSearchInterface::GeometricSearchInterface(InputParameters & params) :
    _geometric_search_data(params.get<SubProblemInterface *>("_subproblem")->geomSearchData())
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
