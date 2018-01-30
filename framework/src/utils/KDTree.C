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
  std::vector<Real> return_dist_sqr(patch_size);
  neighborSearch(query_point, patch_size, return_index, return_dist_sqr);
}

void
KDTree::neighborSearch(Point & query_point,
                       unsigned int patch_size,
                       std::vector<std::size_t> & return_index,
                       std::vector<Real> & return_dist_sqr)
{
  return_index.resize(patch_size);

  std::size_t n_result =
      _kd_tree->knnSearch(&query_point(0), patch_size, return_index.data(), return_dist_sqr.data());

  if (n_result == 0)
    mooseError("Unable to find closest node!");

  return_index.resize(n_result);
  return_dist_sqr.resize(n_result);
}
