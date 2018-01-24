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
                                                const BoundaryName & slave,
                                                Order order)
{
  return _geometric_search_data.getPenetrationLocator(master, slave, order);
}

PenetrationLocator &
GeometricSearchInterface::getQuadraturePenetrationLocator(const BoundaryName & master,
                                                          const BoundaryName & slave,
                                                          Order order)
{
  return _geometric_search_data.getQuadraturePenetrationLocator(master, slave, order);
}

PenetrationLocator &
GeometricSearchInterface::getMortarPenetrationLocator(const BoundaryName & master,
                                                      const BoundaryName & slave,
                                                      Moose::ConstraintType side_type,
                                                      Order order)
{
  return _geometric_search_data.getMortarPenetrationLocator(master, slave, side_type, order);
}

NearestNodeLocator &
GeometricSearchInterface::getNearestNodeLocator(const BoundaryName & master,
                                                const BoundaryName & slave)
{
  return _geometric_search_data.getNearestNodeLocator(master, slave);
}

NearestNodeLocator &
GeometricSearchInterface::getQuadratureNearestNodeLocator(const BoundaryName & master,
                                                          const BoundaryName & slave)
{
  return _geometric_search_data.getQuadratureNearestNodeLocator(master, slave);
}

NearestNodeLocator &
GeometricSearchInterface::getMortarNearestNodeLocator(const BoundaryName & master,
                                                      const BoundaryName & slave,
                                                      Moose::ConstraintType side_type)
{
  return _geometric_search_data.getMortarNearestNodeLocator(master, slave, side_type);
}
