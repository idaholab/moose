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

namespace Moose::Kokkos
{

/**
 * A simple object holding the dimension information of an inner array.
 * @tparam inner The inner array dimension size
 */
template <unsigned int inner>
struct JaggedArrayInnerDim
{
  /**
   * Size of each dimension
   */
  unsigned int dim[inner] = {0};
  /**
   * Stride of each dimension
   */
  unsigned int stride[inner + 1] = {0};

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the size of a dimension
   * @param i The dimension index
   * @returns The size of the dimension
   */
  KOKKOS_FUNCTION unsigned int operator[](unsigned int i) const { return dim[i]; }
#endif
};

#ifdef MOOSE_KOKKOS_SCOPE
/**
 * The inner array wrapper class. This object provides access to an inner array with
 * local multi-dimensional indexing and is created as a temporary object by the jagged array class.
 * @tparam T The data type
 * @tparam inner The inner array dimension size
 * @tparam layout The memory layout type
 */
template <typename T, unsigned int inner, LayoutType layout>
class JaggedArrayInnerData
{
public:
  /**
   * Constructor
   * @param data The pointer to the inner array data
   * @param dim The inner array dimension information
   */
  KOKKOS_FUNCTION JaggedArrayInnerData(T * data, JaggedArrayInnerDim<inner> dim)
    : _data(data), _dim(dim)
  {
  }

  /**
   * Get the total inner array size
   * @returns The total inner array size
   */
  KOKKOS_FUNCTION unsigned int size() const { return _dim.stride[inner]; }
  /**
   * Get the size of a dimension of the inner array
   * @param dim The dimension index
   * @returns The size of the dimension of the inner array
   */
  KOKKOS_FUNCTION unsigned int n(unsigned int dim) const { return _dim[dim]; }
  /**
   * Get an array entry
   * @param i The dimensionless inner array index
   */
  KOKKOS_FUNCTION T & operator[](unsigned int i) const
  {
    KOKKOS_ASSERT(i < _dim.stride[inner]);

    return _data[i];
  }
  /**
   * Get an array entry
   * @param i The inner array index of each dimension
   */
  template <typename... indices>
  KOKKOS_FUNCTION T & operator()(indices... i) const;

protected:
  /**
   * Pointer to the inner array data
   */
  T * _data;
  /**
   * Inner array dimension information
   */
  JaggedArrayInnerDim<inner> _dim;
};

template <typename T, unsigned int inner, LayoutType layout>
template <typename... indices>
KOKKOS_FUNCTION T &
JaggedArrayInnerData<T, inner, layout>::operator()(indices... i) const
{
  static_assert((std::is_convertible<indices, unsigned int>::value && ...),
                "All arguments must be convertible to unsigned int");
  static_assert(sizeof...(i) == inner, "Number of arguments should match array dimension");

#ifndef NDEBUG
  {
    unsigned int idx[inner] = {static_cast<unsigned int>(i)...};

    for (unsigned int d = 0; d < sizeof...(i); ++d)
      KOKKOS_ASSERT(idx[d] < _dim[d]);
  }
#endif

  unsigned int idx = 0;
  unsigned int d = 0;

  if constexpr (layout == LayoutType::LEFT)
    (((idx +=
       (d == 0 ? static_cast<unsigned int>(i) : static_cast<unsigned int>(i) * _dim.stride[d])),
      ++d),
     ...);
  else
    (((idx += (d == inner - 1 ? static_cast<unsigned int>(i)
                              : static_cast<unsigned int>(i) * _dim.stride[d])),
      ++d),
     ...);

  return _data[idx];
}
#endif

/**
 * The Kokkos jagged array class.
 * A Kokkos jagged array aids in treating jagged arrays conveniently and efficiently by using a
 * sequential data storage and providing multi-dimensional indexing using internal dope vectors.
 * A jagged array is divided into the inner and outer arrays. The outer array is the regular part of
 * a jagged array. Each entry of the outer array can hold an inner array, whose size can vary with
 * each other.
 * Calling create() will allocate the outer array, and inner arrays should be reserved one-by-one
 * through reserve(). Once the array structure is set, finalize() should be called. A finalized
 * array cannot be restructured unless it is reset completely by calling create() again.
 * The inner array returned by operator() or operator[] using the outer array index is held in a
 * temporary wrapper object. To avoid the overhead of temporary object creation, it is recommended
 * to store the object locally.
 * @tparam T The data type
 * @tparam inner The inner array dimension size
 * @tparam outer The outer array dimension size
 * @tparam index_type The array index type
 * @tparam layout The memory layout type
 */
template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type = dof_id_type,
          LayoutType layout = LayoutType::LEFT>
class JaggedArray
{
public:
  /**
   * Default constructor
   */
  JaggedArray() = default;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   */
  template <typename... size_type>
  JaggedArray(size_type... n)
  {
    create(n...);
  }

  /**
   * Allocate outer array
   * @param n The vector containing the size of each dimension for the outer array
   */
  void create(const std::vector<index_type> & n);
  /**
   * Allocate outer array
   * @param n The size of each dimension for the outer array
   */
  template <typename... size_type>
  void create(size_type... n);
  /**
   * Reserve inner array for an outer array entry
   * @param index The array containing the index for the outer array
   * @param dimension The array containing the size of each dimension for the inner array
   */
  void reserve(const std::array<uint64_t, outer> & index,
               const std::array<uint64_t, inner> & dimension);
  /**
   * Setup array structure
   */
  void finalize();
  /**
   * Copy data from host to device
   */
  void copyToDevice()
  {
    mooseAssert(_finalized, "KokkosJaggedArray not finalized.");

    _data.copyToDevice();
  }
  /**
   * Copy data from device to host
   */
  void copyToHost()
  {
    mooseAssert(_finalized, "KokkosJaggedArray not finalized.");

    _data.copyToHost();
  }
  /**
   * Copy data from host to device and deallocate host
   */
  void moveToDevice()
  {
    mooseAssert(_finalized, "KokkosJaggedArray not finalized.");

    _dims.moveToDevice();
    _offsets.moveToDevice();
    _data.moveToDevice();
  }
  /**
   * Copy data from device to host and deallocate device
   */
  void moveToHost()
  {
    mooseAssert(_finalized, "KokkosJaggedArray not finalized.");

    _dims.moveToHost();
    _offsets.moveToHost();
    _data.moveToHost();
  }
  /**
   * Get the underlying data array
   * @returns The data array
   */
  auto & array() { return _data; }

