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
 * CSGXCylinder creates an internal representation of a Constructive Solid Geometry (CSG)
 * x-axis aligned cylinder, represented in the following form (y - y0)^2 + (z - z0)^2 = r^2
 */
class CSGXCylinder : public CSGSurface
{
public:
  /**
   * @brief Construct a cylinder surface aligned with the x axis
   *
   * @param name unique name of surface
   * @param y0 y coordinate of center
   * @param z0 z coordinate of center
   * @param r radius
   */
  CSGXCylinder(const std::string & name, const Real y0, const Real z0, const Real r);

  /**
   * Destructor
   */
  virtual ~CSGXCylinder() = default;

  /**
   * @brief Get the coefficients (y0, z0, and r) that define the cylindrical surface
   * with the equation: (y - y0)^2 + (z - z0)^2 = r^2
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
  /**
   * @brief create clone of CSGXCylinder object
   *
   * @return std::unordered_map<CSGSurface> unique_ptr to cloned x-cylinder
   */
  virtual std::unique_ptr<CSGSurface> clone() const override
  {
    return std::make_unique<CSGXCylinder>(_name, _y0, _z0, _r);
  }

  // check that radius is positive
  void checkRadius() const;

  /// Value of y0 in equation of an x-axis aligned cylinder
  Real _y0;

  /// Value of z0 in equation of an x-axis aligned cylinder
  Real _z0;

  /// Value of r in equation of an x-axis aligned cylinder
  Real _r;
};
} // namespace CSG
