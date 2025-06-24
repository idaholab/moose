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
 * CSGSphere creates an internal representation of a Constructive Solid Geometry (CSG)
 * sphere, represented in the form (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = r^2
 */
class CSGSphere : public CSGSurface
{
public:
  /**
   * @brief Construct a new CSGSphere surface
   *
   * @param name unique name for the sphere surface
   * @param center center point of sphere
   * @param r radius of sphere
   * @param boundary CSGSurface::BoundaryType boundary condition for the surface
   */
  CSGSphere(const std::string name,
            const Point center,
            const Real r,
            CSGSurface::BoundaryType boundary);

  /**
   * Destructor
   */
  virtual ~CSGSphere() = default;

  /**
   * @brief Get the coefficients (x0, y0, z0, r) for the equation of a sphere
   * (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = r^2
   *
   * @return std::map<std::string, Real> map of coefficients (x0, y0, z0, and r) and their values
   */
  virtual std::map<std::string, Real> getCoeffs() const override;

  /**
   * @brief get direction from point to sphere surface
   *
   * @param p point
   * @return CSGSurface::Direction direction from point
   */
  virtual CSGSurface::Direction directionFromPoint(const Point p) const override;

protected:
  /// Value of x0 in equation of sphere
  Real _x0;

  /// Value of y0 in equation of sphere
  Real _y0;

  /// Value of z0 in equation of sphere
  Real _z0;

  /// Value of r in equation of sphere
  Real _r;
};
} // namespace CSG
