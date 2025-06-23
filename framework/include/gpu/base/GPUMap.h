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

template <typename T1, typename T2>
class GPUMap
{
private:
  // Host map
  std::shared_ptr<std::map<T1, T2>> _map;
  // GPU key array
  GPUArray<T1> _keys;
  // GPU value array
  GPUArray<T2> _values;

public:
  // Default constructor
  GPUMap() { _map = std::make_shared<std::map<T1, T2>>(); }
#ifdef MOOSE_GPU_SCOPE
  // Get the beginning iterator
  auto begin() { return _map->begin(); }
  // Get the end iterator
  auto end() { return _map->end(); }
  // Get the underlying map
  auto & get() { return *_map; }
  // Get or insert entry
  T2 & operator[](const T1 & key) { return (*_map)[key]; }
  // Copy map from host to device
  void copy()
  {
    std::vector<T1> keys;
    std::vector<T2> values;

    for (auto & [key, value] : *_map)
    {
      keys.push_back(key);
      values.push_back(value);
    }

    _keys = keys;
    _values = values;
  }
  // Get the size of map
  KOKKOS_FUNCTION auto size() const
  {
    KOKKOS_IF_ON_HOST(return _map->size();)
    KOKKOS_IF_ON_DEVICE(return _keys.size();)
  }
  // Find the index of key
  KOKKOS_FUNCTION uint64_t find(const T1 & key) const
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
  // Get the value with the index returned by find
  KOKKOS_FUNCTION const T2 & operator[](const uint64_t index) const { return _values[index]; }
#endif
};
