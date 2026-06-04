//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

#include "CSGSurfaceEngUnit.h"

namespace CSG
{

class CSGBase;

/**
 * CSGNPolygonUnit is a CSGSurfaceEngUnit that represents a regular N-sided polygon
 * (prismatic region) with its axis aligned with the z-axis.
 *
 * The polygon is defined by N infinite planes, one per side, each oriented so that
 * its inward normal points toward the center. The interior of the polygon is the
 * intersection of the N negative half-spaces of those planes.
 *
 * Implements:
 *   - expandUnit() — creates N CSGPlane surfaces in _internal_base, one per side
 *   - getExpandedRegion() — returns the intersection of the N half-spaces formed by the planes that
 *     containing the origin
 *   - evaluateSurfaceEquationAtPoint() — returns the maximum plane evaluation over
 *     all N sides; negative means the point is inside, positive means outside
 *   - clone() — returns a deep copy
 *   - getAttributes() — returns n_sides, apothem
 */
class CSGNPolygonUnit : public CSGSurfaceEngUnit
{
public:
  /**
   * @brief Construct a new CSGNPolygonUnit.
   *
   * @param name unique name of the unit
   * @param n_sides number of sides of the regular polygon (must be >= 3)
   * @param apothem distance from the center to the midpoint of each side
   */
  CSGNPolygonUnit(const std::string & name, int n_sides, Real apothem);

  /**
   * @brief Evaluate the polygon's surface equation at the given point.
   *
   * Returns the maximum signed plane-equation value over all N sides. A negative
   * return value means the point lies inside the polygon; positive means outside;
   * zero means on one of the sides.
   *
   * This can be evaluated before expansion using the stored geometric parameters.
   *
   * @param p point to evaluate
   * @return maximum signed distance from the point to any side (negative = inside)
   */
  Real evaluateSurfaceEquationAtPoint(const Point & p) const override;

  /**
   * @brief Return the polygon attributes for this object.
   *
   * @return map containing: n_sides (unsigned int), apothem (Real)
   */
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override;

  // helper getter functions to get additional polygon dimensions/attributes

  /**
   * @brief Get the number of sides for the polygon
   *
   * @return number of sides
   */
  int getNumSides() { return _n_sides; }

  /**
   * @brief Get the polygon apothem (center-to-flat distance)
   *
   * @return apothem value
   */
  Real getApothem() { return _apothem; }

  /**
   * @brief Get the polygon side length
   *
   * @return side length
   */
  Real getSideLength() { return 2.0 * _apothem * std::tan(M_PI / _n_sides); }

  /**
   * @brief Get the the circumradius for the polygon (center-to-vertex distance)
   *
   * @return radius value
   */
  Real getRadius() { return _apothem / (std::cos(M_PI / _n_sides)); }

protected:
  /**
   * @brief Return a deep copy of this unit.
   *
   * @return unique_ptr to a new CSGNPolygonUnit with identical parameters
   */
  std::unique_ptr<CSGSurface> clone() const override
  {
    return std::make_unique<CSGNPolygonUnit>(_name, _n_sides, _apothem);
  }

  /**
   * @brief Create N CSGPlane surfaces in _internal_base, one per polygon side.
   *
   * Stores const pointers to the registered surfaces for use in getExpandedRegion().
   */
  void expandUnit() override;

private:
  /// Number of sides of the regular polygon
  const int _n_sides;

  /// Distance from the polygon center to the midpoint of each side
  const Real _apothem;

  /// Pointers to the expanded plane surfaces, populated during expandUnit()
  std::vector<const CSGSurface *> _planes;

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testPolygonUnitExpansion);
  FRIEND_TEST(CSGEngUnitTest, testPolygonUnitClone);
#endif
};

} // namespace CSG
