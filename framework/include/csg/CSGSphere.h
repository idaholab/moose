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
  CSGSphere(const std::string name, const Point center, const Real r);

  /**
   * Destructor
   */
  virtual ~CSGSphere() = default;

  /**
   * @brief Get the coefficients (x0, y0, z0, r) for the equation of a sphere
   *
   * @return std::map<std::string, Real>
   */
  virtual std::map<std::string, Real> getCoeffs() override;

  /**
   * @brief get direction from point to sphere surface
   *
   * @param p
   * @return CSGSurface::Direction
   */
  virtual CSGSurface::Direction directionFromPoint(const Point p) override;

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
