//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RankThreeTensor.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MooseException.h"
#include "MooseUtils.h"
#include "MatrixTools.h"
#include "PermutationTensor.h"

#include "libmesh/utility.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

// C++ includes
#include <iomanip>
#include <ostream>

namespace MathUtils
{
template <>
void mooseSetToZero<RankThreeTensor>(RankThreeTensor & v);
template <>
void mooseSetToZero<ADRankThreeTensor>(ADRankThreeTensor & v);
}

template <typename T>
MooseEnum
RankThreeTensorTempl<T>::fillMethodEnum()
{
  return MooseEnum("general plane_normal");
}

template <typename T>
RankThreeTensorTempl<T>::RankThreeTensorTempl()
{
  mooseAssert(N == 3, "RankThreeTensor is currently only tested for 3 dimensions.");

  for (auto i : make_range(N3))
    _vals[i] = 0;
}

template <typename T>
RankThreeTensorTempl<T>::RankThreeTensorTempl(const InitMethod init)
{
  switch (init)
  {
    case initNone:
      break;

    default:
      mooseError("Unknown RankThreeTensor initialization pattern.");
  }
}

template <typename T>
RankThreeTensorTempl<T>::RankThreeTensorTempl(const std::vector<T> & input, FillMethod fill_method)
{
  fillFromInputVector(input, fill_method);
}

template <typename T>
void
RankThreeTensorTempl<T>::zero()
{
  for (auto i : make_range(N3))
    _vals[i] = 0;
}

template <typename T>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator=(const T & value)
{
  for (auto i : make_range(N3))
    _vals[i] = value;
  return *this;
}

template <typename T>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator=(const RankThreeTensorTempl<T> & a)
{
  for (auto i : make_range(N3))
    _vals[i] = a._vals[i];

  return *this;
}

template <typename T>
libMesh::VectorValue<T>
RankThreeTensorTempl<T>::operator*(const RankTwoTensorTempl<T> & a) const
{
  libMesh::VectorValue<T> result;

  for (auto i : make_range(N))
  {
    T sum = 0;
    unsigned int i1 = i * N2;
    for (unsigned int j1 = 0; j1 < N2; j1 += N)
      for (auto k : make_range(N))
        sum += _vals[i1 + j1 + k] * a._coords[j1 + k];
    result(i) = sum;
  }

  return result;
}

template <typename T>
RankThreeTensorTempl<T>
RankThreeTensorTempl<T>::operator*(const T b) const
{
  RankThreeTensorTempl<T> result;

  for (auto i : make_range(N3))
    result._vals[i] = _vals[i] * b;

  return result;
}

template <typename T>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator*=(const T a)
{
  for (auto i : make_range(N3))
    _vals[i] *= a;

  return *this;
}

template <typename T>
RankThreeTensorTempl<T>
RankThreeTensorTempl<T>::operator/(const T b) const
{
  RankThreeTensorTempl<T> result;

  for (auto i : make_range(N3))
    result._vals[i] = _vals[i] / b;

  return result;
}

template <typename T>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator/=(const T a)
{
  for (auto i : make_range(N3))
    _vals[i] /= a;

  return *this;
}

template <typename T>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator+=(const RankThreeTensorTempl<T> & a)
{
  for (auto i : make_range(N3))
    _vals[i] += a._vals[i];

  return *this;
}

template <typename T>
RankThreeTensorTempl<T>
RankThreeTensorTempl<T>::operator+(const RankThreeTensorTempl<T> & b) const
{
  RankThreeTensorTempl<T> result;

  for (auto i : make_range(N3))
    result._vals[i] = _vals[i] + b._vals[i];

  return result;
}

template <typename T>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator-=(const RankThreeTensorTempl<T> & a)
{
  for (auto i : make_range(N3))
    _vals[i] -= a._vals[i];

  return *this;
}

template <typename T>
RankThreeTensorTempl<T>
RankThreeTensorTempl<T>::operator-(const RankThreeTensorTempl<T> & b) const
{
  RankThreeTensorTempl<T> result;

  for (auto i : make_range(N3))
    result._vals[i] = _vals[i] - b._vals[i];

  return result;
}

