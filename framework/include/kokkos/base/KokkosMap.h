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

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos wrapper class for standard map.
 * The map can only be populated on host.
 * Make sure to call copy() after populating the map on host.
 */
template <typename T1, typename T2>
class Map
{
public:
  /**
   * Default constructor
   */
  Map() : _map_host(std::make_shared<std::map<T1, T2>>()) {}

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
    mooseAssert(_map_host, "Kokkos map error: host map was not initialized.");

    return *_map_host;
  }
  /**
   * Get the underlying const host map
   * @returns The const host map
   */
  const auto & get() const
  {
    mooseAssert(_map_host, "Kokkos map error: host map was not initialized.");

    return *_map_host;
  }
  /**
   * Copy the host map to device
   */
  void copy();
  /**
   * Get the size of map
   * @returns The size of map
   */
  KOKKOS_FUNCTION dof_id_type size() const
  {
    KOKKOS_IF_ON_HOST(return get().size();)

    return _map_device.size();
  }
  /**
   * Get whether the key exists
   * @param key The key
   * @returns Whether the key exists
   */
  KOKKOS_FUNCTION bool exists(const T1 & key) const { return _map_device.exists(key); }
  /**
   * Get the value corresponding to a key
   * @param key The key
   * @returns The const reference of the value
   */
  KOKKOS_FUNCTION const T2 & operator[](const T1 & key) const
  {
    KOKKOS_IF_ON_HOST(return get().at(key);)

    KOKKOS_ASSERT(exists(key));

    return _map_device.value_at(_map_device.find(key));
  }
  /**
   * Call host map's operator[]
   * @param key The key
   * @returns The writeable reference of the value
   */
  T2 & operator[](const T1 & key) { return get()[key]; }

private:
  /**
   * Standard map on host
   * Stored as a shared pointer to avoid deep copy
   */
  const std::shared_ptr<std::map<T1, T2>> _map_host;
  /**
   * Kokkos map on device
   */
  ::Kokkos::UnorderedMap<T1, T2, ExecSpace> _map_device;
  /**
   * Maximum allowed size
   */
  static constexpr std::size_t _max_size = std::numeric_limits<uint32_t>::max();
};

template <typename T1, typename T2>
void
Map<T1, T2>::copy()
{
  if (size() > _max_size)
    mooseError("Kokkos map error: size cannot exceed ",
               _max_size,
               ", but the current size is ",
               get().size(),
               ".");

  Array<T1> keys(size());
  Array<T2> values(size());

  uint32_t i = 0;

  for (auto & [key, value] : get())
  {
    keys[i] = key;
    values[i++] = value;
  }

  keys.copyToDevice();
  values.copyToDevice();

  _map_device.rehash(size());

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<uint32_t>> policy(0, size());
  ::Kokkos::parallel_for(
      policy, KOKKOS_CLASS_LAMBDA(const uint32_t i) { _map_device.insert(keys[i], values[i]); });
}

} // namespace Kokkos
} // namespace Moose
