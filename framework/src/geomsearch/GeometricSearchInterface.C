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

  MooseEnum search_methods("nearest_node_connected_sides all_proximate_sides",
                           "nearest_node_connected_sides");

  params.addParam<MooseEnum>(
      "search_method",
      search_methods,
      "Choice of search algorithm.  All options begin by finding the nearest node in the "
      "primary boundary to a query point in the secondary boundary.  In the default "
      "nearest_node_connected_sides algorithm, primary boundary elements are searched iff "
      "that nearest node is one of their nodes.  This is fast to determine via a "
      "pregenerated node-to-elem map and is robust on conforming meshes.  In the optional "
      "all_proximate_sides algorithm, primary boundary elements are searched iff they touch "
      "that nearest node, even if they are not topologically connected to it.  This is "
      "more CPU-intensive but is necessary for robustness on any boundary surfaces which "
      "has disconnections (such as Flex IGA meshes) or non-conformity (such as hanging nodes "
      "in adaptively h-refined meshes).");

  params.addParamNamesToGroup("search_method", "Advanced");

  return params;
}

GeometricSearchInterface::GeometricSearchInterface(const MooseObject * moose_object)
  : _geometric_search_data(moose_object->parameters()
                               .getCheckedPointerParam<SubProblem *>("_subproblem")
                               ->geomSearchData()),
    _requires_geometric_search(false)
{
  if (moose_object->getParam<MooseEnum>("search_method") == "all_proximate_sides")
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
