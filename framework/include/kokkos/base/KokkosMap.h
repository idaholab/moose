//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"

#include <memory>
#include <map>

namespace Moose::Kokkos
{

#ifdef MOOSE_KOKKOS_SCOPE
constexpr uint32_t FNV_PRIME = 0x01000193;        // 16777619
constexpr uint32_t FNV_OFFSET_BASIS = 0x811C9DC5; // 2166136261

template <typename T>
KOKKOS_FUNCTION uint32_t
fnv1aHash(const T & key, uint32_t hash)
{
  auto bytes = reinterpret_cast<const uint8_t *>(&key);

  for (size_t i = 0; i < sizeof(T); ++i)
  {
    hash ^= bytes[i];
    hash *= FNV_PRIME;
  }

  return hash;
}

template <typename T>
KOKKOS_FUNCTION uint32_t
fnv1aHash(const T & key)
{
  return fnv1aHash(key, FNV_OFFSET_BASIS);
}

template <typename T1, typename T2>
struct Pair;

template <typename T1, typename T2>
KOKKOS_FUNCTION uint32_t
fnv1aHash(const Pair<T1, T2> & key)
{
  return fnv1aHash(key.second, fnv1aHash(key.first));
}
#endif

template <typename T1, typename T2>
class Map;

template <typename T1, typename T2, typename Context>
void dataStore(std::ostream & stream, Map<T1, T2> & map, Context context);
template <typename T1, typename T2, typename Context>
void dataLoad(std::istream & stream, Map<T1, T2> & map, Context context);

/**
 * The Kokkos wrapper class for standard map.
 * The map can only be populated on host.
 * Make sure to call copyToDevice() or copyToDeviceNested() after populating the map on host.
 */
template <typename T1, typename T2>
class Map
{
public:
  /**
   * Get the beginning writeable iterator of the host map
   * @returns The beginning iterator
   */
  auto begin() { return get().begin(); }
  /**
   * Get the beginning const iterator of the host map
   * @returns The beginning iterator
   */
  auto begin() const { return get().cbegin(); }
  /**
   * Get the end writeable iterator of the host map
   * @returns The end iterator
   */
  auto end() { return get().end(); }
  /**
   * Get the end const iterator of the host map
   * @returns The end iterator
   */
  auto end() const { return get().cend(); }
  /**
   * Get the underlying writeable host map
   * @returns The writeable host map
   */
  auto & get()
  {
    if (!_map_host)
      _map_host = std::make_shared<std::map<T1, T2>>();

    return *_map_host;
  }
  /**
   * Get the underlying const host map
   * @returns The const host map
   */
  const auto & get() const
  {
    if (!_map_host)
      _map_host = std::make_shared<std::map<T1, T2>>();

    return *_map_host;
  }
  /**
   * Clear the underlying data
   */
  void clear();
  /**
   * Call host map's operator[]
   * @param key The key
   * @returns The writeable reference of the value
   */
  T2 & operator[](const T1 & key) { return get()[key]; }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Copy host map to device
   */
  void copyToDevice();
  /**
   * Copy host map to device, perform nested copy for Kokkos arrays
   */
  void copyToDeviceNested();
  /**
   * Swap with another Kokkos map
   * @param map The Kokkos map to be swapped
   */
  void swap(Map<T1, T2> & map);

  /**
   * Get the size of map
   * @returns The size of map
   */
  KOKKOS_FUNCTION dof_id_type size() const
  {
    KOKKOS_IF_ON_HOST(return get().size();)

    return _keys.size();
  }
  /**
   * Find the index of a key
   * @param key The key
   * @returns The index of the key, invalid_id if the key does not exist
   */
  KOKKOS_FUNCTION dof_id_type find(const T1 & key) const;
  /**
   * Get whether a key exists
   * @param key The key
   * @returns Whether the key exists
   */
  KOKKOS_FUNCTION bool exists(const T1 & key) const { return find(key) != invalid_id; }
  /**
   * Get the key of an index
   * @param idx The index returned by find()
   * @returns The const reference of the key
   */
  KOKKOS_FUNCTION const T1 & key(dof_id_type idx) const
  {
    KOKKOS_ASSERT(idx != invalid_id);

    return _keys[idx];
  }
  /**
   * Get the value of an index
   * @param idx The index returned by find()
   * @returns The const reference of the value
   */
  KOKKOS_FUNCTION const T2 & value(dof_id_type idx) const
  {
    KOKKOS_ASSERT(idx != invalid_id);

    return _values[idx];
  }
  /**
   * Get the value corresponding to a key
   * @param key The key
   * @returns The const reference of the value
   */
  ///@{
  KOKKOS_FUNCTION const T2 & operator[](const T1 & key) const
  {
    KOKKOS_IF_ON_HOST(return get().at(key);)

    auto idx = find(key);

    KOKKOS_ASSERT(idx != invalid_id);

    return _values[idx];
  }
  // Due to a stupid NVCC compiler bug, one cannot do var[i][j] for a variable whose type is
  // Array<Map<...>> (the second operator[] seems to be interpreted as if it is the operator of
  // Array), while var[i](j) works. Until we figure out what is going on, one can use the following
  // operator as a workaround.
  KOKKOS_FUNCTION const T2 & operator()(const T1 & key) const { return operator[](key); }
  ///@}
#endif

