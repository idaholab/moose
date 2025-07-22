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
  /**
   * @brief Construct a new CSGPlane surface from three non co-linear points
   *
   * @param name unique name of plane
   * @param p1 point 1
   * @param p2 point 2
   * @param p3 point 3
   * @param boundary (optional) boundary type for the surface, default "TRANSMISSION"
   */
  CSGPlane(const std::string & name,
           const Point & p1,
           const Point & p2,
           const Point & p3,
           std::string boundary = "TRANSMISSION");

  /**
   * @brief Construct a new CSGPlane surface from coefficients (a, b, c, d) for the
   * equation of a plane: aX + bY + cZ = d
   *
   * @param name unique name of plane
   * @param a coefficient a
   * @param b coefficient b
   * @param c coefficient c
   * @param d coefficient d
   * @param boundary (optional) boundary type for the surface, default "TRANSMISSION"
   */
  CSGPlane(const std::string & name,
           const Real a,
           const Real b,
           const Real c,
           const Real d,
           std::string boundary = "TRANSMISSION");

  /**
   * Destructor
   */
  virtual ~CSGPlane() = default;

  /**
   * @brief get coefficients (a, b, c, d) of the Plane aX + bY + cZ = d
   *
   * @return std::unordered_map<std::string, Real> map of coefficients (a, b, c, and d) and their
   * values
   */
  virtual std::unordered_map<std::string, Real> getCoeffs() const override;

  /**
   * @brief given a point, determine if it is in the positive or negative
   * half-space of the plane as compared to the surface normal
   *
   * @param p point
   * @return sign of the half-space
   */
  virtual CSGSurface::Halfspace getHalfspaceFromPoint(const Point & p) const override;

protected:
  // calculate the equivalent coeffients (aX + bY + cZ = d) from 3 points on a plane
  void coeffsFromPoints(const Point & p1, const Point & p2, const Point & p3);

  /// Value of a in equation of plane
  Real _a;

  /// Value of b in equation of plane
  Real _b;

  /// Value of c in equation of plane
  Real _c;

  /// Value of d in equation of plane
  Real _d;
};
} // namespace CSG
