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

#include "libmesh/point.h"

namespace CSG
{

/**
 * CSGYCylinder creates an internal representation of a Constructive Solid Geometry (CSG)
 * y-axis aligned cylinder, represented in the following form (x - x0)^2 + (z - z0)^2 = r^2
 */
class CSGYCylinder : public CSGSurface
{
public:
  CSGYCylinder(const std::string name, const Real x0, const Real z0, const Real r);

  /**
   * Destructor
   */
  virtual ~CSGYCylinder() = default;

  virtual std::map<std::string, Real> getCoeffs() override;

  virtual CSGSurface::Direction directionFromPoint(const Point p) override;

protected:
  /// Value of x0 in equation of an y-axis aligned cylinder
  Real _x0;

  /// Value of z0 in equation of an y-axis aligned cylinder
  Real _z0;

  /// Value of r in equation of an y-axis aligned cylinder
  Real _r;
};
} // namespace CSG
