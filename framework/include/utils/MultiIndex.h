//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseError.h"
#include "DataIO.h"
#include <algorithm>

/**
 * Implements a container class for multi-indexed objects
 * with an arbitrary number of indices.
 */
template <class T>
class MultiIndex
{
public:
  /// MultiIndex container iterator
  template <bool is_const = false>
  class const_noconst_iterator;

  ///@{ container related types and categories
  typedef T value_type;
  typedef std::vector<unsigned int> size_type;
  using iterator = const_noconst_iterator<false>;
  using const_iterator = const_noconst_iterator<true>;
  ///@}

  /// construct zero initialized container of a given shape
  MultiIndex(const size_type & shape);

  /// construct container of a given shape initialized from a flat data blob
  MultiIndex(const size_type & shape, const std::vector<T> & data);

  ///@{ element access operators
  T & operator()(const size_type & indices);
  const T & operator()(const size_type & indices) const;
  ///@}

  ///@{ direct data access via bracket operator
  T & operator[](unsigned int j) { return _data[j]; }
  const T & operator[](unsigned int j) const { return _data[j]; }
  T & at(unsigned int j) { return _data.at(j); }
  ///@}

  ///@{ accesses a slice of the multi index object
  MultiIndex<T> slice(unsigned int dimension, unsigned int index) const;
  MultiIndex<T> slice(size_type dimension, size_type index) const;
  ///@}

  /// container size as dim dimensional vector
  const size_type & size() const { return _shape; }

  /// dimension of the container
  unsigned int dim() const { return _shape.size(); }

  /// total number of values stored in the container
  unsigned int nEntries() const { return _nentries; }

  /// get the raw data vector
  std::vector<T> getRawData() const { return _data; }

  /// Resize container. Must keep dimensionality constant.
  void resize(const size_type & shape);

  /// Reshape container arbitrarily and initialize with value
  void assign(const size_type & shape, T value);

  ///@{ access the stride
  unsigned int stride(unsigned int j) const;
  const size_type & stride() const { return _stride; }
  ///@}

  ///@{ Implement loadHelper and storeHelper for easier data (de)serialization
  void dataStore(std::ostream & stream, void * context);
  void dataLoad(std::istream & stream, void * context);
  ///@}

  ///@{ iterators for begin and end of this container
  iterator begin() { return const_noconst_iterator<false>(*this, 0); }
  iterator end() { return const_noconst_iterator<false>(*this, _nentries); }
  const_iterator begin() const { return const_noconst_iterator<true>(*this, 0); }
  const_iterator end() const { return const_noconst_iterator<true>(*this, _nentries); }
  ///@}

  /// compute the flat index for a size_type index
  unsigned int flatIndex(const size_type & indices) const;

protected:
  /// given a flat index computes the vector of indices i0, i1, i2, ...
  void findIndexVector(unsigned int flat_index, size_type & indices) const;

  /// the size along each index
  size_type _shape;

  /// the number of dimensions TODO: get rid of this -> _shape.size()
  unsigned int _dim;

  /// the number of entries TODO: get rid of this -> _data.size()
  unsigned int _nentries;

  /// stride for each index, e.g. if you know {i, j, k} -> flat_index,  {i, j + 1, k} = flat_index + stride[1]
  size_type _stride;

  /// the data unrolled into a vector
  std::vector<T> _data;

private:
  /// build accumulated shape vector for flat index calculation
  void buildAccumulatedShape();

  /// change the container shape and reset meta data
  void reshape(const size_type & shape);
};

/**
 * Nested iterator class for MultiIndex containers
 */
template <class T>
template <bool is_const>
class MultiIndex<T>::const_noconst_iterator
{
public:
  typedef typename std::conditional<is_const, const MultiIndex<T> &, MultiIndex<T> &>::type
      reference_type;

  const_noconst_iterator(reference_type multi_index, unsigned int position)
    : _multi_index(multi_index), _flat_index(position), _shape(multi_index.size())
  {
    _multi_index.findIndexVector(position, _indices);
  }

  // Simple data getters
  unsigned int flatIndex() const { return _flat_index; }
  reference_type getMultiIndexObject() const { return _multi_index; }

  // assignment =
  const_noconst_iterator & operator=(const const_noconst_iterator & other)
  {
    _multi_index = other.getMultiIndexObject();
    _flat_index = other.flatIndex();
    _multi_index.findIndexVector(_flat_index, _indices);
    return *this;
  }

  // Compiler generated copy constructor
  const_noconst_iterator(const const_noconst_iterator &) = default;

