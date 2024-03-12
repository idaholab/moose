//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseUtils.h"
#include "DataIO.h"
#include "libmesh/int_range.h"
#include "libmesh/nanoflann.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <tuple>
#include <optional>
#include <exception>

template <typename T>
class ValueCache;
template <typename T>
void dataStore(std::ostream & stream, ValueCache<T> & c, void * context);
template <typename T>
void dataLoad(std::istream & stream, ValueCache<T> & c, void * context);

/**
 * ValueCache is a generic helper template to implement an unstructured data
 * cache, where arbitrary result types can be placed in an n-dimensional space of
 * real numbers. Upon lookup the closest result to a given point in n-space is
 * returned. Applications include caching reasonable initial guesses for local
 * Newton solves as found in certain phase field models as well as thermodynamic
 * Gibbs energy minimization.
 */
template <typename T>
class ValueCache
{
public:
  /// Construct a ValueCache with indices in in_dim dimensions
  ValueCache(std::size_t in_dim, std::size_t max_leaf_size = 10);

  /// Same as above, but provide a filename for storing/restoring the cache content between runs
  ValueCache(const std::string & file_name, std::size_t in_dim, std::size_t max_leaf_size = 10);

  /// Object destructor. If the cache was constructed with a file name, it gets written here.
  ~ValueCache();

  /// insert a new value out_value at the position in_val
  void insert(const std::vector<Real> & in_val, const T & out_val);

  /// get a single neighbor of in_val along with stored values and distances
  std::tuple<const std::vector<Real> &, const T &, Real>
  getNeighbor(const std::vector<Real> & in_val);

  /// get a list of up to k neighbors of in_val along with stored values and distances
  std::vector<std::tuple<const std::vector<Real> &, const T &, Real>>
  getNeighbors(const std::vector<Real> & in_val, const std::size_t k);

  /// return the number of cache entries
  std::size_t size();

  /// remove all data from the cache
  void clear();

  ///@{ Nanoflann interface functions
  std::size_t kdtree_get_point_count() const;
  Real kdtree_get_pt(const std::size_t idx, const std::size_t dim) const;
  template <class BBOX>
  bool kdtree_get_bbox(BBOX & bb) const;
  ///@}

protected:
  /// rebuild the kd-tree from scratch and update the bounding box
  void rebuildTree();

  using KdTreeT =
      nanoflann::KDTreeSingleIndexDynamicAdaptor<nanoflann::L2_Simple_Adaptor<Real, ValueCache<T>>,
                                                 ValueCache<T>,
                                                 -1,
                                                 std::size_t>;

  std::vector<std::pair<std::vector<Real>, T>> _location_data;
  std::unique_ptr<KdTreeT> _kd_tree;

  const std::size_t _in_dim;
  const std::size_t _max_leaf_size;
  const std::size_t _max_subtrees;

  /// file name for persistent store/restore of the cache
  std::optional<std::string> _persistent_storage_file;

  /// bounding box (updated upon insertion)
  std::vector<std::pair<Real, Real>> _bbox;

  friend void dataStore<T>(std::ostream & stream, ValueCache<T> & c, void * context);
  friend void dataLoad<T>(std::istream & stream, ValueCache<T> & c, void * context);
};

template <typename T>
ValueCache<T>::ValueCache(std::size_t in_dim, std::size_t max_leaf_size)
  : _kd_tree(nullptr), _in_dim(in_dim), _max_leaf_size(max_leaf_size), _max_subtrees(100)
{
}

template <typename T>
ValueCache<T>::ValueCache(const std::string & file_name,
                          std::size_t in_dim,
                          std::size_t max_leaf_size)
  : ValueCache(in_dim, max_leaf_size)
{
  _persistent_storage_file = file_name;

  // if the persistent storage file exists and is readable, load it
  if (MooseUtils::checkFileReadable(*_persistent_storage_file,
                                    /*check_line_endings =*/false,
                                    /*throw_on_unreadable =*/false))
  {
    std::ifstream in_file(_persistent_storage_file->c_str());
    if (!in_file)
      mooseError("Failed to open '", *_persistent_storage_file, "' for reading.");
    dataLoad(in_file, *this, nullptr);
  }
}

template <typename T>
ValueCache<T>::~ValueCache()
{
  // if a persistent storage file was specified, write results back to it
  if (_persistent_storage_file.has_value())
  {
    std::ofstream out_file(_persistent_storage_file->c_str());
    if (!out_file)
      mooseWarning("Failed to open '", *_persistent_storage_file, "' for writing.");
    dataStore(out_file, *this, nullptr);
  }
}

