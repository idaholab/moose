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

GeometricSearchInterface::GeometricSearchInterface(const MooseObject * moose_object)
  : _geometric_search_data(moose_object->parameters()
                               .getCheckedPointerParam<SubProblem *>("_subproblem")
                               ->geomSearchData())
{
}

PenetrationLocator &
GeometricSearchInterface::getPenetrationLocator(const BoundaryName & master,
                                                const BoundaryName & secondary,
                                                Order order)
{
  return _geometric_search_data.getPenetrationLocator(master, secondary, order);
}

PenetrationLocator &
GeometricSearchInterface::getQuadraturePenetrationLocator(const BoundaryName & master,
                                                          const BoundaryName & secondary,
                                                          Order order)
{
  return _geometric_search_data.getQuadraturePenetrationLocator(master, secondary, order);
}

NearestNodeLocator &
GeometricSearchInterface::getNearestNodeLocator(const BoundaryName & master,
                                                const BoundaryName & secondary)
{
  return _geometric_search_data.getNearestNodeLocator(master, secondary);
}

NearestNodeLocator &
GeometricSearchInterface::getQuadratureNearestNodeLocator(const BoundaryName & master,
                                                          const BoundaryName & secondary)
{
  return _geometric_search_data.getQuadratureNearestNodeLocator(master, secondary);
}
