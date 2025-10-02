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

namespace Moose
{
namespace Kokkos
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
  dof_id_type dim[inner] = {0};
  /**
   * Stride of each dimension
   */
  dof_id_type stride[inner] = {0};

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the size of a dimension
   * @param i The dimension index
   * @returns The size of the dimension
   */
  KOKKOS_FUNCTION dof_id_type operator[](unsigned int i) const { return dim[i]; }
#endif
};

#ifdef MOOSE_KOKKOS_SCOPE
/**
 * The base class of the inner array wrapper. This object provides access to an inner array with
 * local multi-dimensional indexing and is created as a temporary object by the jagged array class.
 * @tparam T The data type
 * @tparam inner The inner array dimension size
 */
template <typename T, unsigned int inner>
class JaggedArrayInnerDataBase
{
public:
  /**
   * Constructor
   * @param data The pointer to the inner array data
   * @param dim The inner array dimension information
   */
  KOKKOS_FUNCTION JaggedArrayInnerDataBase(T * data, JaggedArrayInnerDim<inner> dim)
    : _data(data), _dim(dim)
  {
  }

  /**
   * Get the total inner array size
   * @returns The total inner array size
   */
  KOKKOS_FUNCTION dof_id_type size() const { return _dim.stride[inner - 1]; }
  /**
   * Get the size of a dimension of the inner array
   * @param dim The dimension index
   * @returns The size of the dimension of the inner array
   */
  KOKKOS_FUNCTION dof_id_type n(unsigned int dim) const { return _dim[dim]; }
  /**
   * Get an array entry
   * @param i The dimensionless inner array index
   */
  KOKKOS_FUNCTION T & operator[](dof_id_type i) const
  {
    KOKKOS_ASSERT(i < _dim.stride[inner - 1]);

    return _data[i];
  }

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

#define usingKokkosJaggedArrayInnerDataBaseMembers(T, inner)                                       \
private:                                                                                           \
  using JaggedArrayInnerDataBase<T, inner>::_data;                                                 \
  using JaggedArrayInnerDataBase<T, inner>::_dim

/**
 * The specialization of the inner array wrapper class for each dimension.
 */
///{@
template <typename T, unsigned int inner>
class JaggedArrayInnerData;

template <typename T>
class JaggedArrayInnerData<T, 1> : public JaggedArrayInnerDataBase<T, 1>
{
  usingKokkosJaggedArrayInnerDataBaseMembers(T, 1);

public:
  /**
   * Constructor
   * @param data The pointer to the inner array data
   * @param dim The inner array dimension information
   */
  KOKKOS_FUNCTION JaggedArrayInnerData(T * data, JaggedArrayInnerDim<1> dim)
    : JaggedArrayInnerDataBase<T, 1>(data, dim)
  {
  }

  /**
   * Get an array entry
   * @param i0 The first dimension inner array index
   */
  KOKKOS_FUNCTION T & operator()(dof_id_type i0) const
  {
    KOKKOS_ASSERT(i0 < _dim[0]);

    return _data[i0];
  }
};

template <typename T>
class JaggedArrayInnerData<T, 2> : public JaggedArrayInnerDataBase<T, 2>
{
  usingKokkosJaggedArrayInnerDataBaseMembers(T, 2);

public:
  /**
   * Constructor
   * @param data The pointer to the inner array data
   * @param dim The inner array dimension information
   */
  KOKKOS_FUNCTION JaggedArrayInnerData(T * data, JaggedArrayInnerDim<2> dim)
    : JaggedArrayInnerDataBase<T, 2>(data, dim)
  {
  }

  /**
   * Get an array entry
   * @param i0 The first dimension inner array index
   * @param i1 The second dimension inner array index
   */
  KOKKOS_FUNCTION T & operator()(dof_id_type i0, dof_id_type i1) const
  {
    KOKKOS_ASSERT(i0 < _dim[0]);
    KOKKOS_ASSERT(i1 < _dim[1]);

    return _data[i0 + _dim.stride[0] * i1];
  }
};

template <typename T>
class JaggedArrayInnerData<T, 3> : public JaggedArrayInnerDataBase<T, 3>
{
  usingKokkosJaggedArrayInnerDataBaseMembers(T, 3);

public:
  /**
   * Constructor
   * @param data The pointer to the inner array data
   * @param dim The inner array dimension information
   */
  KOKKOS_FUNCTION JaggedArrayInnerData(T * data, JaggedArrayInnerDim<3> dim)
    : JaggedArrayInnerDataBase<T, 3>(data, dim)
  {
  }