template <typename T>
void
ValueCache<T>::insert(const std::vector<Real> & in_val, const T & out_val)
{
  mooseAssert(in_val.size() == _in_dim, "Key dimensions do not match cache dimensions");

  auto id = size();
  _location_data.emplace_back(in_val, out_val);

  // update bounding box
  if (id == 0)
  {
    // first item is inserted
    _bbox.resize(_in_dim);
    for (const auto i : make_range(_in_dim))
      _bbox[i] = {in_val[i], in_val[i]};
  }
  else
    for (const auto i : make_range(_in_dim))
      _bbox[i] = {std::min(_bbox[i].first, in_val[i]), std::max(_bbox[i].second, in_val[i])};

  // do we have too many subtrees?
  if (_kd_tree && _kd_tree->getAllIndices().size() > _max_subtrees)
    _kd_tree = nullptr;

  // rebuild tree or add point
  if (!_kd_tree)
  {
    _kd_tree = std::make_unique<KdTreeT>(_in_dim, *this, _max_leaf_size);
    mooseAssert(_kd_tree != nullptr, "KDTree was not properly initialized.");
  }
  else
    _kd_tree->addPoints(id, id);
}

/**
 * Retrieve a single closest neighbor to in_val. Throws an exception if no values are stored.
 */
template <typename T>
std::tuple<const std::vector<Real> &, const T &, Real>
ValueCache<T>::getNeighbor(const std::vector<Real> & in_val)
{
  // throw an exception if this is called on an empty cache
  if (_location_data.empty())
    throw std::runtime_error("Attempting to retrieve a neighbor from an empty ValueCache.");

  // buffers for the kNN search
  nanoflann::KNNResultSet<Real> result_set(1);
  std::size_t return_index;
  Real distance;

  // kNN search
  result_set.init(&return_index, &distance);
  _kd_tree->findNeighbors(result_set, in_val.data());

  const auto & [location, data] = _location_data[return_index];
  return {std::cref(location), std::cref(data), distance};
}

/**
 * This function performs a search on the value cache and returns either the k-nearest neighbors or
 * the neighbors available if the cache size is less than k.
 */
template <typename T>
std::vector<std::tuple<const std::vector<Real> &, const T &, Real>>
ValueCache<T>::getNeighbors(const std::vector<Real> & in_val, const std::size_t k)
{
  // return early if no points are stored
  if (_location_data.empty())
    return {};

  // buffers for the kNN search
  nanoflann::KNNResultSet<Real> result_set(std::min(k, size()));
  std::vector<std::size_t> return_indices(std::min(k, size()));
  std::vector<Real> distances(std::min(k, size()));

  // kNN search
  result_set.init(return_indices.data(), distances.data());
  _kd_tree->findNeighbors(result_set, in_val.data());

  // prepare results to be returned
  std::vector<std::tuple<const std::vector<Real> &, const T &, Real>> nearest_neighbors;
  for (const auto i : index_range(result_set))
  {
    const auto & [location, data] = _location_data[return_indices[i]];
    nearest_neighbors.emplace_back(std::cref(location), std::cref(data), distances[i]);
  }
  return nearest_neighbors;
}

template <typename T>
std::size_t
ValueCache<T>::size()
{
  return kdtree_get_point_count();
}

template <typename T>
void
ValueCache<T>::clear()
{
  _location_data.clear();
  _kd_tree = nullptr;
}

template <typename T>
void
ValueCache<T>::rebuildTree()
{
  if (_location_data.empty())
    return;

  // reset bounding box (must be done before the tree is built)
  _bbox.resize(_in_dim);
  const auto & location0 = _location_data[0].first;
  for (const auto i : make_range(_in_dim))
    _bbox[i] = {location0[i], location0[i]};

  for (const auto & pair : _location_data)
  {
    const auto & location = pair.first;
    for (const auto i : make_range(_in_dim))
      _bbox[i] = {std::min(_bbox[i].first, location[i]), std::max(_bbox[i].second, location[i])};
  }

  // build kd-tree
  _kd_tree = std::make_unique<KdTreeT>(_in_dim, *this, _max_leaf_size);
  mooseAssert(_kd_tree != nullptr, "KDTree was not properly initialized.");
}

template <typename T>
std::size_t
ValueCache<T>::kdtree_get_point_count() const
{
  return _location_data.size();
}

template <typename T>
Real
ValueCache<T>::kdtree_get_pt(const std::size_t idx, const std::size_t dim) const
{
  return _location_data[idx].first[dim];
}

template <typename T>
template <class BBOX>
bool
ValueCache<T>::kdtree_get_bbox(BBOX & bb) const
{
  if (_location_data.empty())
    return false;

  // return the bounding box incrementally built upon insertion
  for (const auto i : make_range(_in_dim))
    bb[i] = {_bbox[i].first, _bbox[i].second};
  return true;
}

template <typename T>
inline void
dataStore(std::ostream & stream, ValueCache<T> & c, void * context)
{
  storeHelper(stream, c._location_data, context);
}

template <typename T>
inline void
dataLoad(std::istream & stream, ValueCache<T> & c, void * context)
{
  loadHelper(stream, c._location_data, context);
  c.rebuildTree();
}
