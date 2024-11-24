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
 * CSGPlane creates an internal representation of a Constructive Solid Geometry (CSG)
 * plane, represented in the form aX + bY + cZ = d
 */
class CSGPlane : public CSGSurface
{
public:
  CSGPlane(const std::string name, const Point p1, const Point p2, const Point p3);

  CSGPlane(const std::string name, const Real a, const Real b, const Real c, const Real d);

  /**
   * Destructor
   */
  virtual ~CSGPlane() = default;

  virtual std::map<std::string, Real> getCoeffs() override;

  virtual CSGSurface::Direction directionFromPoint(const Point p) override;

protected:
  void coeffsFromPoints(const Point p1, const Point p2, const Point p3);

  /// Value of a in equation of plane
  Real _a;

  /// Value of b in equation of plane
  Real _b;

  /// Value of b in equation of plane
  Real _c;

  /// Value of b in equation of plane
  Real _d;
};
} // namespace CSG