  // prefix ++
  const_noconst_iterator & operator++()
  {
    ++_flat_index;
    // increment indices
    for (unsigned int j = 0; j < _indices.size(); ++j)
    {
      _indices[_indices.size() - j - 1] =
          (_indices[_indices.size() - j - 1] + 1) % _shape[_indices.size() - j - 1];
      if (_indices[_indices.size() - j - 1] != 0)
        break;
    }
    return *this;
  }

  // postfix ++
  const_noconst_iterator & operator++(int)
  {
    const_noconst_iterator clone(*this);
    ++_flat_index;
    // increment indices
    for (unsigned int j = 0; j < _indices.size(); ++j)
    {
      _indices[_indices.size() - j - 1] =
          (_indices[_indices.size() - j - 1] + 1) % _shape[_indices.size() - j - 1];
      if (_indices[_indices.size() - j - 1] != 0)
        break;
    }
    return clone;
  }

  // prefix --
  const_noconst_iterator & operator--()
  {
    --_flat_index;
    // decrement indices
    for (unsigned int j = 0; j < _indices.size(); ++j)
    {
      if (_indices[_indices.size() - j - 1] == 0)
        _indices[_indices.size() - j - 1] = _shape[_indices.size() - j - 1] - 1;
      else
      {
        --_indices[_indices.size() - j - 1];
        break;
      }
    }
    return *this;
  }

  // postfix --
  const_noconst_iterator & operator--(int)
  {
    const_noconst_iterator clone(*this);
    --_flat_index;
    // decrement indices
    for (unsigned int j = 0; j < _indices.size(); ++j)
    {
      if (_indices[_indices.size() - j - 1] == 0)
        _indices[_indices.size() - j - 1] = _shape[_indices.size() - j - 1] - 1;
      else
      {
        --_indices[_indices.size() - j - 1];
        break;
      }
    }
    return clone;
  }

  /// to be equal both iterators must hold a reference to the same MultiIndexObject and be at the same _flat_index
  bool operator==(const const_noconst_iterator & other) const
  {
    return _flat_index == other.flatIndex() && &_multi_index == &other.getMultiIndexObject();
  }
  bool operator!=(const const_noconst_iterator & other) const { return !(*this == other); }

  /// dereferencing operator
  std::pair<const size_type &, T &> operator*()
  {
    return std::pair<const size_type &, T &>(_indices, _multi_index._data[_flat_index]);
  }
  std::pair<const size_type &, const T &> operator*() const
  {
    return std::pair<const size_type &, const T &>(_indices, _multi_index._data[_flat_index]);
  }

protected:
  reference_type _multi_index;
  unsigned int _flat_index;
  size_type _shape;
  size_type _indices;
};

template <class T>
MultiIndex<T>::MultiIndex(const size_type & shape)
{
  reshape(shape);
  _data.resize(_nentries);
}

template <class T>
MultiIndex<T>::MultiIndex(const size_type & shape, const std::vector<T> & data)
{
  reshape(shape);

  if (data.size() != _nentries)
    mooseError("shape and data arguments' sizes are inconsistent.");
  _data = data;
}

template <class T>
T &
MultiIndex<T>::operator()(const size_type & indices)
{
  return _data[flatIndex(indices)];
}

template <class T>
const T &
MultiIndex<T>::operator()(const size_type & indices) const
{
  return _data[flatIndex(indices)];
}

template <class T>
MultiIndex<T>
MultiIndex<T>::slice(unsigned int dimension, unsigned int index) const
{
  size_type dim(1, dimension);
  size_type ind(1, index);
  return slice(dim, ind);
}

template <class T>
MultiIndex<T>
MultiIndex<T>::slice(size_type dimension, size_type index) const
{
  // input checks
  if (dimension.size() != index.size())
    mooseError("dimension and index must have the same size.");
  if (dimension.size() > _dim - 1)
    mooseError("The result of slice must be at least of dimension 1.");

#if DEBUG
  for (unsigned int d = 0; d < dimension.size(); ++d)
  {
    if (dimension[d] >= _dim)
      mooseError("dimension is set to ", dimension[d], " which is larger than _dim ", _dim);
    if (index[d] >= _shape[dimension[d]])
      mooseError("index= ",
                 index[d],
                 " at dimension=",
                 dimension[d],
                 " is larger than ",
                 _shape[dimension[d]]);
  }
#endif // DEBUG

  // create a MultiIndex object with new dim = dim - dimension.size()
  size_type new_shape;
  for (unsigned int j = 0; j < _dim; ++j)
    if (std::find(dimension.begin(), dimension.end(), j) == dimension.end())
      new_shape.push_back(_shape[j]);
  MultiIndex<T> multi_index = MultiIndex(new_shape);

  // copy the data
  MultiIndex<T>::iterator it = multi_index.begin();
  for (unsigned int n = 0; n < _nentries; ++n)
  {
    size_type indices;
    findIndexVector(n, indices);
    bool addTo = true;
    for (unsigned int d = 0; d < dimension.size(); ++d)
      if (indices[dimension[d]] != index[d])
      {
        addTo = false;
        break;
      }

    if (addTo)
    {
      (*it).second = _data[n];
      ++it;
    }
  }
  return multi_index;
}

