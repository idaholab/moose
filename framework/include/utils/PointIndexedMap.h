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
  PointIndexedMap(const Point & mesh_max_coords)
  {
    for (auto i : make_range(LIBMESH_DIM))
      normalization(i) =
          MooseUtils::absoluteFuzzyEqual(mesh_max_coords(i), 0) ? 1 : mesh_max_coords(i);
  }

  /**
   * Normalize, expand then round a point to create local bins
   * - normalize the point by dividing by the meshes max (non-zero) absolute coordinates
   * - multiply the result by 1e12 and round, which keeps a fixed number of digits
   */
  inline Point getRoundedPoint(const Point & p) const
  {
    // We cant support coordinates too far from the normalization point
    mooseAssert(p(0) / normalization(0) < 1 && p(1) / normalization(1) < 1 &&
                    p(2) / normalization(2) < 1,
                "Point coordinates for indexing must be normalized below 1. "
                "Point: "
                    << p << " normalization: " << normalization);

    std::vector<Real> rounded_p{round(p(0) / normalization(0) * pow(10, 12)),
                                round(p(1) / normalization(1) * pow(10, 12)),
                                round(p(2) / normalization(2) * pow(10, 12))};
    return Point(rounded_p[0], rounded_p[1], rounded_p[2]);
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
      Moose::hash_combine(seed, p(0), p(1), p(2));
      return seed;
    }
  };

  /// The container indexed by points
  std::unordered_map<Point, Number, hash_point> base_map;

  /// Normalization factors used to scale points before insertions/comparisons
  Point normalization;

  Number & operator[](Point p)
  {
    auto it = find(p);
    if (it != end())
      return const_cast<Number &>(it->second);
    else
    {
      const auto rp = getRoundedPoint(p);
      return base_map[rp];
    }
  }

  unsigned int hasKey(Point p)
  {
    auto it = find(p);
    if (it != end())
      return true;
    else
      return false;
  }

  std::unordered_map<Point, Number, hash_point>::const_iterator find(const Point & pt) const
  {
    const auto rp = getRoundedPoint(pt);

    auto it = base_map.find(rp);
    if (it != base_map.end())
      return it;
    else
    {
      // Search the point with coordinates rounded down to origin
      auto rounded = base_map.find(rp);
      if (rounded != base_map.end())
        return rounded;

      // To store the output of tentative searches
      std::unordered_map<Point, Number, hash_point>::const_iterator out;

      // Search in all directions around
      for (int i : make_range(2))
        for (int j : make_range(2))
          for (int k : make_range(2))
            if (attempt_find(pt, 2 * i - 1, 2 * j - 1, 2 * k - 1, out))
              return out;

      return base_map.end();
    }
  }

  bool attempt_find(const Point & p,
                    Real dx,
                    Real dy,
                    Real dz,
                    std::unordered_map<Point, Number, hash_point>::const_iterator & out) const
  {
    // Some epsilon to move the point by.
    // There is no multiplicative epsilon that will reliably change the last decimal
    const auto eps = 1e-13;
    const auto shifted_rp = getRoundedPoint(p + Point(dx * eps * normalization(0),
                                                      dy * eps * normalization(1),
                                                      dz * eps * normalization(2)));
    out = base_map.find(shifted_rp);
    if (out != base_map.end())
      return true;
    return false;
  }

  std::unordered_map<Point, Number, hash_point>::const_iterator end() const
  {
    return base_map.end();
  }
};