template <typename T>
RankThreeTensorTempl<T>
RankThreeTensorTempl<T>::operator-() const
{
  RankThreeTensorTempl<T> result;

  for (auto i : make_range(N3))
    result._vals[i] = -_vals[i];

  return result;
}

template <typename T>
T
RankThreeTensorTempl<T>::L2norm() const
{
  T l2 = 0;

  for (auto i : make_range(N3))
    l2 += Utility::pow<2>(_vals[i]);

  return std::sqrt(l2);
}

template <typename T>
void
RankThreeTensorTempl<T>::fillFromInputVector(const std::vector<T> & input, FillMethod fill_method)
{
  zero();

  if (fill_method == automatic)
  {
    if (input.size() == 27)
      fill_method = general;
    else if (input.size() == 3)
      fill_method = plane_normal;
    else
      mooseError("Unsupported automatic fill method, use 27 values for 'general' and 3 for "
                 "'plane_normal', the supplied size was ",
                 input.size(),
                 ".");
  }

  if (fill_method == general)
    fillGeneralFromInputVector(input);

  else if (fill_method == plane_normal)
  {
    if (input.size() != 3)
      mooseError("To use fillFromPlaneNormal, your input must have size 3, the supplied size was ",
                 input.size(),
                 ".");
    fillFromPlaneNormal(libMesh::VectorValue<T>(input[0], input[1], input[2]));
  }

  else
    // This is un-reachable unless a FillMethod is added and the if statement is not updated
    mooseError("fillFromInputVector called with unknown fill_method of ", fill_method);
}

template <typename T>
void
RankThreeTensorTempl<T>::fillFromPlaneNormal(const libMesh::VectorValue<T> & input)
{
  unsigned int index = 0;
  for (auto i : make_range(N))
  {
    const T a = input(i);
    for (auto j : make_range(N))
    {
      const T b = input(j);
      for (auto k : make_range(N))
      {
        const T c = input(k);
        T sum = 0;
        sum = -2.0 * a * b * c;
        if (i == j)
          sum += c;
        if (i == k)
          sum += b;
        _vals[index++] = sum / 2.0;
      }
    }
  }
}

template <typename T>
RankFourTensorTempl<T>
RankThreeTensorTempl<T>::mixedProductRankFour(const RankTwoTensorTempl<T> & a) const
{
  RankFourTensorTempl<T> result;

  unsigned int index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          for (auto m : make_range(N))
            for (auto n : make_range(N))
              result._vals[index] += (*this)(m, i, j) * a(m, n) * (*this)(n, k, l);
          index++;
        }

  return result;
}

template <typename T>
void
RankThreeTensorTempl<T>::rotate(const libMesh::TensorValue<T> & R)
{
  RankThreeTensorTempl<T> old = *this;

  unsigned int index = 0;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
      {
        T sum = 0.0;
        unsigned int index2 = 0;
        for (auto m : make_range(N))
        {
          T a = R(i, m);
          for (auto n : make_range(N))
          {
            T ab = a * R(j, n);
            for (auto o : make_range(N))
              sum += ab * R(k, o) * old._vals[index2++];
          }
        }
        _vals[index++] = sum;
      }
}

template <typename T>
void
RankThreeTensorTempl<T>::fillGeneralFromInputVector(const std::vector<T> & input)
{
  if (input.size() != 27)
    mooseError(
        "To use fillGeneralFromInputVector, your input must have size 27, the supplied size was ",
        input.size(),
        ".");

  for (auto i : make_range(N3))
    _vals[i] = input[i];
}

template <typename T>
libMesh::VectorValue<T>
RankThreeTensorTempl<T>::doubleContraction(const RankTwoTensorTempl<T> & b) const
{
  libMesh::VectorValue<T> result;

  for (auto i : make_range(N))
    for (auto j : make_range(N2))
      result(i) += _vals[i * N2 + j] * b._coords[j];

  return result;
}

template <typename T>
void
RankThreeTensorTempl<T>::print(std::ostream & stm) const
{
  for (auto i : make_range(N))
  {
    stm << "a(" << i << ", j, k) = \n";
    for (auto j : make_range(N))
    {
      for (auto k : make_range(N))
        stm << std::setw(15) << (*this)(i, j, k) << ' ';
      stm << "\n";
    }
    stm << "\n";
  }

  stm << std::flush;
}
