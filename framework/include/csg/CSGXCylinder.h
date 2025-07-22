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
   * @param boundary (optional) boundary type for the surface, default "TRANSMISSION"
   */
  CSGXCylinder(const std::string & name,
               const Real y0,
               const Real z0,
               const Real r,
               std::string boundary = "TRANSMISSION");

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
   * @brief get direction from point to surface
   *
   * @param p point
   * @return sign of direction to surface from point
   */
  virtual CSGSurface::Direction getHalfspaceFromPoint(const Point & p) const override;

protected:
  /// Value of y0 in equation of an x-axis aligned cylinder
  Real _y0;

  /// Value of z0 in equation of an x-axis aligned cylinder
  Real _z0;

  /// Value of r in equation of an x-axis aligned cylinder
  Real _r;
};
} // namespace CSG
