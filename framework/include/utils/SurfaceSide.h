//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include <ostream>
#include <vector>

/// The side of a closed surface where a query point is located.
enum class SurfaceSide
{
  /// The point lies strictly in the interior of the closed surface.
  INSIDE,
  /// The point lies strictly in the exterior of the closed surface.
  OUTSIDE,
  /// The point lies on the surface itself, within tolerance.
  ON
};

/**
 * Classify a signed level-set value into a SurfaceSide.
 *
 * With the signed-distance convention (inside_is_negative == true) a value below
 * -tolerance is INSIDE, a value within +/-tolerance of zero is ON, and a value above
 * +tolerance is OUTSIDE. Setting inside_is_negative == false flips the sign for a
 * level set that is positive in the interior.
 *
 * @param phi The signed function value at the query point.
 * @param tolerance Half-width, in value space, of the on-surface band around zero.
 * @param inside_is_negative Whether negative values denote the interior.
 */
SurfaceSide signedValueSideness(Real phi, Real tolerance, bool inside_is_negative);

/**
 * Combine two classifications for a union: INSIDE if either is INSIDE, otherwise ON
 * if either is ON, otherwise OUTSIDE (the INSIDE > ON > OUTSIDE precedence).
 */
SurfaceSide unionSideness(SurfaceSide a, SurfaceSide b);

/**
 * Combine per-geometry classifications into the classification against their union.
 *
 * A point is INSIDE the union if it is INSIDE any geometry; otherwise it is ON the
 * union if it is ON any geometry; otherwise it is OUTSIDE. This encodes the
 * INSIDE > ON > OUTSIDE precedence.
 */
SurfaceSide unionSideness(const std::vector<SurfaceSide> & sides);

/// Stream a human-readable name for a SurfaceSide value. Defined for debugging
/// output and so test frameworks (e.g. gtest EXPECT_EQ) report the enumerator
/// name rather than its underlying integer value on failure.
inline std::ostream &
operator<<(std::ostream & os, const SurfaceSide & side)
{
  switch (side)
  {
    case SurfaceSide::INSIDE:
      return os << "INSIDE";
    case SurfaceSide::OUTSIDE:
      return os << "OUTSIDE";
    case SurfaceSide::ON:
      return os << "ON";
  }
  return os;
}
