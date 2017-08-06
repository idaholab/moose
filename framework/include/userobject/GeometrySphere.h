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

#ifndef GEOMETRYSPHERE_H
#define GEOMETRYSPHERE_H

#include "GeometryBase.h"

class GeometrySphere;

template <>
InputParameters validParams<GeometrySphere>();

/**
 * Snaps the selected nodes to the surface of a sphere (or circular disk in 2D)
 */
class GeometrySphere : public GeometryBase
{
public:
  GeometrySphere(const InputParameters & parameters);

protected:
  virtual void snapNode(Node & node);

  const Point _center;
  const Real _radius;
};

#endif // GEOMETRYSPHERE_H