template <class T>
void
MultiIndex<T>::resize(const size_type & shape)
{
  if (shape.size() != _shape.size())
    mooseError("resize cannot change the dimensionality of MultiIndex.");

  // first copy the old shape and data
  size_type old_shape = _shape;
  size_type old_stride = _stride;
  std::vector<T> old_data = _data;

  // reset _shape & recompute meta data
  reshape(shape);

  // fill in _data to all possible indices
  _data.assign(_nentries, T(0));
  for (unsigned int j = 0; j < _nentries; ++j)
  {
    size_type indices;
    findIndexVector(j, indices);

    // check if indices existed in the old version of _data
    bool existed_in_old = true;
    for (unsigned int d = 0; d < _dim; ++d)
      if (indices[d] >= old_shape[d])
      {
        existed_in_old = false;
        break;
      }

    // find the corresponding old_j
    if (existed_in_old)
    {
      unsigned int old_j = 0;
      for (unsigned int d = 0; d < _dim; ++d)
        old_j += indices[d] * old_stride[d];

      // finally set the data entry
      _data[j] = old_data[old_j];
    }
  }
}

template <class T>
unsigned int
MultiIndex<T>::stride(unsigned int j) const
{
  mooseAssert(j < _dim, "Dimension is" << _dim << ", stride(j) called with j = " << j);
  return _stride[j];
}

template <class T>
void
MultiIndex<T>::assign(const size_type & shape, T value)
{
  reshape(shape);
  _data.assign(_nentries, value);
}

template <class T>
void
MultiIndex<T>::dataStore(std::ostream & stream, void * context)
{
  ::dataStore(stream, _shape, context);
  ::dataStore(stream, _data, context);
}

template <class T>
void
MultiIndex<T>::dataLoad(std::istream & stream, void * context)
{
  ::dataLoad(stream, _shape, context);
  ::dataLoad(stream, _data, context);
  _dim = _shape.size();
  _nentries = _data.size();
  buildAccumulatedShape();
}

template <class T>
void
MultiIndex<T>::findIndexVector(unsigned int flat_index, MultiIndex<T>::size_type & indices) const
{
  indices.resize(_dim);
  for (unsigned int d = 0; d < _dim; ++d)
  {
    unsigned int i = flat_index / _stride[d];
    indices[d] = i;
    flat_index -= i * _stride[d];
  }
}

// compute the accumulated shapes as:
// as[0] = I_1 * I_2 ...* I_{M}, as[1] = I_2 * I_3 ...* I_{M} ...
template <class T>
void
MultiIndex<T>::buildAccumulatedShape()
{
  // TODO: simplify - this is needlessly complicated - can be done in a single loop
  _stride.resize(_dim);
  for (unsigned int d = 0; d < _dim; ++d)
  {
    unsigned int k = 1;
    for (unsigned int j = d + 1; j < _dim; ++j)
      k *= _shape[j];
    _stride[d] = k;
  }
}

template <class T>
void
MultiIndex<T>::reshape(const size_type & shape)
{
  if (shape.size() == 0)
  {
    _shape = {};
    _dim = 0;
    _stride = {};
    _nentries = 0;
    return;
  }

  _shape = shape;
  _dim = shape.size();

  _nentries = 1;
  for (unsigned int d = 0; d < _dim; ++d)
    _nentries *= _shape[d];

  buildAccumulatedShape();
}

template <class T>
unsigned int
MultiIndex<T>::flatIndex(const size_type & indices) const
{
  mooseAssert(indices.size() == _dim,
              "Indices vector has wrong size. size=" << indices.size() << " vs. dim=" << _dim);
#if DEBUG
  for (unsigned int j = 0; j < indices.size(); ++j)
    if (indices[j] >= _shape[j])
      mooseError("Indices vector at entry ", j, " is ", indices[j], " vs. shape ", _shape[j]);
#endif // DEBUG

  // implement the index
  // index = i_M + i_{M-1} * I_M + i_{M-1} * I_M * I_{M-1} ...
  unsigned int index = 0;
  for (unsigned int d = 0; d < _dim; ++d)
    index += indices[d] * _stride[d];

  return index;
}

template <class T>
void
dataStore(std::ostream & stream, MultiIndex<T> & mi, void * context)
{
  mi.dataStore(stream, context);
}

template <class T>
void
dataLoad(std::istream & stream, MultiIndex<T> & mi, void * context)
{
  mi.dataLoad(stream, context);
}
