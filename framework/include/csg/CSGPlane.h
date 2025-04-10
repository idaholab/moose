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

  /**
   * @brief get coefficients (a, b, c, d) of the Plane aX + bY + cZ = d
   *
   * @return std::map<std::string, Real>
   */
  virtual std::map<std::string, Real> getCoeffs() override;

  /**
   * @brief get direction from point p to plane
   *
   * @param p
   * @return CSGSurface::Direction
   */
  virtual CSGSurface::Direction directionFromPoint(const Point p) override;

protected:
  // calculate the equivalent coeffients (aX + bY + cZ = d) from 3 points on a plane
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
