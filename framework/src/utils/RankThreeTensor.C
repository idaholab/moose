//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankThreeTensor.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MooseException.h"
#include "MooseUtils.h"
#include "MatrixTools.h"
#include "MaterialProperty.h"
#include "PermutationTensor.h"

#include "libmesh/utility.h"

// C++ includes
#include <iomanip>
#include <ostream>

template <>
void
mooseSetToZero<RankThreeTensor>(RankThreeTensor & v)
{
  v.zero();
}

template <>
void
dataStore(std::ostream & stream, RankThreeTensor & rtht, void * context)
{
  dataStore(stream, rtht._vals, context);
}

template <>
void
dataLoad(std::istream & stream, RankThreeTensor & rtht, void * context)
{
  dataLoad(stream, rtht._vals, context);
}

MooseEnum RankThreeTensor::fillMethodEnum() // TODO: Need new fillMethodEnum() -- for now we will
                                            // just use general (at most 27 components)
{
  return MooseEnum("general");
}

RankThreeTensor::RankThreeTensor()
{
  mooseAssert(N == 3, "RankThreeTensor is currently only tested for 3 dimensions.");

  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] = 0;
}

RankThreeTensor::RankThreeTensor(const InitMethod init)
{
  switch (init)
  {
    case initNone:
      break;

    default:
      mooseError("Unknown RankThreeTensor initialization pattern.");
  }
}

RankThreeTensor::RankThreeTensor(const std::vector<Real> & input, FillMethod fill_method)
{
  fillFromInputVector(input, fill_method);
}

void
RankThreeTensor::zero()
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] = 0;
}

RankThreeTensor &
RankThreeTensor::operator=(const RankThreeTensor & a)
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] = a._vals[i];

  return *this;
}

RealVectorValue RankThreeTensor::operator*(const RankTwoTensor & a) const
{
  RealVectorValue result;

  for (unsigned int i = 0; i < N; ++i)
  {
    Real sum = 0;
    unsigned int i1 = i * N2;
    for (unsigned int j1 = 0; j1 < N2; j1 += N)
      for (unsigned int k = 0; k < N; ++k)
        sum += _vals[i1 + j1 + k] * a._vals[j1 + k];
    result(i) = sum;
  }

  return result;
}

RankThreeTensor RankThreeTensor::operator*(const Real b) const
{
  RankThreeTensor result;

  for (unsigned int i = 0; i < N3; ++i)
    result._vals[i] = _vals[i] * b;

  return result;
}

RankThreeTensor &
RankThreeTensor::operator*=(const Real a)
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] *= a;

  return *this;
}

RankThreeTensor
RankThreeTensor::operator/(const Real b) const
{
  RankThreeTensor result;

  for (unsigned int i = 0; i < N3; ++i)
    result._vals[i] = _vals[i] / b;

  return result;
}

RankThreeTensor &
RankThreeTensor::operator/=(const Real a)
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] /= a;

  return *this;
}

RankThreeTensor &
RankThreeTensor::operator+=(const RankThreeTensor & a)
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] += a._vals[i];

  return *this;
}

RankThreeTensor
RankThreeTensor::operator+(const RankThreeTensor & b) const
{
  RankThreeTensor result;

  for (unsigned int i = 0; i < N3; ++i)
    result._vals[i] = _vals[i] + b._vals[i];

  return result;
}

RankThreeTensor &
RankThreeTensor::operator-=(const RankThreeTensor & a)
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] -= a._vals[i];

  return *this;
}

RankThreeTensor
RankThreeTensor::operator-(const RankThreeTensor & b) const
{
  RankThreeTensor result;

  for (unsigned int i = 0; i < N3; ++i)
    result._vals[i] = _vals[i] - b._vals[i];

  return result;
}

RankThreeTensor
RankThreeTensor::operator-() const
{
  RankThreeTensor result;

  for (unsigned int i = 0; i < N3; ++i)
    result._vals[i] = -_vals[i];

  return result;
}

Real
RankThreeTensor::L2norm() const
{
  Real l2 = 0;

  for (unsigned int i = 0; i < N3; ++i)
    l2 += Utility::pow<2>(_vals[i]);

  return std::sqrt(l2);
}

void
RankThreeTensor::fillFromInputVector(const std::vector<Real> & input, FillMethod fill_method)
{
  zero();

  switch (fill_method)
  {
    case general:
      fillGeneralFromInputVector(input);
      break;
    default:
      mooseError("fillFromInputVector called with unknown fill_method of ", fill_method);
  }
}

void
RankThreeTensor::fillFromPlaneNormal(const RealVectorValue & input)
{
  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    const Real a = input(i);
    for (unsigned int j = 0; j < N; ++j)
    {
      const Real b = input(j);
      for (unsigned int k = 0; k < N; ++k)
      {
        const Real c = input(k);
        Real sum = 0;
        sum = -2 * a * b * c;
        if (i == j)
          sum += c;
        if (i == k)
          sum += b;
        _vals[index++] = sum / 2.0;
      }
    }
  }
}

RankFourTensor
RankThreeTensor::mixedProductRankFour(const RankTwoTensor & a) const
{
  RankFourTensor result;

  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          for (unsigned int m = 0; m < N; ++m)
            for (unsigned int n = 0; n < N; ++n)
              result._vals[index] += (*this)(m, i, j) * a._vals[m * N + n] * (*this)(n, k, l);
          index++;
        }

  return result;
}

void
RankThreeTensor::rotate(const RealTensorValue & R)
{
  RankThreeTensor old = *this;

  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
      {
        Real sum = 0.0;
        unsigned int index2 = 0;
        for (unsigned int m = 0; m < N; ++m)
        {
          Real a = R(i, m);
          for (unsigned int n = 0; n < N; ++n)
          {
            Real ab = a * R(j, n);
            for (unsigned int o = 0; o < N; ++o)
              sum += ab * R(k, o) * old._vals[index2++];
          }
        }
        _vals[index++] = sum;
      }
}

void
RankThreeTensor::rotate(const RankTwoTensor & R)
{
  RankThreeTensor old = *this;

  unsigned int index = 0;
  unsigned int i1 = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    unsigned int j1 = 0;
    for (unsigned int j = 0; j < N; ++j)
    {
      unsigned int k1 = 0;
      for (unsigned int k = 0; k < N; ++k)
      {
        Real sum = 0.0;
        unsigned int index2 = 0;
        for (unsigned int m = 0; m < N; ++m)
        {
          Real a = R._vals[i1 + m];
          for (unsigned int n = 0; n < N; ++n)
          {
            Real ab = a * R._vals[j1 + n];
            for (unsigned int o = 0; o < N; ++o)
              sum += ab * R._vals[k1 + o] * old._vals[index2++];
          }
        }
        _vals[index++] = sum;
        k1 += N;
      }
      j1 += N;
    }
    i1 += N;
  }
}

void
RankThreeTensor::fillGeneralFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 27)
    mooseError("To use fillGeneralFromInputVector, your input must have size 27. Yours has size ",
               input.size());

  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] = input[i];
}

RankTwoTensor operator*(const RealVectorValue & p, const RankThreeTensor & b)
{
  RankTwoTensor result;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        result(i, j) += p(k) * b(k, i, j);

  return result;
}

RealVectorValue
RankThreeTensor::doubleContraction(const RankTwoTensor & b) const
{
  RealVectorValue result;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N2; ++j)
      result(i) += _vals[i * N2 + j] * b._vals[j];

  return result;
}
