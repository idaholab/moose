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
   */
  CSGSphere(const std::string & name, const Point & center, const Real r);

  /**
   * @brief Construct a new CSGSphere surface
   *
   * @param name unique name for the sphere surface
   * @param r radius of sphere
   */
  CSGSphere(const std::string & name, const Real r);

  /**
   * Destructor
   */
  virtual ~CSGSphere() = default;

  /**
   * @brief Get the coefficients (x0, y0, z0, r) for the equation of a sphere
   * (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = r^2
   *
   * @return map of coefficients (x0, y0, z0, and r) and their values
   */
  virtual std::unordered_map<std::string, Real> getCoeffs() const override;

  /**
   * @brief given a point, determine its evaluation based on the equation of the sphere
   *
   * @param p point
   * @return evaluation of point based on surface equation
   */
  virtual Real evaluateSurfaceEquationAtPoint(const Point & p) const override;

protected:
  /**
   * @brief create clone of CSGSphere object
   *
   * @return std::unordered_map<CSGSurface> unique_ptr to cloned sphere
   */
  virtual std::unique_ptr<CSGSurface> clone() const override
  {
    return std::make_unique<CSGSphere>(_name, Point(_x0, _y0, _z0), _r);
  }

  // check that radius is positive
  void checkRadius() const;

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