  static const dof_id_type invalid_id = libMesh::DofObject::invalid_id;

private:
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Internal method to copy host map to device
   */
  void copy();
#endif

  /**
   * Standard map on host
   * Stored as a shared pointer to avoid deep copy
   */
  mutable std::shared_ptr<std::map<T1, T2>> _map_host;
  /**
   * Keys on device
   */
  Array<T1> _keys;
  /**
   * Values on device
   */
  Array<T2> _values;
  /**
   * Beginning offset into device arrays of each bucket
   */
  Array<dof_id_type> _offset;

  template <typename Context>
  friend void dataStore(std::ostream &, Map<T1, T2> &, Context);
  template <typename Context>
  friend void dataLoad(std::istream &, Map<T1, T2> &, Context);
};

template <typename T1, typename T2>
void
Map<T1, T2>::clear()
{
  get().clear();

  _keys.destroy();
  _values.destroy();
  _offset.destroy();
}

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T1, typename T2>
void
Map<T1, T2>::copy()
{
  _keys.create(size());
  _values.create(size());
  _offset.create(size() + 1);
  _offset = 0;

  for (const auto & [key, value] : get())
  {
    auto bucket = fnv1aHash(key) % size();

    _offset[bucket]++;
  }

  std::exclusive_scan(_offset.begin(), _offset.end(), _offset.begin(), 0);

  _offset.copyToDevice();

  std::vector<dof_id_type> idx(size(), 0);

  for (const auto & [key, value] : get())
  {
    auto bucket = fnv1aHash(key) % size();

    _keys[_offset[bucket] + idx[bucket]] = key;
    _values[_offset[bucket] + idx[bucket]] = value;
    idx[bucket]++;
  }
}

template <typename T1, typename T2>
void
Map<T1, T2>::copyToDevice()
{
  copy();

  _keys.copyToDevice();
  _values.copyToDevice();
}

template <typename T1, typename T2>
void
Map<T1, T2>::copyToDeviceNested()
{
  copy();

  _keys.copyToDeviceNested();
  _values.copyToDeviceNested();
}

template <typename T1, typename T2>
void
Map<T1, T2>::swap(Map<T1, T2> & map)
{
  get().swap(map.get());
  _keys.swap(map._keys);
  _values.swap(map._values);
  _offset.swap(map._offset);
}

template <typename T1, typename T2>
KOKKOS_FUNCTION dof_id_type
Map<T1, T2>::find(const T1 & key) const
{
  if (!size())
    return invalid_id;

  auto bucket = fnv1aHash(key) % size();
  auto begin = _offset[bucket];
  auto end = _offset[bucket + 1];

  for (dof_id_type i = begin; i < end; ++i)
    if (_keys[i] == key)
      return i;

  return invalid_id;
}

template <typename T1, typename T2, typename Context>
void
dataStore(std::ostream & stream, Map<T1, T2> & map, Context context)
{
  using ::dataStore;

  dataStore(stream, map.get(), context);
  dataStore(stream, map._keys, context);
  dataStore(stream, map._values, context);
  dataStore(stream, map._offset, context);
}

template <typename T1, typename T2, typename Context>
void
dataLoad(std::istream & stream, Map<T1, T2> & map, Context context)
{
  using ::dataLoad;

  dataLoad(stream, map.get(), context);
  dataLoad(stream, map._keys, context);
  dataLoad(stream, map._values, context);
  dataLoad(stream, map._offset, context);
}
#endif

} // namespace Moose::Kokkos
