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
 *   - expandUnit(CSGBase &) — creates N CSGPlane surfaces in CSGBase, one per side
 *   - getExpandedRegion() — returns the intersection of the N negative half-spaces
 *   - evaluateSurfaceEquationAtPoint() — returns the maximum plane evaluation over
 *     all N sides; negative means the point is inside, positive means outside
 *   - clone() — returns a deep copy
 *   - getAttributes() — returns n_sides, apothem, x_center, y_center, rotation_deg
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
   * @param x0 x-coordinate of the polygon center (default 0.0)
   * @param y0 y-coordinate of the polygon center (default 0.0)
   */
  CSGNPolygonUnit(
      const std::string & name, unsigned int n_sides, Real apothem, Real x0 = 0.0, Real y0 = 0.0);

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
   * @brief Return the engineering attributes of this unit.
   *
   * @return map containing: n_sides (unsigned int), apothem (Real),
   *         x_center (Real), y_center (Real)
   */
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override;

protected:
  /**
   * @brief Return a deep copy of this unit.
   *
   * @return unique_ptr to a new CSGNPolygonUnit with identical parameters
   */
  std::unique_ptr<CSGSurface> clone() const override
  {
    return std::make_unique<CSGNPolygonUnit>(_name, _n_sides, _apothem, _x_center, _y_center);
  }

  /**
   * @brief Create N CSGPlane surfaces in CSGBase, one per polygon side.
   *
   * Stores const pointers to the registered surfaces for use in getExpandedRegion().
   *
   * @param base CSGBase to which the plane surfaces will be added
   */
  void expandUnit(CSGBase & base) override;

  /**
   * @brief Return the intersection of the N negative half-spaces of the expanded planes.
   *
   * Called by CSGBase immediately after expandUnit(). The returned region represents
   * the interior of the polygon.
   *
   * @return CSGRegion representing the polygon interior
   */
  CSGRegion getExpandedRegion() const override;

private:
  /// Number of sides of the regular polygon
  const unsigned int _n_sides;

  /// Distance from the polygon center to the midpoint of each side
  const Real _apothem;

  /// x-coordinate of the polygon center
  const Real _x_center;

  /// y-coordinate of the polygon center
  const Real _y_center;

  /// Pointers to the expanded plane surfaces, populated during expandUnit()
  std::vector<const CSGSurface *> _planes;
};

} // namespace CSG