  /**
   * Get whether the array is finalized
   * @returns Whether the array is finalized
   */
  KOKKOS_FUNCTION bool isFinalized() const { return _finalized; }
  /**
   * Get whether the array was allocated on host
   * @returns Whether the array was allocated on host
   */
  KOKKOS_FUNCTION bool isHostAlloc() const { return _data.isHostAlloc(); }
  /**
   * Get whether the array was allocated on device
   * @returns Whether the array was allocated on device
   */
  KOKKOS_FUNCTION bool isDeviceAlloc() const { return _data.isDeviceAlloc(); }
  /**
   * Get the total data array size
   * @returns The total data array size
   */
  KOKKOS_FUNCTION index_type size() const { return _data.size(); }
  /**
   * Get the total outer array size
   * @returns The total outer array size
   */
  KOKKOS_FUNCTION index_type n() const { return _offsets.size(); }
  /**
   * Get the size of a dimension of the outer array
   * @param dim The dimension index
   * @returns The size of the dimension of the outer array
   */
  KOKKOS_FUNCTION index_type n(unsigned int dim) const { return _offsets.n(dim); }
  /**
   * Get an inner array
   * @param i The dimensionless outer array index
   * @returns The inner array wrapper object
   */
  KOKKOS_FUNCTION auto operator[](index_type i) const;
  /**
   * Get an inner array
   * @param i The index of each dimension
   * @returns The inner array wrapper object
   */
  template <typename... indices>
  KOKKOS_FUNCTION auto operator()(indices... i) const;
#endif

protected:
  /**
   * Sequential data array
   */
  Array1D<T, index_type> _data;
  /**
   * Dimension information of each inner array
   */
  Array<JaggedArrayInnerDim<inner>, outer, index_type> _dims;
  /**
   * Starting offset of each inner array into the sequential data array
   */
  Array<index_type, outer, index_type> _offsets;
  /**
   * Whether the array was finalized
   */
  bool _finalized = false;
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type,
          LayoutType layout>
void
JaggedArray<T, inner, outer, index_type, layout>::create(const std::vector<index_type> & n)
{
  _data.destroy();
  _offsets.create(n);
  _dims.create(n);

  _finalized = false;
}

template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type,
          LayoutType layout>
template <typename... size_type>
void
JaggedArray<T, inner, outer, index_type, layout>::create(size_type... n)
{
  _data.destroy();
  _offsets.create(n...);
  _dims.create(n...);

  _finalized = false;
}

template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type,
          LayoutType layout>
void
JaggedArray<T, inner, outer, index_type, layout>::reserve(
    const std::array<uint64_t, outer> & index, const std::array<uint64_t, inner> & dimension)
{
  mooseAssert(!_finalized, "KokkosJaggedArray already finalized.");

  index_type idx = 0;
  index_type stride = 1;

  for (unsigned int o = 0; o < outer; ++o)
  {
    idx += index[o] * stride;
    stride *= _offsets.n(o);
  }

  for (unsigned int i = 0; i < inner; ++i)
    _dims[idx].dim[i] = dimension[i];

  stride = 1;

  if constexpr (layout == LayoutType::LEFT)
    for (unsigned int i = 0; i < inner; ++i)
    {
      _dims[idx].stride[i] = stride;
      stride *= dimension[i];
    }
  else
    for (int i = inner - 1; i >= 0; --i)
    {
      _dims[idx].stride[i] = stride;
      stride *= dimension[i];
    }

  _dims[idx].stride[inner] = stride;
}

template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type,
          LayoutType layout>
void
JaggedArray<T, inner, outer, index_type, layout>::finalize()
{
  mooseAssert(!_finalized, "KokkosJaggedArray already finalized.");

  index_type stride = 1;

  for (index_type o = 0; o < _offsets.size(); ++o)
  {
    stride = 1;

    for (unsigned int i = 0; i < inner; ++i)
      stride *= _dims[o][i];

    _offsets[o] = stride;
  }

  std::exclusive_scan(_offsets.begin(), _offsets.end(), _offsets.begin(), 0);

  _dims.copyToDevice();
  _offsets.copyToDevice();
  _data.create(_offsets.last() + stride);

  _finalized = true;
}

template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type,
          LayoutType layout>
KOKKOS_FUNCTION auto
JaggedArray<T, inner, outer, index_type, layout>::operator[](index_type i) const
{
  auto data = &_data[_offsets[i]];
  const auto & dim = _dims[i];

  return JaggedArrayInnerData<T, inner, layout>(data, dim);
}

template <typename T,
          unsigned int inner,
          unsigned int outer,
          typename index_type,
          LayoutType layout>
template <typename... indices>
KOKKOS_FUNCTION auto
JaggedArray<T, inner, outer, index_type, layout>::operator()(indices... i) const
{
  auto data = &_data[_offsets(i...)];
  const auto & dim = _dims(i...);

  return JaggedArrayInnerData<T, inner, layout>(data, dim);
}
#endif

} // namespace Moose::Kokkos
