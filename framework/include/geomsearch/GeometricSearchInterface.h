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

#ifndef GEOMETRICSEARCHINTERFACE_H
#define GEOMETRICSEARCHINTERFACE_H

#include "InputParameters.h"

#include "libmesh/enum_order.h"

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
  PenetrationLocator & getPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order);

  /**
   * Retrieve the Quadrature PentrationLocator associated with the two sides.
   *
   * A "Quadrature" vesion means that it's going to find the penetration each quadrature point on this boundary
   */
  PenetrationLocator & getQuadraturePenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order);

  /**
   * Retrieve the PentrationLocator associated with the two sides.
   */
  NearestNodeLocator & getNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave);

  /**
   * Retrieve a Quadrature NearestNodeLocator associated with the two sides.
   *
   * A "Quadrature" vesion means that it's going to find the nearest nodes to each quadrature point on this boundary
   */
  NearestNodeLocator & getQuadratureNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave);

protected:
  GeometricSearchData & _geometric_search_data;
};

#endif //GEOMETRICSEARCHINTERFACE_H
