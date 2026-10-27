//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceSide.h"

#include <cmath>

namespace SurfaceGeometry
{

SurfaceSide
signedValueSideness(Real phi, const Real tolerance, const bool inside_is_negative)
{
  // Normalize so that negative always denotes the interior.
  if (!inside_is_negative)
    phi = -phi;

  if (phi < -tolerance)
    return SurfaceSide::INSIDE;
  if (std::abs(phi) <= tolerance)
    return SurfaceSide::ON;
  return SurfaceSide::OUTSIDE;
}

// This two-argument form holds the union precedence. Callers fold their per-geometry
// queries through it one at a time, which lets them stop as soon as the result reaches
// INSIDE (the maximal value), skipping further (possibly expensive) queries once union
// membership is decided.
SurfaceSide
unionSideness(const SurfaceSide a, const SurfaceSide b)
{
  // Precedence: INSIDE beats ON beats OUTSIDE.
  if (a == SurfaceSide::INSIDE || b == SurfaceSide::INSIDE)
    return SurfaceSide::INSIDE;
  if (a == SurfaceSide::ON || b == SurfaceSide::ON)
    return SurfaceSide::ON;
  return SurfaceSide::OUTSIDE;
}

} // namespace SurfaceGeometry