  /**
   * Get an array entry
   * @param i0 The first dimension inner array index
   * @param i1 The second dimension inner array index
   * @param i2 The third dimension inner array index
   */
  KOKKOS_FUNCTION T & operator()(dof_id_type i0, dof_id_type i1, dof_id_type i2) const
  {
    KOKKOS_ASSERT(i0 < _dim[0]);
    KOKKOS_ASSERT(i1 < _dim[1]);
    KOKKOS_ASSERT(i2 < _dim[2]);

    return _data[i0 + _dim.stride[0] * i1 + _dim.stride[1] * i2];
  }
};
///@}
#endif

/**
 * The base class for Kokkos jagged arrays.
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
 */
template <typename T, unsigned int inner, unsigned int outer>
class JaggedArrayBase
{
#ifdef MOOSE_KOKKOS_SCOPE
public:
  /**
   * Allocate outer array
   * @param n The vector containing the size of each dimension for the outer array
   */
  void create(const std::vector<dof_id_type> & n);
  /**
   * Reserve inner array for an outer array entry
   * @param index The array containing the index for the outer array
   * @param dimension The array containing the size of each dimension for the inner array
   */
  void reserve(const std::array<dof_id_type, outer> & index,
               const std::array<dof_id_type, inner> & dimension);
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
   * Get whether the array is finalized
   * @returns Whether the array is finalized
   */
  KOKKOS_FUNCTION bool isFinalized() { return _finalized; }
  /**
   * Get the total data array size
   * @returns The total data array size
   */
  KOKKOS_FUNCTION dof_id_type size() const { return _data.size(); }
  /**
   * Get the total outer array size
   * @returns The total outer array size
   */
  KOKKOS_FUNCTION dof_id_type n() const { return _offsets.size(); }
  /**
   * Get the size of a dimension of the outer array
   * @param dim The dimension index
   * @returns The size of the dimension of the outer array
   */
  KOKKOS_FUNCTION dof_id_type n(unsigned int dim) const { return _offsets.n(dim); }
  /**
   * Get an inner array
   * @param i The dimensionless outer array index
   * @returns The inner array wrapper object
   */
  KOKKOS_FUNCTION auto operator[](dof_id_type i) const
  {
    auto data = &_data[_offsets[i]];
    auto dim = _dims[i];

    return JaggedArrayInnerData<T, inner>(data, dim);
  }
#endif

protected:
  /**
   * Sequential data array
   */
  Array<T> _data;
  /**
   * Dimension information of each inner array
   */
  Array<JaggedArrayInnerDim<inner>, outer> _dims;
  /**
   * Starting offset of each inner array into the sequential data array
   */
  Array<dof_id_type, outer> _offsets;
  /**
   * Whether the array was finalized
   */
  bool _finalized;
};

#define usingKokkosJaggedArrayBaseMembers(T, inner, outer)                                         \
private:                                                                                           \
  using JaggedArrayBase<T, inner, outer>::_data;                                                   \
  using JaggedArrayBase<T, inner, outer>::_dims;                                                   \
  using JaggedArrayBase<T, inner, outer>::_offsets

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T, unsigned int inner, unsigned int outer>
void
JaggedArrayBase<T, inner, outer>::create(const std::vector<dof_id_type> & n)
{
  _data.destroy();
  _offsets.create(n);
  _dims.create(n);

  _finalized = false;
}

template <typename T, unsigned int inner, unsigned int outer>
void
JaggedArrayBase<T, inner, outer>::reserve(const std::array<dof_id_type, outer> & index,
                                          const std::array<dof_id_type, inner> & dimension)
{
  mooseAssert(!_finalized, "KokkosJaggedArray already finalized.");

  dof_id_type idx = 0;
  dof_id_type stride = 1;

  for (unsigned int o = 0; o < outer; ++o)
  {
    idx += index[o] * stride;
    stride *= _offsets.n(o);
  }

  stride = 1;

  for (unsigned int i = 0; i < inner; ++i)
  {
    stride *= dimension[i];

    _dims[idx].dim[i] = dimension[i];
    _dims[idx].stride[i] = stride;
  }
}

template <typename T, unsigned int inner, unsigned int outer>
void
JaggedArrayBase<T, inner, outer>::finalize()
{
  mooseAssert(!_finalized, "KokkosJaggedArray already finalized.");

  dof_id_type dim = 1;

  for (dof_id_type o = 0; o < _offsets.size(); ++o)
  {
    dim = 1;

    for (unsigned int i = 0; i < inner; ++i)
      dim *= _dims[o][i];

    _offsets[o] = dim;
  }

  std::exclusive_scan(_offsets.begin(), _offsets.end(), _offsets.begin(), 0);

  _dims.copyToDevice();
  _offsets.copyToDevice();
  _data.create(_offsets.last() + dim);

  _finalized = true;
}
#endif

/**
 * The specialization of the Kokkos jagged array class for each dimension.
 */
///{@
template <typename T, unsigned int inner, unsigned int outer>
class JaggedArray;

template <typename T, unsigned int inner>
class JaggedArray<T, inner, 1> : public JaggedArrayBase<T, inner, 1>
{
  usingKokkosJaggedArrayBaseMembers(T, inner, 1);

public:
  /**
   * Default constructor
   */
  JaggedArray() = default;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Allocate outer array with given dimensions
   * @param n0 The first dimension size for the outer array
   */
  JaggedArray(dof_id_type n0) { create(n0); }
  /**
   * Allocate outer array with given dimensions
   * @param n0 The first dimension size for the outer array
   */
  void create(dof_id_type n0) { JaggedArrayBase<T, inner, 1>::create({n0}); }

