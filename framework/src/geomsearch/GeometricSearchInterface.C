//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricSearchInterface.h"

// MOOSE includes
#include "GeometricSearchData.h"
#include "MooseObject.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "SubProblem.h"
#include "SystemBase.h"

InputParameters
GeometricSearchInterface::validParams()
{
  return emptyInputParameters();
}

GeometricSearchInterface::GeometricSearchInterface(const MooseObject * moose_object)
  : _geometric_search_data(moose_object->parameters()
                               .getCheckedPointerParam<SubProblem *>("_subproblem")
                               ->geomSearchData()),
    _requires_geometric_search(false)
{
}

PenetrationLocator &
GeometricSearchInterface::getPenetrationLocator(const BoundaryName & primary,
                                                const BoundaryName & secondary,
                                                Order order)
{
  _requires_geometric_search = true;
  return _geometric_search_data.getPenetrationLocator(primary, secondary, order);
}

PenetrationLocator &
GeometricSearchInterface::getQuadraturePenetrationLocator(const BoundaryName & primary,
                                                          const BoundaryName & secondary,
                                                          Order order)
{
  _requires_geometric_search = true;
  return _geometric_search_data.getQuadraturePenetrationLocator(primary, secondary, order);
}

NearestNodeLocator &
GeometricSearchInterface::getNearestNodeLocator(const BoundaryName & primary,
                                                const BoundaryName & secondary)
{
  _requires_geometric_search = true;
  return _geometric_search_data.getNearestNodeLocator(primary, secondary);
}

NearestNodeLocator &
GeometricSearchInterface::getQuadratureNearestNodeLocator(const BoundaryName & primary,
                                                          const BoundaryName & secondary)
{
  _requires_geometric_search = true;
  return _geometric_search_data.getQuadratureNearestNodeLocator(primary, secondary);
}
