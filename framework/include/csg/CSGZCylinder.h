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
 * CSGZCylinder creates an internal representation of a Constructive Solid Geometry (CSG)
 * z-axis aligned cylinder, represented in the following form (x - x0)^2 + (y - y0)^2 = r^2
 */
class CSGZCylinder : public CSGSurface
{
public:
  /**
   * @brief Construct a cylinder surface aligned with the z axis
   *
   * @param name unique name of surface
   * @param x0 x coordinate of center
   * @param y0 y coordinate of center
   * @param r radius
   */
  CSGZCylinder(const std::string & name, const Real x0, const Real y0, const Real r);

  /**
   * Destructor
   */
  virtual ~CSGZCylinder() = default;

  /**
   * @brief Get the coefficients (x0, y0, and r) that define the cylindrical surface
   * with the equation: (x - x0)^2 + (y - y0)^2 = r^2
   *
   * @return map of coefficients to their value
   */
  virtual std::unordered_map<std::string, Real> getCoeffs() const override;

  /**
   * @brief given a point, determine its evaluation based on the equation of the cylinder
   *
   * @param p point
   * @return evaluation of point based on surface equation
   */
  virtual Real evaluateSurfaceEquationAtPoint(const Point & p) const override;

protected:
  // check that radius is positive
  void checkRadius() const;

  /// Value of x0 in equation of an z-axis aligned cylinder
  Real _x0;

  /// Value of y0 in equation of an z-axis aligned cylinder
  Real _y0;

  /// Value of r in equation of an z-axis aligned cylinder
  Real _r;
};
} // namespace CSG
