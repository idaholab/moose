//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurface.h"
#include "CSGHalfspace.h"

#include "libmesh/point.h"

namespace CSG
{

/**
 * CSGZCylinder creates an internal representation of a Constructive Solid Geometry (CSG)
 * z-axis aligned cylinder, represented in the following form (x - x0)^2 + (y - y0)^2 = r^2
 */
class CSGZCylinder : public CSGSurface
{
public:
  CSGZCylinder(const std::string name, const Real x0, const Real y0, const Real r);

  /**
   * Destructor
   */
  virtual ~CSGZCylinder() = default;

  virtual std::map<std::string, Real> getCoeffs() override;

  virtual CSGSurface::Direction directionFromPoint(const Point p) override;

protected:
  /// Value of x0 in equation of an z-axis aligned cylinder
  Real _x0;

  /// Value of y0 in equation of an z-axis aligned cylinder
  Real _y0;

  /// Value of r in equation of an z-axis aligned cylinder
  Real _r;
};
} // namespace CSG
