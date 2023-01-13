//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseUtils.h"
#include "MooseHashing.h"
#include "libmesh/point.h"

/// An unordered map indexed by Point, eg 3 floating point numbers
/// Because floating point rounding errors can affect the hashing, eg the binning of values,
/// we may need to look near a Point/key for where the initial value was stored
struct PointIndexedMap
{
  /**
   * Get the nearest point with all coordinates of the form m * 10^n such that:
   * - m * 1e12 is a signed integer
   * - 1e12 =< |m * 1e12| < 1e13
   * - n an integer
   */
  static inline Point getMantissaRoundedPoint(const Point & p)
  {
    // - get the scientific notation for the number
    // - use round on the mantissa * 1e12
    // - if rounding to the next power of 10, adjust the exponent
    // - form a new point using the scientific notation
    std::vector<int> exponents{
        MooseUtils::absoluteFuzzyEqual(p(0), 0, 1e-15) ? 0 : (int)log10(fabs(p(0))),
        MooseUtils::absoluteFuzzyEqual(p(1), 0, 1e-15) ? 0 : (int)log10(fabs(p(1))),
        MooseUtils::absoluteFuzzyEqual(p(2), 0, 1e-15) ? 0 : (int)log10(fabs(p(2)))};
    std::vector<Real> rounded_m{round(p(0) / pow(10, exponents[0] - 12)),
                                round(p(1) / pow(10, exponents[1] - 12)),
                                round(p(2) / pow(10, exponents[2] - 12))};
    for (auto i : make_range(LIBMESH_DIM))
    {
      if (fabs(rounded_m[i]) == 1e13)
      {
        exponents[i] += 1;
        rounded_m[i] = (rounded_m[i] > 0) ? 1e12 : -1e12;
      }
    }
    return Point(rounded_m[0] * pow(10, exponents[0] - 12),
                 rounded_m[1] * pow(10, exponents[1] - 12),
                 rounded_m[2] * pow(10, exponents[2] - 12));
  }

  // This needs to be moved into libMesh according to Fande
  // The hash helps sort the points in buckets
  // Hashing the floats also leads to floating point comparison errors in their own way
  // We hash a rounding of the float instead
  struct hash_point
  {
    std::size_t operator()(const Point & p) const
    {
      std::size_t seed = 0;
      const auto rp = getMantissaRoundedPoint(p);
      Moose::hash_combine(seed, rp(0), rp(1), rp(2));
      return seed;
    }
  };

  // We cannot compare Points exactly, there will be floating point errors from
  // operations like adjusting the position or coordinate transformations
  // We do the next best thing and compare with a tolerance: 12 decimal digits in the
  // scientific representation of the coordinates shall be the same
  struct compare_point : public std::binary_function<Point, Point, bool>
  {
    bool operator()(const Point & p1, const Point & p2) const
    {
      const auto rp1 = getMantissaRoundedPoint(p1);
      const auto rp2 = getMantissaRoundedPoint(p2);
      return (rp1 == rp2);
    }
  };

  /// The container indexed by points
  std::unordered_map<Point, Number, hash_point, compare_point> base_map;

  Number & operator[](Point p)
  {
    auto it = find(p);
    if (it != end())
      return const_cast<Number &>(it->second);
    else
      return base_map[p];
  }

  unsigned int hasKey(Point p)
  {
    auto it = find(p);
    if (it != end())
      return true;
    else
      return false;
  }

  std::unordered_map<Point, Number, hash_point, compare_point>::const_iterator
  find(const Point & pt) const
  {
    auto it = base_map.find(pt);
    if (it != base_map.end())
      return it;
    else
    {
      // Rounding errors could prevent the hashing function from returning the right bin
      Point p = pt;

      // If searching while being slightly off 0 due to a round off error, 12th digit numerical
      // precision on the error becomes a ridiculously small tolerance for finding the bin at 0
      auto min_size = 1e-12;
      if (MooseUtils::absoluteFuzzyEqual(p(0), 0, min_size))
        p(0) = 0;
      if (MooseUtils::absoluteFuzzyEqual(p(1), 0, min_size))
        p(1) = 0;
      if (MooseUtils::absoluteFuzzyEqual(p(2), 0, min_size))
        p(2) = 0;

      // Search the point with coordinates rounded down to origin
      auto rounded = base_map.find(p);
      if (rounded != base_map.end())
        return rounded;

      // To store the output of tentative searches
      std::unordered_map<Point, Number, hash_point, compare_point>::const_iterator out;

      // Search in all directions around
      for (int i : make_range(2))
        for (int j : make_range(2))
          for (int k : make_range(2))
            if (attempt_find(p, 2 * i - 1, 2 * j - 1, 2 * k - 1, out))
              return out;

      return base_map.end();
    }
  }

  bool attempt_find(
      const Point & p,
      Real dx,
      Real dy,
      Real dz,
      std::unordered_map<Point, Number, hash_point, compare_point>::const_iterator & out) const
  {
    // Some epsilon to move the point by.
    // There is no multiplicative epsilon that will reliably change the last decimal
    const auto eps = 3e-13 - 5e-20;
    out = base_map.find(
        p + Point(std::abs(p(0)) * dx * eps, std::abs(p(1)) * dy * eps, std::abs(p(2)) * dz * eps));
    if (out != base_map.end())
      return true;
    return false;
  }

  std::unordered_map<Point, Number, hash_point, compare_point>::const_iterator end() const
  {
    return base_map.end();
  }
};
