//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef KDTREE_H
#define KDTREE_H

// Moose includes
#include "MooseMesh.h"

#include "libmesh/nanoflann.hpp"
#include "libmesh/utility.h"

class KDTree
{
public:
  KDTree(std::vector<Point> & master_points, unsigned int max_leaf_size);

  virtual ~KDTree() = default;

  void neighborSearch(Point & query_point,
                      unsigned int patch_size,
                      std::vector<std::size_t> & return_index);

  void neighborSearch(Point & query_point,
                      unsigned int patch_size,
                      std::vector<std::size_t> & return_index,
                      std::vector<Real> & return_dist_sqr);

  /**
   * PointListAdaptor is required to use libMesh Point coordinate type with
   * nanoflann KDTree library. The member functions within the PointListAdaptor
   * are used by nanoflann library.
   */
  template <unsigned int KDDim>
  class PointListAdaptor
  {
  private:
    const std::vector<Point> & _pts;

  public:
    PointListAdaptor(const std::vector<Point> & pts) : _pts(pts) {}

    /**
     * libMesh \p Point coordinate type
     */
    using coord_t = Real;
    /**
     * Must return the number of data points
     */
    inline size_t kdtree_get_point_count() const { return _pts.size(); }

    /**
     * Returns the distance between the vector "p1[0:size-1]"
     * and the data point with index "idx_p2" stored in the class
     */
    inline coord_t kdtree_distance(const coord_t * p1, const size_t idx_p2, size_t /*size*/) const
    {
      mooseAssert(idx_p2 <= _pts.size(),
                  "The point index should be less than"
                  "total number of points used to build"
                  "the KDTree.");

      const Point & p2(_pts[idx_p2]);

      coord_t dist = 0.0;

      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
        dist += Utility::pow<2>(p1[i] - p2(i));

      return dist;
    }

    /**
     * Returns the dim'th component of the idx'th point in the class:
     * Since this is inlined and the "dim" argument is typically an immediate
     * value, the
     *  "if's" are actually solved at compile time.
     */
    inline coord_t kdtree_get_pt(const size_t idx, int dim) const
    {
      mooseAssert(dim < (int)LIBMESH_DIM,
                  "The required component number should be less than the LIBMESH_DIM.");
      mooseAssert(idx < _pts.size(),
                  "The index of the point should be less"
                  "than total number of points used to"
                  "construct the KDTree.");

      const Point & p(_pts[idx]);

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

  using KdTreeT = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<LIBMESH_DIM>>,
      PointListAdaptor<LIBMESH_DIM>,
      LIBMESH_DIM>;

protected:
  PointListAdaptor<LIBMESH_DIM> _point_list_adaptor;
  std::unique_ptr<KdTreeT> _kd_tree;
};

#endif // KDTREE_H
