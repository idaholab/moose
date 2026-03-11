//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosHeader.h"
#endif

#include "MooseUtils.h"
#include "Conversion.h"

namespace Moose::Kokkos
{

using ThreadID = dof_id_type;

/**
 * The Kokkos thread object that aids in converting the one-dimensional thread index into
 * multi-dimensional thread indices
 */
template <typename thread_id_type = ThreadID, unsigned int max_dimension = 4>
class Thread
{
  static_assert(std::is_integral_v<thread_id_type>,
                "Kokkos thread index type must be an integral type");
  static_assert(std::is_unsigned_v<thread_id_type>, "Kokkos thread index type must be unsigned");
  static_assert(!std::is_same_v<bool, thread_id_type>, "Kokkos thread index type must not be bool");

public:
  using id_type = thread_id_type;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Set the thread pool size and dimension
   * @param sizes The size of each dimension
   */
  template <typename... size_type>
  void resize(size_type... sizes);
  /**
   * Get the total thread pool size
   * @returns The total thread pool size
   */
  thread_id_type size() const { return _size; }
  /**
   * Get the multi-dimensional thread index of a dimension given a one-dimensional thread index
   * @param tid The one-dimensional thread index
   * @param dim for which the multi-dimensional thread index is to be returned
   * @returns The multi-dimensional thread index of the dimension
   */
  KOKKOS_FUNCTION thread_id_type operator()(thread_id_type tid, unsigned int dim) const
  {
    KOKKOS_ASSERT(dim < _dim);

    return (tid / _strides[dim]) % _dims[dim];
  }
#endif

protected:
  /**
   * Total thread pool size
   */
  thread_id_type _size = 0;
  /**
   * Thread pool dimension
   */
  unsigned int _dim = 0;
  /**
   * Thread pool size of each dimension
   */
  thread_id_type _dims[max_dimension];
  /**
   * Thread pool stride of each dimension
   */
  thread_id_type _strides[max_dimension];
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename thread_id_type, unsigned int max_dimension>
template <typename... size_type>
void
Thread<thread_id_type, max_dimension>::resize(size_type... sizes)
{
  static_assert((std::is_convertible<size_type, thread_id_type>::value && ...),
                "All arguments must be convertible to thread_id_type");
  static_assert(sizeof...(sizes) <= max_dimension, "Number of arguments exceeds maximum dimension");

  std::vector<thread_id_type> dims;
  (dims.push_back(sizes), ...);

  uint64_t size = 1;
  _dim = dims.size();

  for (unsigned int dim = 0; dim < _dim; ++dim)
  {
    _dims[dim] = dims[dim];
    _strides[dim] = dim ? _strides[dim - 1] * dims[dim - 1] : 1;
    size *= dims[dim];
  }

  if (size > std::numeric_limits<thread_id_type>::max())
    mooseError("Kokkos thread error: the dimensions provided (",
               Moose::stringify(dims),
               ") has the total size of ",
               size,
               " which exceeds the limit of ",
               MooseUtils::prettyCppType<thread_id_type>(),
               ".");

  _size = size;
}
#endif

} // namespace Moose::Kokkos
