//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/nanoflann.hpp"
#include "libmesh/utility.h"
#include "libmesh/point.h"

#include <iterator>

template <typename PointObject>
class PointListAdaptor
{
public:
  using Iterator = typename std::vector<PointObject>::const_iterator;

  PointListAdaptor(Iterator begin, Iterator end)
    : _begin(begin), _end(end), _size(std::distance(begin, end))
  {
  }

private:
  /// begin iterator of the underlying point type vector
  const Iterator _begin;

  /// end iterator of the underlying point type vector
  const Iterator _end;

  /// number of elements pointed to
  std::size_t _size;

public:
  /**
   * libMesh \p Point coordinate type
   */
  using coord_t = Real;

  /**
   * Must return the number of data points
   */
  inline size_t kdtree_get_point_count() const { return _size; }

  /**
   * get a Point reference from the PointData object at index idx in the list
   */
  const Point & getPoint(const PointObject & item) const;

  /**
   * Returns the distance between the vector "p1[0:size-1]"
   * and the data point with index "idx_p2" stored in the class
   */
  inline coord_t kdtree_distance(const coord_t * p1, const size_t idx_p2, size_t /*size*/) const
  {
    mooseAssert(idx_p2 < _size,
                "The point index should be less than"
                "total number of points used to build"
                "the KDTree.");

    auto it = _begin;
    std::advance(it, idx_p2);
    const Point & p2 = getPoint(*it);

    coord_t dist = 0.0;

    for (const auto i : make_range(Moose::dim))
      dist += Utility::pow<2>(p1[i] - p2(i));

    return dist;
  }

  /**
   * Returns the dim'th component of the idx'th point in the class
   */
  inline coord_t kdtree_get_pt(const size_t idx, int dim) const
  {
    mooseAssert(dim < (int)Moose::dim,
                "The required component number should be less than the LIBMESH_DIM.");
    mooseAssert(idx < _size,
                "The index of the point should be less"
                "than total number of points used to"
                "construct the KDTree.");

    auto it = _begin;
    std::advance(it, idx);
    const Point & p = getPoint(*it);

    return p(dim);
  }

  /**
   * Optional bounding-box computation. This function is called by the nanoflann library.
   * If the return value is false, the standard bbox computation loop in the nanoflann
   * library is activated.
   */
  template <class BBOX>
  bool kdtree_get_bbox(BBOX & /* bb */) const
  {
    return false;
  }
};

// Specialization for PointListAdaptor<Point> (provide your own for custom types)
template <>
inline const Point &
PointListAdaptor<Point>::getPoint(const Point & item) const
{
  return item;
}
