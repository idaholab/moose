//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FNSFUtils.h"
#include "MooseError.h"

namespace FNSF
{

const Real R0{4.8};   // Major radius, m
const Real a{1.2};    // Minor radius, m
const Real tau{0.63}; // triangularity
const Real k{2.2};    // elongation
const Real b{a * k};

Point
torus(Real xi)
{
  xi *= M_PI / 180.0;
  Real R = R0 + a * std::cos(xi + tau * std::sin(xi));
  Real z = b * std::sin(xi);
  return {R, 0, z};
}

Point
orthogonal(Real xi)
{
  xi *= M_PI / 180.0;
  return {b * std::cos(xi), 0, a * std::sin(xi + tau * std::sin(xi)) * (1 + tau * std::cos(xi))};
}

std::pair<Real, Real>
find_xi_depth(Real r, Real z)
{
  // Define search parameters.
  constexpr int MAX_ITERS{50};
  constexpr Real TOLERANCE{1e-6};
  const Real SQ_TOL = TOLERANCE * TOLERANCE;

  // Initialize the bounds of the binary search to +/- 90 degrees.
  Real xi_lo = -90;
  Real xi_hi = 90;

  // Search for the xi value that can be extrapolated out to the given (r, z).
  Point rz{r, 0, z};
  for (int i = 0; i < MAX_ITERS; ++i)
  {
    // Get the midpoint xi.
    Real xi_mid = 0.5 * (xi_hi + xi_lo);

    // Get the (r, z) coords of this xi at zero depth (coords that lie on the
    // last closed flux surface) and the orthogonal vector.
    Point rz_lcfs = torus(xi_mid);
    Point orthog = orthogonal(xi_mid);

    // Project the target (r, z) location onto the line that extends outwards
    // from the LCFS at xi = xi_mid.
    Point delta_mid = rz - rz_lcfs;
    Point proj_rz = rz_lcfs + ((delta_mid * orthog) / (orthog * orthog)) * orthog;

    // Get the displacement from the projected point to the target point.
    Point delta_proj = rz - proj_rz;
    Real sq_err = delta_proj.norm_sq();

    if (sq_err < SQ_TOL)
    {
      // The projected point is sufficiently close to the target. Compute the
      // depth and return (xi, depth).
      orthog /= orthog.norm();
      Real depth = delta_mid * orthog;
      return {xi_mid, depth};
    }

    // Use the z-displacement between the projected and target coordinates to
    // shrink the search space.
    if (delta_proj(2) > 0)
    {
      xi_lo = xi_mid;
    }
    else
    {
      xi_hi = xi_mid;
    }
  }

  mooseError("Failed to find extrapolated Miller coordinates for a point");
  return {0, 0};
}

} // namespace FNSF
