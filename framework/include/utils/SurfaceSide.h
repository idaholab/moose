//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <ostream>

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
