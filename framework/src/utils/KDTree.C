//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KDTree.h"
#include "MooseError.h"

#include "libmesh/nanoflann.hpp"

KDTree::KDTree(std::vector<Point> & master_points, unsigned int max_leaf_size)
  : _point_list_adaptor(master_points),
    _kd_tree(libmesh_make_unique<KdTreeT>(
        LIBMESH_DIM, _point_list_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size)))
{
  mooseAssert(_kd_tree != nullptr, "KDTree was not properly initalized.");

  _kd_tree->buildIndex();
}

void
KDTree::neighborSearch(Point & query_point,
                       unsigned int patch_size,
                       std::vector<std::size_t> & return_index)
{
  // The query point has to be converted from a C++ array to a C array because nanoflann library
  // expects C arrays.
  const Real query_pt[] = {query_point(0), query_point(1), query_point(2)};

  return_index.resize(patch_size, std::numeric_limits<std::size_t>::max());
  std::vector<Real> return_dist_sqr(patch_size, std::numeric_limits<Real>::max());

  _kd_tree->knnSearch(&query_pt[0], patch_size, &return_index[0], &return_dist_sqr[0]);

  if (return_dist_sqr[0] == std::numeric_limits<Real>::max() ||
      return_index[0] == std::numeric_limits<std::size_t>::max())
    mooseError("Unable to find closest node!");
}
