//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/enum_order.h"

// Forward Declarations
class GeometricSearchData;
class PenetrationLocator;
class NearestNodeLocator;
class MooseObject;
class BoundaryName;

class GeometricSearchInterface
{
public:
  static InputParameters validParams();

  GeometricSearchInterface(const MooseObject * moose_object);

  /**
   * Retrieve the PenetrationLocator associated with the two sides.
   */
  PenetrationLocator &
  getPenetrationLocator(const BoundaryName & primary, const BoundaryName & secondary, Order order);

  /**
   * Retrieve the Quadrature PentrationLocator associated with the two sides.
   *
   * A "Quadrature" version means that it's going to find the penetration each quadrature point on
   * this boundary
   */
  PenetrationLocator & getQuadraturePenetrationLocator(const BoundaryName & primary,
                                                       const BoundaryName & secondary,
                                                       Order order);

  /**
   * Retrieve the PentrationLocator associated with the two sides.
   */
  NearestNodeLocator & getNearestNodeLocator(const BoundaryName & primary,
                                             const BoundaryName & secondary);

  /**
   * Retrieve a Quadrature NearestNodeLocator associated with the two sides.
   *
   * A "Quadrature" version means that it's going to find the nearest nodes to each quadrature point
   * on this boundary
   */
  NearestNodeLocator & getQuadratureNearestNodeLocator(const BoundaryName & primary,
                                                       const BoundaryName & secondary);

  /**
   * Whether any of this interface's methods have been called, e.g. whether the object that this
   * interface is for requires geometric search data
   */
  bool requiresGeometricSearch() const { return _requires_geometric_search; }

protected:
  GeometricSearchData & _geometric_search_data;

  /// Whether any of this interface's methods have been called, e.g. whether the object that this
  /// interface is for requires geometric search data
  bool _requires_geometric_search;
};
