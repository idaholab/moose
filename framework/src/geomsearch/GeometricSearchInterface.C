//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = emptyInputParameters();

  params.addParam<bool>(
      "search_using_point_locator",
      false,
      "Whether to use the mesh point locator (typically an octree search) to find "
      "elements around closest points of near-contact, contact, or penetration.  "
      "This is less efficient than the default search via node-element connectivity, but "
      "may be necessary to accurately detect gaps, contact, or penetration on any "
      "boundaries whose elements are not connected via shared nodes. ");

  params.addParamNamesToGroup("search_using_point_locator", "Advanced");

  return params;
}

GeometricSearchInterface::GeometricSearchInterface(const MooseObject * moose_object)
  : _geometric_search_data(moose_object->parameters()
                               .getCheckedPointerParam<SubProblem *>("_subproblem")
                               ->geomSearchData()),
    _requires_geometric_search(false)
{
  if (moose_object->getParam<bool>("search_using_point_locator"))
    _geometric_search_data.setSearchUsingPointLocator(true);
}

#ifdef MOOSE_KOKKOS_ENABLED
GeometricSearchInterface::GeometricSearchInterface(const GeometricSearchInterface & object,
                                                   const Moose::Kokkos::FunctorCopy &)
  : _geometric_search_data(object._geometric_search_data),
    _requires_geometric_search(object._requires_geometric_search)
{
}
#endif

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
