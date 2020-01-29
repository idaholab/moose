//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiDimensionalInterpolation.h"

#include "DualRealOps.h"

#include <cassert>
#include <fstream>
#include <stdexcept>

template <typename T>
MultiDimensionalInterpolationTempl<T>::MultiDimensionalInterpolationTempl(
    const std::vector<std::vector<Real>> & base_points, const MultiIndex<Real> & data)
  : _base_points(base_points), _data(data)
{
  errorCheck();
}

template <typename T>
MultiDimensionalInterpolationTempl<T>::MultiDimensionalInterpolationTempl()
  : _base_points(std::vector<std::vector<Real>>()), _data(MultiIndex<Real>({}))
{
}

template <typename T>
void
MultiDimensionalInterpolationTempl<T>::errorCheck()
{
  if (_base_points.size() != _data.dim())
    throw std::domain_error("Size of the leading dimension of base points must be equal to "
                            "dimensionality of data array");

  MultiIndex<Real>::size_type shape = _data.size();
  for (unsigned int j = 0; j < _data.dim(); ++j)
  {
    if (shape[j] != _base_points[j].size())
    {
      std::ostringstream oss;
      oss << "Dimension " << j << " has " << _base_points[j].size()
          << " base points but dimension of data array is " << shape[j];
      throw std::domain_error(oss.str());
    }

    for (unsigned int i = 1; i < _base_points[j].size(); ++i)
    {
      if (_base_points[j][i - 1] >= _base_points[j][i])
      {
        std::ostringstream oss;
        oss << "Base point values for dimension " << j << " are not strictly increasing: bp["
            << i - 1 << "]: " << _base_points[j][i - 1] << " bc[" << i
            << "]: " << _base_points[j][i];
        throw std::domain_error(oss.str());
      }
    }
  }
}

template <typename T>
void
MultiDimensionalInterpolationTempl<T>::linearSearch(std::vector<T> & values,
                                                    MultiIndex<Real>::size_type & indices) const
{
  assert(values.size() == _data.dim());

  indices.resize(_data.dim());
  for (unsigned int d = 0; d < _data.dim(); ++d)
    indices[d] = linearSearchHelper(values[d], _base_points[d]);
}

template <typename T>
unsigned int
MultiDimensionalInterpolationTempl<T>::linearSearchHelper(T & x,
                                                          const std::vector<Real> & vector) const
{
// check order of vector only in debug
#ifndef NDEBUG
  for (unsigned int i = 1; i < vector.size(); ++i)
  {
    if (vector[i - 1] >= vector[i])
    {
      std::ostringstream oss;
      oss << "In linearSearchHelper vector is not strictly increasing: vector[" << i - 1
          << "]: " << vector[i - 1] << " vector[" << i << "]: " << vector[i];
      throw std::domain_error(oss.str());
    }
  }
#endif

  // smaller/larger cases for tabulation
  if (x < vector[0])
  {
    x = vector[0];
    return 0;
  }
  else if (x >= vector.back())
  {
    x = vector.back();
    return vector.size() - 1;
  }

  for (unsigned int j = 0; j < vector.size() - 2; ++j)
    if (x >= vector[j] && x < vector[j + 1])
      return j;
  return vector.size() - 2;
}

template <typename T>
T
MultiDimensionalInterpolationTempl<T>::multiLinearInterpolation(const std::vector<T> & x) const
{
  // assert does not seem to be enough to check data consistency here
  // because this function is exposed to user and will be frequently used
  if (x.size() != _data.dim())
  {
    std::ostringstream oss;
    oss << "In sample the parameter x has size " << x.size() << " but data has dimension "
        << _data.dim();
    throw std::domain_error(oss.str());
  }

  // make a copy of x because linearSearch needs to be able to modify it
  std::vector<T> y = x;

  // obtain the indices using linearSearch & get flat index in _data array
  MultiIndex<Real>::size_type indices;
  linearSearch(y, indices);

  // pre-compute the volume of the hypercube and weights used for the interpolation
  Real volume = 1;
  std::vector<std::vector<T>> weights(_data.dim());
  for (unsigned int j = 0; j < _data.dim(); ++j)
  {
    volume *= _base_points[j][indices[j] + 1] - _base_points[j][indices[j]];

    // now compute weight for each dimension, note that
    // weight[j][0] = bp[j][high] - x[j]
    // weight[j][0] = x[j] - bp[j][low]
    // because in the interpolation the value on the "left" is weighted with the
    // distance of x to the "right"
    weights[j].resize(2);
    weights[j][0] = _base_points[j][indices[j] + 1] - x[j];
    weights[j][1] = x[j] - _base_points[j][indices[j]];
  }

  // get flat index and stride
  unsigned int flat_index = _data.flatIndex(indices);
  MultiIndex<Real>::size_type stride = _data.stride();

  // this is the interpolation routine, evaluation can be expensive if dim is large

  // first we retrieve the data around the point y
  // we use a 2^dim MultiIndex hypercube for generating the permulations
  MultiIndex<Real>::size_type shape(_data.dim(), 2);
  MultiIndex<Real> hypercube = MultiIndex<Real>(shape);
  T interpolated_value = 0;
  for (const auto & p : hypercube)
  {
    // this loop computes the flat index for the point in the
    // hypercube and accumulates the total weight for this point
    // the selection of the dimensional weight is based on the swapping
    // done in the pre-computing loop before
    unsigned int index = flat_index;
    T w = 1;
    for (unsigned int j = 0; j < p.first.size(); ++j)
    {
      index += stride[j] * p.first[j];
      w *= weights[j][p.first[j]];
    }

    // add the contribution of this base point to the interpolated value
    interpolated_value += w * _data[index];
  }

  return interpolated_value / volume;
}

template class MultiDimensionalInterpolationTempl<Real>;
template class MultiDimensionalInterpolationTempl<DualReal>;
