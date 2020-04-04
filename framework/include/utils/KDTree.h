//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "MooseMesh.h"
#include "PointListAdaptor.h"

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

  void radiusSearch(Point & query_point,
                    Real radius,
                    std::vector<std::pair<std::size_t, Real>> & indices_dist);

  using KdTreeT = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<Point>>,
      PointListAdaptor<Point>,
      LIBMESH_DIM>;

protected:
  PointListAdaptor<Point> _point_list_adaptor;
  std::unique_ptr<KdTreeT> _kd_tree;
};
