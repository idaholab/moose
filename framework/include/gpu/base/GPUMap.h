//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUArray.h"

#include <memory>
#include <map>

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos wrapper class for standard map.
 * The map can only be populated on host.
 * Make sure to call copy() after populating the map on host.
 * The lookup on device is done in two steps: get the index with the key using find() and get the
 * value using operator[] with the index.
 * Uses simple binary search on device, so be cautious about the potential performance impact.
 */
template <typename T1, typename T2>
class Map
{
public:
  /**
   * Default constructor
   */
  Map() { _map = std::make_shared<std::map<T1, T2>>(); }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the beginning iterator of the host map
   * @returns The beginning iterator
   */
  auto begin() { return _map->begin(); }
  /**
   * Get the end iterator of the host map
   * @returns The end iterator
   */
  auto end() { return _map->end(); }
  /**
   * Get the underlying host map
   * @returns The host map
   */
  auto & get() { return *_map; }
  /**
   * Call the host map's operator[]
   */
  T2 & operator[](const T1 & key) { return (*_map)[key]; }
  /**
   * Copy the host map to device
   */
  void copy();
  /**
   * Get the size of map
   * @returns The size of map
   */
  KOKKOS_FUNCTION auto size() const
  {
    KOKKOS_IF_ON_HOST(return _map->size();)

    return _keys.size();
  }
  /**
   * Find the index of a key
   * @param key The key
   * @returns The index into the map, size of the map if key is not found
   */
  KOKKOS_FUNCTION uint64_t find(const T1 & key) const;
  /**
   * Get the value corresponding to an index
   * @param index The index into the map returned by find()
   * @returns The reference of the value
   */
  KOKKOS_FUNCTION const T2 & operator[](const uint64_t index) const
  {
    KOKKOS_ASSERT(index < size());

    return _values[index];
  }
#endif

private:
  /**
   * Standard map on host
   * Stored as a shared pointer to avoid deep copy
   */
  std::shared_ptr<std::map<T1, T2>> _map;
  /**
   * Key array on device
   */
  Array<T1> _keys;
  /**
   * Value array on device
   */
  Array<T2> _values;
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T1, typename T2>
void
Map<T1, T2>::copy()
{
  std::vector<T1> keys;
  std::vector<T2> values;

  keys.reserve(_map->size());
  values.reserve(_map->size());

  for (auto & [key, value] : *_map)
  {
    keys.push_back(key);
    values.push_back(value);
  }

  _keys = keys;
  _values = values;
}

template <typename T1, typename T2>
KOKKOS_FUNCTION uint64_t
Map<T1, T2>::find(const T1 & key) const
{
  auto left = &_keys.first();
  auto right = &_keys.last();

  while (left <= right)
  {
    auto mid = left + (right - left) / 2;

    if (*mid == key)
      return mid - left;
    else if (*mid < key)
      left = mid + 1;
    else
      right = mid - 1;
  }

  return _keys.size();
}
#endif

} // namespace Kokkos
} // namespace Moose
