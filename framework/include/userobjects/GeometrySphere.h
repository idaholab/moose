//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometryBase.h"

/**
 * Snaps the selected nodes to the surface of a sphere (or circular disk in 2D)
 */
class GeometrySphere : public GeometryBase
{
public:
  static InputParameters validParams();

  GeometrySphere(const InputParameters & parameters);

protected:
  virtual void snapNode(Node & node);

  const Point _center;
  const Real _radius;
};
