//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUHeader.h"
#include "GPUArray.h"

#include "MooseApp.h"

namespace Moose
{
namespace Kokkos
{

/**
 * A temporary object returned by the Kokkos memory pool during memory chunk allocation.
 * The instances of this class are expected to be only created by the Kokkos memory pool.
 * This object holds the size and pointer to the memory chunk and allocates and deallocates the
 * memory chunk upon its construction and destruction by interacting with the underlying memory
 * pool. Therefore, the instance of this class returned by the Kokkos memory pool should be alive
 * while the memory chunk is used.
 */
template <typename T>
class MemoryChunk
{
public:
  /**
   * Constructor
   * @param pool The Kokkos memory pool
   * @param size The memory chunk size in the number of bytes
   */
  KOKKOS_FUNCTION MemoryChunk(const ::Kokkos::MemoryPool<MemSpace> & pool, dof_id_type size)
    : _pool(pool), _size(size)
  {
    auto ptr = pool.allocate(size);

    KOKKOS_ASSERT(ptr);

    _ptr = static_cast<T *>(ptr);
  }
  /**
   * Destructor
   */
  KOKKOS_FUNCTION ~MemoryChunk() { _pool.deallocate(_ptr, _size); }
  /**
   * Get the pointer to the chunk
   * @returns The pointer to the chunk
   */
  KOKKOS_FUNCTION T * get() const { return _ptr; }

private:
  /**
   * Reference of the Kokkos memory pool
   */
  const ::Kokkos::MemoryPool<MemSpace> & _pool;
  /**
   * Memory chunk size in the number of bytes
   */
  const dof_id_type _size;
  /**
   * Pointer to the memory chunk
   */
  T * _ptr;
};

/**
 * The Kokkos class that manages memory pool for dynamically-sized temporary arrays in Kokkos
 * parallel functions
 */
class MemoryPool
{
public:
  /**
   * Constructor
   * @param size The memory pool size in the number of bytes
   * @param ways The number of parallel ways
   */
  MemoryPool(dof_id_type size, unsigned int ways);

  /**
   * Allocate a memory chunk
   * The returned object should be alive while the memory chunk is used
   * @param indx The index to select pool
   * @param size The memory chunk size in the number of elements
   * @returns The memory chunk object containing the pointer to the memory chunk
   */
  template <typename T>
  KOKKOS_FUNCTION MemoryChunk<T> allocate(dof_id_type idx, unsigned int size) const
  {
    auto pool = idx % _pools.size();

    return MemoryChunk<T>(_pools[pool], size * sizeof(T));
  }

private:
  /**
   * Kokkos memory pools
   */
  Array<::Kokkos::MemoryPool<MemSpace>> _pools;
};

/**
 * The Kokkos interface that holds the Kokkos memory pool.
 * Copies the latest Kokkos memory pool object to device during parallel dispatch.
 */
class MemoryPoolHolder
{
public:
  /**
   * Constructor
   * @param app The MOOSE app
   */
  MemoryPoolHolder(const MooseApp & app) : _app(app), _pool(_app.getKokkosMemoryPool()) {}
  /**
   * Copy constructor
   */
  MemoryPoolHolder(const MemoryPoolHolder & holder)
    : _app(holder._app), _pool(_app.getKokkosMemoryPool())
  {
  }

protected:
  /**
   * Get the const reference of the Kokkos memory pool
   * @returns The const reference of the Kokkos memory pool
   */
  KOKKOS_FUNCTION const MemoryPool & kokkosMemoryPool() const { return _pool; }

private:
  /**
   * Reference of the MOOSE app
   */
  const MooseApp & _app;
  /**
   * Device copy of the Kokkos memory pool
   */
  MemoryPool _pool;
};

} // namespace Kokkos
} // namespace Moose
