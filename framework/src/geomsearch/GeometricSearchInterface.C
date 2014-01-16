/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GeometricSearchInterface.h"
#include "GeometricSearchData.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "SystemBase.h"

GeometricSearchInterface::GeometricSearchInterface(InputParameters & params) :
    _geometric_search_data(params.get<SubProblem *>("_subproblem")->geomSearchData())
{
}

PenetrationLocator &
GeometricSearchInterface::getPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order)
{
  return _geometric_search_data.getPenetrationLocator(master, slave, order);
}

PenetrationLocator &
GeometricSearchInterface::getQuadraturePenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order)
{
  return _geometric_search_data.getQuadraturePenetrationLocator(master, slave, order);
}

PenetrationLocator &
GeometricSearchInterface::getMortarPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Moose::ConstraintType side_type, Order order)
{
  return _geometric_search_data.getMortarPenetrationLocator(master, slave, side_type, order);
}

NearestNodeLocator &
GeometricSearchInterface::getNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave)
{
  return _geometric_search_data.getNearestNodeLocator(master, slave);
}

NearestNodeLocator &
GeometricSearchInterface::getQuadratureNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave)
{
  return _geometric_search_data.getQuadratureNearestNodeLocator(master, slave);
}

NearestNodeLocator &
GeometricSearchInterface::getMortarNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave, Moose::ConstraintType side_type)
{
  return _geometric_search_data.getMortarNearestNodeLocator(master, slave, side_type);
}
