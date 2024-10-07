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
#include "libmesh/point.h"

// Make newer nanoflann API compatible with older nanoflann versions
#if NANOFLANN_VERSION < 0x150
namespace nanoflann
{
typedef SearchParams SearchParameters;

template <typename T, typename U>
using ResultItem = std::pair<T, U>;
}
#endif

KDTree::KDTree(const std::vector<Point> & master_points, unsigned int max_leaf_size)
  : _point_list_adaptor(master_points.begin(), master_points.end()),
    _kd_tree(std::make_unique<KdTreeT>(
        LIBMESH_DIM, _point_list_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size)))
{
  mooseAssert(_kd_tree != nullptr, "KDTree was not properly initialized.");

  _kd_tree->buildIndex();
}

void
KDTree::neighborSearch(const Point & query_point,
                       unsigned int patch_size,
                       std::vector<std::size_t> & return_index)
{
  std::vector<Real> return_dist_sqr(patch_size);
  neighborSearch(query_point, patch_size, return_index, return_dist_sqr);
}

void
KDTree::neighborSearch(const Point & query_point,
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

void
KDTree::radiusSearch(const Point & query_point,
                     Real radius,
                     std::vector<nanoflann::ResultItem<std::size_t, Real>> & indices_dist)
{
  nanoflann::SearchParameters sp;
  _kd_tree->radiusSearch(&query_point(0), radius * radius, indices_dist, sp);
}

std::size_t
KDTree::numberCandidatePoints()
{
  return _point_list_adaptor.kdtree_get_point_count();
}