  /**
   * Get an inner array
   * @param i0 The first dimension outer array index
   * @returns The inner array wrapper object
   */
  KOKKOS_FUNCTION auto operator()(dof_id_type i0) const
  {
    auto data = &_data[_offsets(i0)];
    auto dim = _dims(i0);

    return JaggedArrayInnerData<T, inner>(data, dim);
  }
#endif
};

template <typename T, unsigned int inner>
class JaggedArray<T, inner, 2> : public JaggedArrayBase<T, inner, 2>
{
  usingKokkosJaggedArrayBaseMembers(T, inner, 2);

public:
  /**
   * Default constructor
   */
  JaggedArray() = default;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Allocate outer array with given dimensions
   * @param n0 The first dimension size for the outer array
   * @param n1 The second dimension size for the outer array
   */
  JaggedArray(dof_id_type n0, dof_id_type n1) { create(n0, n1); }
  /**
   * Allocate outer array with given dimensions
   * @param n0 The first dimension size for the outer array
   * @param n1 The second dimension size for the outer array
   */
  void create(dof_id_type n0, dof_id_type n1) { JaggedArrayBase<T, inner, 2>::create({n0, n1}); }

  /**
   * Get an inner array
   * @param i0 The first dimension outer array index
   * @param i1 The second dimension outer array index
   * @returns The inner array wrapper object
   */
  KOKKOS_FUNCTION auto operator()(dof_id_type i0, dof_id_type i1) const
  {
    auto data = &_data[_offsets(i0, i1)];
    auto dim = _dims(i0, i1);

    return JaggedArrayInnerData<T, inner>(data, dim);
  }
#endif
};

template <typename T, unsigned int inner>
class JaggedArray<T, inner, 3> : public JaggedArrayBase<T, inner, 3>
{
  usingKokkosJaggedArrayBaseMembers(T, inner, 3);

public:
  /**
   * Default constructor
   */
  JaggedArray() = default;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Allocate outer array with given dimensions
   * @param n0 The first dimension size for the outer array
   * @param n1 The second dimension size for the outer array
   * @param n2 The third dimension size for the outer array
   */
  JaggedArray(dof_id_type n0, dof_id_type n1, dof_id_type n2) { create(n0, n1, n2); }
  /**
   * Allocate outer array with given dimensions
   * @param n0 The first dimension size for the outer array
   * @param n1 The second dimension size for the outer array
   * @param n2 The third dimension size for the outer array
   */
  void create(dof_id_type n0, dof_id_type n1, dof_id_type n2)
  {
    JaggedArrayBase<T, inner, 3>::create({n0, n1, n2});
  }

  /**
   * Get an inner array
   * @param i0 The first dimension outer array index
   * @param i1 The second dimension outer array index
   * @param i2 The third dimension outer array index
   * @returns The inner array wrapper object
   */
  KOKKOS_FUNCTION auto operator()(dof_id_type i0, dof_id_type i1, dof_id_type i2) const
  {
    auto data = &_data[_offsets(i0, i1, i2)];
    auto dim = _dims(i0, i1, i2);

    return JaggedArrayInnerData<T, inner>(data, dim);
  }
#endif
};
///@}

} // namespace Kokkos
} // namespace Moose
