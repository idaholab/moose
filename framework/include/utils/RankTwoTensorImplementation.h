//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RankTwoTensor.h"

// MOOSE includes
#include "MooseEnum.h"
#include "ColumnMajorMatrix.h"
#include "MooseRandom.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "Conversion.h"
#include "MooseArray.h"

#include "libmesh/libmesh.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"
#include "libmesh/utility.h"

// PETSc includes
#include <petscblaslapack.h>

// C++ includes
#include <iomanip>
#include <ostream>
#include <vector>
#include <array>

// Eigen includes
#include <Eigen/Core>
#include <Eigen/Eigenvalues>

namespace Eigen
{
namespace internal
{
template <>
struct cast_impl<ADReal, int>
{
  static inline int run(const ADReal & x) { return static_cast<int>(MetaPhysicL::raw_value(x)); }
};
} // namespace internal
} // namespace Eigen

template <typename T>
constexpr Real RankTwoTensorTempl<T>::identityCoords[];

namespace MathUtils
{
template <>
void mooseSetToZero<RankTwoTensor>(RankTwoTensor & v);
template <>
void mooseSetToZero<ADRankTwoTensor>(ADRankTwoTensor & v);
}

template <typename T>
MooseEnum
RankTwoTensorTempl<T>::fillMethodEnum()
{
  return MooseEnum("autodetect=0 isotropic1=1 diagonal3=3 symmetric6=6 general=9", "autodetect");
}

template <typename T>
RankTwoTensorTempl<T>::RankTwoTensorTempl()
{
  mooseAssert(N == 3, "RankTwoTensorTempl is currently only tested for 3 dimensions.");

  for (unsigned int i = 0; i < N2; i++)
    _coords[i] = 0.0;
}

template <typename T>
RankTwoTensorTempl<T>::RankTwoTensorTempl(const InitMethod init)
{
  switch (init)
  {
    case initNone:
      break;

    case initIdentity:
      this->zero();
      for (const auto i : make_range(N))
        (*this)(i, i) = 1.0;
      break;

    default:
      mooseError("Unknown RankTwoTensorTempl initialization pattern.");
  }
}

template <typename T>
RankTwoTensorTempl<T>::RankTwoTensorTempl(const libMesh::TypeVector<T> & row1,
                                          const libMesh::TypeVector<T> & row2,
                                          const libMesh::TypeVector<T> & row3)
{
  mooseDeprecated(
      "This constructor is deprecated in favor of RankTwoTensorTempl<T>::initializeFromRows");

  // Initialize the Tensor matrix from the passed in vectors
  for (const auto i : make_range(N))
    _coords[i] = row1(i);

  for (const auto i : make_range(N))
    _coords[N + i] = row2(i);

  for (const auto i : make_range(N))
    _coords[2 * N + i] = row3(i);
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::initializeSymmetric(const libMesh::TypeVector<T> & v0,
                                           const libMesh::TypeVector<T> & v1,
                                           const libMesh::TypeVector<T> & v2)
{
  return RankTwoTensorTempl<T>(v0(0),
                               (v1(0) + v0(1)) / 2.0,
                               (v2(0) + v0(2)) / 2.0,
                               (v1(0) + v0(1)) / 2.0,
                               v1(1),
                               (v2(1) + v1(2)) / 2.0,
                               (v2(0) + v0(2)) / 2.0,
                               (v2(1) + v1(2)) / 2.0,
                               v2(2));
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::initializeFromRows(const libMesh::TypeVector<T> & row0,
                                          const libMesh::TypeVector<T> & row1,
                                          const libMesh::TypeVector<T> & row2)
{
  return RankTwoTensorTempl<T>(
      row0(0), row1(0), row2(0), row0(1), row1(1), row2(1), row0(2), row1(2), row2(2));
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::initializeFromColumns(const libMesh::TypeVector<T> & col0,
                                             const libMesh::TypeVector<T> & col1,
                                             const libMesh::TypeVector<T> & col2)
{
  return RankTwoTensorTempl<T>(
      col0(0), col0(1), col0(2), col1(0), col1(1), col1(2), col2(0), col2(1), col2(2));
}

template <typename T>
RankTwoTensorTempl<T>::RankTwoTensorTempl(
    const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12)
{
  (*this)(0, 0) = S11;
  (*this)(1, 1) = S22;
  (*this)(2, 2) = S33;
  (*this)(1, 2) = (*this)(2, 1) = S23;
  (*this)(0, 2) = (*this)(2, 0) = S13;
  (*this)(0, 1) = (*this)(1, 0) = S12;
}

template <typename T>
RankTwoTensorTempl<T>::RankTwoTensorTempl(const T & S11,
                                          const T & S21,
                                          const T & S31,
                                          const T & S12,
                                          const T & S22,
                                          const T & S32,
                                          const T & S13,
                                          const T & S23,
                                          const T & S33)
{
  (*this)(0, 0) = S11;
  (*this)(1, 0) = S21;
  (*this)(2, 0) = S31;
  (*this)(0, 1) = S12;
  (*this)(1, 1) = S22;
  (*this)(2, 1) = S32;
  (*this)(0, 2) = S13;
  (*this)(1, 2) = S23;
  (*this)(2, 2) = S33;
}

template <typename T>
void
RankTwoTensorTempl<T>::fillFromInputVector(const std::vector<T> & input, FillMethod fill_method)
{
  if (fill_method != autodetect && fill_method != input.size())
    mooseError("Expected an input vector size of ", fill_method, " to fill the RankTwoTensorTempl");

  switch (input.size())
  {
    case 1:
      this->zero();
      (*this)(0, 0) = input[0]; // S11
      (*this)(1, 1) = input[0]; // S22
      (*this)(2, 2) = input[0]; // S33
      break;

    case 3:
      this->zero();
      (*this)(0, 0) = input[0]; // S11
      (*this)(1, 1) = input[1]; // S22
      (*this)(2, 2) = input[2]; // S33
      break;

    case 6:
      (*this)(0, 0) = input[0];                 // S11
      (*this)(1, 1) = input[1];                 // S22
      (*this)(2, 2) = input[2];                 // S33
      (*this)(1, 2) = (*this)(2, 1) = input[3]; // S23
      (*this)(0, 2) = (*this)(2, 0) = input[4]; // S13
      (*this)(0, 1) = (*this)(1, 0) = input[5]; // S12
      break;

    case 9:
      (*this)(0, 0) = input[0]; // S11
      (*this)(1, 0) = input[1]; // S21
      (*this)(2, 0) = input[2]; // S31
      (*this)(0, 1) = input[3]; // S12
      (*this)(1, 1) = input[4]; // S22
      (*this)(2, 1) = input[5]; // S32
      (*this)(0, 2) = input[6]; // S13
      (*this)(1, 2) = input[7]; // S23
      (*this)(2, 2) = input[8]; // S33
      break;

    default:
      mooseError("Please check the number of entries in the input vector for building "
                 "a RankTwoTensorTempl. It must be 1, 3, 6, or 9");
  }
}

template <typename T>
void
RankTwoTensorTempl<T>::fillFromScalarVariable(const VariableValue & scalar_variable)
{
  switch (scalar_variable.size())
  {
    case 1:
      this->zero();
      (*this)(0, 0) = scalar_variable[0]; // S11
      break;

    case 3:
      this->zero();
      (*this)(0, 0) = scalar_variable[0];                 // S11
      (*this)(1, 1) = scalar_variable[1];                 // S22
      (*this)(0, 1) = (*this)(1, 0) = scalar_variable[2]; // S12
      break;

    case 6:
      (*this)(0, 0) = scalar_variable[0];                 // S11
      (*this)(1, 1) = scalar_variable[1];                 // S22
      (*this)(2, 2) = scalar_variable[2];                 // S33
      (*this)(1, 2) = (*this)(2, 1) = scalar_variable[3]; // S23
      (*this)(0, 2) = (*this)(2, 0) = scalar_variable[4]; // S13
      (*this)(0, 1) = (*this)(1, 0) = scalar_variable[5]; // S12
      break;

    default:
      mooseError("Only FIRST, THIRD, or SIXTH order scalar variable can be used to build "
                 "a RankTwoTensorTempl.");
  }
}

template <typename T>
libMesh::VectorValue<T>
RankTwoTensorTempl<T>::column(const unsigned int c) const
{
  return libMesh::VectorValue<T>((*this)(0, c), (*this)(1, c), (*this)(2, c));
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::timesTranspose(const RankTwoTensorTempl<T> & a)
{
  return a * a.transpose();
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::transposeTimes(const RankTwoTensorTempl<T> & a)
{
  return a.transpose() * a;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::plusTranspose(const RankTwoTensorTempl<T> & a)
{
  return a + a.transpose();
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::square() const
{
  return *this * *this;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::rotated(const RankTwoTensorTempl<T> & R) const
{
  RankTwoTensorTempl<T> result(*this);
  result.rotate(R);
  return result;
}

template <typename T>
void
RankTwoTensorTempl<T>::rotate(const RankTwoTensorTempl<T> & R)
{
  RankTwoTensorTempl<T> temp;
  for (const auto i : make_range(N))
  {
    const auto i1 = i * N;
    for (const auto j : make_range(N))
    {
      // tmp += R(i,k)*R(j,l)*(*this)(k,l);
      // clang-format off
      const auto j1 = j * N;
      T tmp = R._coords[i1 + 0] * R._coords[j1 + 0] * (*this)(0, 0) +
                 R._coords[i1 + 0] * R._coords[j1 + 1] * (*this)(0, 1) +
                 R._coords[i1 + 0] * R._coords[j1 + 2] * (*this)(0, 2) +
                 R._coords[i1 + 1] * R._coords[j1 + 0] * (*this)(1, 0) +
                 R._coords[i1 + 1] * R._coords[j1 + 1] * (*this)(1, 1) +
                 R._coords[i1 + 1] * R._coords[j1 + 2] * (*this)(1, 2) +
                 R._coords[i1 + 2] * R._coords[j1 + 0] * (*this)(2, 0) +
                 R._coords[i1 + 2] * R._coords[j1 + 1] * (*this)(2, 1) +
                 R._coords[i1 + 2] * R._coords[j1 + 2] * (*this)(2, 2);
      // clang-format on
      temp._coords[i1 + j] = tmp;
    }
  }
  for (unsigned int i = 0; i < N2; i++)
    _coords[i] = temp._coords[i];
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::rotateXyPlane(T a)
{
  T c = std::cos(a);
  T s = std::sin(a);
  T x = (*this)(0, 0) * c * c + (*this)(1, 1) * s * s + 2.0 * (*this)(0, 1) * c * s;
  T y = (*this)(0, 0) * s * s + (*this)(1, 1) * c * c - 2.0 * (*this)(0, 1) * c * s;
  T xy = ((*this)(1, 1) - (*this)(0, 0)) * c * s + (*this)(0, 1) * (c * c - s * s);

  RankTwoTensorTempl<T> b(*this);

  b(0, 0) = x;
  b(1, 1) = y;
  b(1, 0) = b(0, 1) = xy;

  return b;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::transpose() const
{
  return libMesh::TensorValue<T>::transpose();
}

template <typename T>
RankTwoTensorTempl<T> &
RankTwoTensorTempl<T>::operator+=(const RankTwoTensorTempl<T> & a)
{
  libMesh::TensorValue<T>::operator+=(a);
  return *this;
}

template <typename T>
RankTwoTensorTempl<T> &
RankTwoTensorTempl<T>::operator-=(const RankTwoTensorTempl<T> & a)
{
  libMesh::TensorValue<T>::operator-=(a);
  return *this;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::operator-() const
{
  return libMesh::TensorValue<T>::operator-();
}

template <typename T>
RankTwoTensorTempl<T> &
RankTwoTensorTempl<T>::operator*=(const T & a)
{
  libMesh::TensorValue<T>::operator*=(a);
  return *this;
}

template <typename T>
RankTwoTensorTempl<T> &
RankTwoTensorTempl<T>::operator/=(const T & a)
{
  libMesh::TensorValue<T>::operator/=(a);
  return *this;
}

template <typename T>
RankTwoTensorTempl<T> &
RankTwoTensorTempl<T>::operator*=(const libMesh::TypeTensor<T> & a)
{
  *this = *this * a;
  return *this;
}

template <typename T>
bool
RankTwoTensorTempl<T>::operator==(const RankTwoTensorTempl<T> & a) const
{
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      if (!MooseUtils::absoluteFuzzyEqual((*this)(i, j), a(i, j)))
        return false;

  return true;
}

template <typename T>
bool
RankTwoTensorTempl<T>::isSymmetric() const
{
  auto test = MetaPhysicL::raw_value(*this - transpose());
  return MooseUtils::absoluteFuzzyEqual(test.norm_sq(), 0);
}

template <typename T>
RankTwoTensorTempl<T> &
RankTwoTensorTempl<T>::operator=(const ColumnMajorMatrixTempl<T> & a)
{
  if (a.n() != N || a.m() != N)
    mooseError("Dimensions of ColumnMajorMatrixTempl<T> are incompatible with RankTwoTensorTempl");

  const T * cmm_rawdata = a.rawData();
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      _coords[i * N + j] = cmm_rawdata[i + j * N];

  return *this;
}

template <typename T>
T
RankTwoTensorTempl<T>::doubleContraction(const RankTwoTensorTempl<T> & b) const
{
  // deprecate this!
  return libMesh::TensorValue<T>::contract(b);
}

template <typename T>
RankThreeTensorTempl<T>
RankTwoTensorTempl<T>::contraction(const RankThreeTensorTempl<T> & b) const
{
  RankThreeTensorTempl<T> result;

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        for (const auto l : make_range(N))
          result(i, k, l) += (*this)(i, j) * b(j, k, l);

  return result;
}

template <typename T>
RankThreeTensorTempl<T>
RankTwoTensorTempl<T>::mixedProductJkI(const libMesh::VectorValue<T> & b) const
{
  RankThreeTensorTempl<T> result;

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        result(i, j, k) += (*this)(j, k) * b(i);

  return result;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::deviatoric() const
{
  RankTwoTensorTempl<T> deviatoric(*this);
  // actually construct deviatoric part
  deviatoric.addIa(-1.0 / 3.0 * this->tr());
  return deviatoric;
}

template <typename T>
T
RankTwoTensorTempl<T>::generalSecondInvariant() const
{
  return (*this)(0, 0) * (*this)(1, 1) + (*this)(0, 0) * (*this)(2, 2) +
         (*this)(1, 1) * (*this)(2, 2) - (*this)(0, 1) * (*this)(1, 0) -
         (*this)(0, 2) * (*this)(2, 0) - (*this)(1, 2) * (*this)(2, 1);
}

template <typename T>
T
RankTwoTensorTempl<T>::secondInvariant() const
{
  T result;

  // RankTwoTensorTempl<T> deviatoric(*this);
  // deviatoric.addIa(-1.0/3.0 * this->tr()); // actually construct deviatoric part
  // result = 0.5*(deviatoric + deviatoric.transpose()).doubleContraction(deviatoric +
  // deviatoric.transpose());
  result = Utility::pow<2>((*this)(0, 0) - (*this)(1, 1)) / 6.0;
  result += Utility::pow<2>((*this)(0, 0) - (*this)(2, 2)) / 6.0;
  result += Utility::pow<2>((*this)(1, 1) - (*this)(2, 2)) / 6.0;
  result += Utility::pow<2>((*this)(0, 1) + (*this)(1, 0)) / 4.0;
  result += Utility::pow<2>((*this)(0, 2) + (*this)(2, 0)) / 4.0;
  result += Utility::pow<2>((*this)(1, 2) + (*this)(2, 1)) / 4.0;
  return result;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::dsecondInvariant() const
{
  return RankTwoTensorTempl<T>::plusTranspose(deviatoric()) * 0.5;
}

template <typename T>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::d2secondInvariant() const
{
  RankFourTensorTempl<T> result;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        for (const auto l : make_range(N))
          result(i, j, k, l) = 0.5 * (i == k) * (j == l) + 0.5 * (i == l) * (j == k) -
                               (1.0 / 3.0) * (i == j) * (k == l);
  return result;
}

template <typename T>
T
RankTwoTensorTempl<T>::trace() const
{
  // deprecate this!
  return this->tr();
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::dtrace() const
{
  return RankTwoTensorTempl<T>(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::inverse() const
{
  return libMesh::TensorValue<T>::inverse();
}

template <typename T>
T
RankTwoTensorTempl<T>::thirdInvariant() const
{
  const auto s = RankTwoTensorTempl<T>::plusTranspose(deviatoric()) * 0.5;
  return s(0, 0) * (s(1, 1) * s(2, 2) - s(2, 1) * s(1, 2)) -
         s(1, 0) * (s(0, 1) * s(2, 2) - s(2, 1) * s(0, 2)) +
         s(2, 0) * (s(0, 1) * s(1, 2) - s(1, 1) * s(0, 2));
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::dthirdInvariant() const
{
  const auto s = RankTwoTensorTempl<T>::plusTranspose(deviatoric()) * 0.5;
  const T s3 = secondInvariant() / 3.0;

  RankTwoTensorTempl<T> d;
  d(0, 0) = s(1, 1) * s(2, 2) - s(2, 1) * s(1, 2) + s3;
  d(0, 1) = s(2, 0) * s(1, 2) - s(1, 0) * s(2, 2);
  d(0, 2) = s(1, 0) * s(2, 1) - s(2, 0) * s(1, 1);
  d(1, 0) = s(2, 1) * s(0, 2) - s(0, 1) * s(2, 2);
  d(1, 1) = s(0, 0) * s(2, 2) - s(2, 0) * s(0, 2) + s3;
  d(1, 2) = s(2, 0) * s(0, 1) - s(0, 0) * s(2, 1);
  d(2, 0) = s(0, 1) * s(1, 2) - s(1, 1) * s(0, 2);
  d(2, 1) = s(1, 0) * s(0, 2) - s(0, 0) * s(1, 2);
  d(2, 2) = s(0, 0) * s(1, 1) - s(1, 0) * s(0, 1) + s3;
  return d;
}

template <typename T>
RankFourTensorTempl<T>
RankTwoTensorTempl<T>::d2thirdInvariant() const
{
  const auto s = RankTwoTensorTempl<T>::plusTranspose(deviatoric()) * 0.5;

  RankFourTensorTempl<T> d2;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        for (const auto l : make_range(N))
        {
          d2(i, j, k, l) = Real(i == j) * s(k, l) / 3.0 + Real(k == l) * s(i, j) / 3.0;
          // for (const auto a: make_range(N))
          //  for (const auto b: make_range(N))
          //    d2(i, j, k, l) += 0.5*(PermutationTensor::eps(i, k, a)*PermutationTensor::eps(j, l,
          //    b) + PermutationTensor::eps(i, l, a)*PermutationTensor::eps(j, k, b))*s(a, b);
        }

  // I'm not sure which is more readable: the above
  // PermutationTensor stuff, or the stuff below.
  // Anyway, they yield the same result, and so i leave
  // both of them here to enlighten you!

  d2(0, 0, 1, 1) += s(2, 2);
  d2(0, 0, 1, 2) -= s(2, 1);
  d2(0, 0, 2, 1) -= s(1, 2);
  d2(0, 0, 2, 2) += s(1, 1);

  d2(0, 1, 0, 1) -= s(2, 2) / 2.0;
  d2(0, 1, 1, 0) -= s(2, 2) / 2.0;
  d2(0, 1, 0, 2) += s(1, 2) / 2.0;
  d2(0, 1, 2, 0) += s(1, 2) / 2.0;
  d2(0, 1, 1, 2) += s(2, 0) / 2.0;
  d2(0, 1, 2, 1) += s(2, 0) / 2.0;
  d2(0, 1, 2, 2) -= s(1, 0);

  d2(0, 2, 0, 1) += s(2, 1) / 2.0;
  d2(0, 2, 1, 0) += s(2, 1) / 2.0;
  d2(0, 2, 0, 2) -= s(1, 1) / 2.0;
  d2(0, 2, 2, 0) -= s(1, 1) / 2.0;
  d2(0, 2, 1, 1) -= s(2, 0);
  d2(0, 2, 1, 2) += s(1, 0) / 2.0;
  d2(0, 2, 2, 1) += s(1, 0) / 2.0;

  d2(1, 0, 0, 1) -= s(2, 2) / 2.0;
  d2(1, 0, 1, 0) -= s(2, 2) / 2.0;
  d2(1, 0, 0, 2) += s(1, 2) / 2.0;
  d2(1, 0, 2, 0) += s(1, 2) / 2.0;
  d2(1, 0, 1, 2) += s(2, 0) / 2.0;
  d2(1, 0, 2, 1) += s(2, 0) / 2.0;
  d2(1, 0, 2, 2) -= s(1, 0);

  d2(1, 1, 0, 0) += s(2, 2);
  d2(1, 1, 0, 2) -= s(2, 0);
  d2(1, 1, 2, 0) -= s(2, 0);
  d2(1, 1, 2, 2) += s(0, 0);

  d2(1, 2, 0, 0) -= s(2, 1);
  d2(1, 2, 0, 1) += s(2, 0) / 2.0;
  d2(1, 2, 1, 0) += s(2, 0) / 2.0;
  d2(1, 2, 0, 2) += s(0, 1) / 2.0;
  d2(1, 2, 2, 0) += s(0, 1) / 2.0;
  d2(1, 2, 1, 2) -= s(0, 0) / 2.0;
  d2(1, 2, 2, 1) -= s(0, 0) / 2.0;

  d2(2, 0, 0, 1) += s(2, 1) / 2.0;
  d2(2, 0, 1, 0) += s(2, 1) / 2.0;
  d2(2, 0, 0, 2) -= s(1, 1) / 2.0;
  d2(2, 0, 2, 0) -= s(1, 1) / 2.0;
  d2(2, 0, 1, 1) -= s(2, 0);
  d2(2, 0, 1, 2) += s(1, 0) / 2.0;
  d2(2, 0, 2, 1) += s(1, 0) / 2.0;

  d2(2, 1, 0, 0) -= s(2, 1);
  d2(2, 1, 0, 1) += s(2, 0) / 2.0;
  d2(2, 1, 1, 0) += s(2, 0) / 2.0;
  d2(2, 1, 0, 2) += s(0, 1) / 2.0;
  d2(2, 1, 2, 0) += s(0, 1) / 2.0;
  d2(2, 1, 1, 2) -= s(0, 0) / 2.0;
  d2(2, 1, 2, 1) -= s(0, 0) / 2.0;

  d2(2, 2, 0, 0) += s(1, 1);
  d2(2, 2, 0, 1) -= s(1, 0);
  d2(2, 2, 1, 0) -= s(1, 0);
  d2(2, 2, 1, 1) += s(0, 0);

  return d2;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::ddet() const
{
  RankTwoTensorTempl<T> d;

  d(0, 0) = (*this)(1, 1) * (*this)(2, 2) - (*this)(2, 1) * (*this)(1, 2);
  d(0, 1) = (*this)(2, 0) * (*this)(1, 2) - (*this)(1, 0) * (*this)(2, 2);
  d(0, 2) = (*this)(1, 0) * (*this)(2, 1) - (*this)(2, 0) * (*this)(1, 1);
  d(1, 0) = (*this)(2, 1) * (*this)(0, 2) - (*this)(0, 1) * (*this)(2, 2);
  d(1, 1) = (*this)(0, 0) * (*this)(2, 2) - (*this)(2, 0) * (*this)(0, 2);
  d(1, 2) = (*this)(2, 0) * (*this)(0, 1) - (*this)(0, 0) * (*this)(2, 1);
  d(2, 0) = (*this)(0, 1) * (*this)(1, 2) - (*this)(1, 1) * (*this)(0, 2);
  d(2, 1) = (*this)(1, 0) * (*this)(0, 2) - (*this)(0, 0) * (*this)(1, 2);
  d(2, 2) = (*this)(0, 0) * (*this)(1, 1) - (*this)(1, 0) * (*this)(0, 1);

  return d;
}

template <typename T>
void
RankTwoTensorTempl<T>::print(std::ostream & stm) const
{
  const RankTwoTensorTempl<T> & a = *this;
  for (const auto i : make_range(N))
  {
    for (const auto j : make_range(N))
      stm << std::setw(15) << a(i, j) << ' ';
    stm << std::endl;
  }
}

template <>
void RankTwoTensor::printReal(std::ostream & stm) const;

template <>
void ADRankTwoTensor::printReal(std::ostream & stm) const;

template <>
void ADRankTwoTensor::printADReal(unsigned int nDual, std::ostream & stm) const;

template <typename T>
void
RankTwoTensorTempl<T>::addIa(const T & a)
{
  for (const auto i : make_range(N))
    (*this)(i, i) += a;
}

template <typename T>
T
RankTwoTensorTempl<T>::L2norm() const
{
  T norm = 0.0;
  for (const auto i : make_range(N2))
  {
    T v = _coords[i];
    norm += v * v;
  }
  return norm == 0.0 ? 0.0 : std::sqrt(norm);
}

template <typename T>
void
RankTwoTensorTempl<T>::surfaceFillFromInputVector(const std::vector<T> & input)
{
  if (input.size() == 4)
  {
    // initialize with zeros
    this->zero();
    (*this)(0, 0) = input[0];
    (*this)(0, 1) = input[1];
    (*this)(1, 0) = input[2];
    (*this)(1, 1) = input[3];
  }
  else
    mooseError("please provide correct number of values for surface RankTwoTensorTempl<T> "
               "initialization.");
}

template <typename T>
void
RankTwoTensorTempl<T>::syev(const char *, std::vector<T> &, std::vector<T> &) const
{
  mooseError("The syev method is only supported for Real valued tensors");
}

template <>
void RankTwoTensor::syev(const char * calculation_type,
                         std::vector<Real> & eigvals,
                         std::vector<Real> & a) const;

template <typename T>
void
RankTwoTensorTempl<T>::symmetricEigenvalues(std::vector<T> & eigvals) const
{
  RankTwoTensorTempl<T> a;
  symmetricEigenvaluesEigenvectors(eigvals, a);
}

template <>
void RankTwoTensor::symmetricEigenvalues(std::vector<Real> & eigvals) const;

template <typename T>
void
RankTwoTensorTempl<T>::symmetricEigenvaluesEigenvectors(std::vector<T> &,
                                                        RankTwoTensorTempl<T> &) const
{
  mooseError(
      "symmetricEigenvaluesEigenvectors is only available for ordered tensor component types");
}

template <>
void RankTwoTensor::symmetricEigenvaluesEigenvectors(std::vector<Real> & eigvals,
                                                     RankTwoTensor & eigvecs) const;

template <>
void ADRankTwoTensor::symmetricEigenvaluesEigenvectors(std::vector<ADReal> & eigvals,
                                                       ADRankTwoTensor & eigvecs) const;

template <typename T>
void
RankTwoTensorTempl<T>::dsymmetricEigenvalues(std::vector<T> & eigvals,
                                             std::vector<RankTwoTensorTempl<T>> & deigvals) const
{
  deigvals.resize(N);

  std::vector<T> a;
  syev("V", eigvals, a);

  // now a contains the eigenvetors
  // extract these and place appropriately in deigvals
  std::vector<T> eig_vec;
  eig_vec.resize(N);

  for (const auto i : make_range(N))
  {
    for (const auto j : make_range(N))
      eig_vec[j] = a[i * N + j];
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        deigvals[i](j, k) = eig_vec[j] * eig_vec[k];
  }

  // There are discontinuities in the derivative
  // for equal eigenvalues.  The following is
  // an attempt to make a sensible choice for
  // the derivative.  This agrees with a central-difference
  // approximation to the derivative.
  if (eigvals[0] == eigvals[1] && eigvals[0] == eigvals[2])
    deigvals[0] = deigvals[1] = deigvals[2] = (deigvals[0] + deigvals[1] + deigvals[2]) / 3.0;
  else if (eigvals[0] == eigvals[1])
    deigvals[0] = deigvals[1] = (deigvals[0] + deigvals[1]) / 2.0;
  else if (eigvals[0] == eigvals[2])
    deigvals[0] = deigvals[2] = (deigvals[0] + deigvals[2]) / 2.0;
  else if (eigvals[1] == eigvals[2])
    deigvals[1] = deigvals[2] = (deigvals[1] + deigvals[2]) / 2.0;
}

template <typename T>
void
RankTwoTensorTempl<T>::d2symmetricEigenvalues(std::vector<RankFourTensorTempl<T>> & deriv) const
{
  std::vector<T> eigvec;
  std::vector<T> eigvals;
  T ev[N][N];

  // reset rank four tensor
  deriv.assign(N, RankFourTensorTempl<T>());

  // get eigen values and eigen vectors
  syev("V", eigvals, eigvec);

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      ev[i][j] = eigvec[i * N + j];

  for (unsigned int alpha = 0; alpha < N; ++alpha)
    for (unsigned int beta = 0; beta < N; ++beta)
    {
      if (eigvals[alpha] == eigvals[beta])
        continue;

      for (const auto i : make_range(N))
        for (const auto j : make_range(N))
          for (const auto k : make_range(N))
            for (const auto l : make_range(N))
            {
              deriv[alpha](i, j, k, l) +=
                  0.5 * (ev[beta][i] * ev[alpha][j] + ev[alpha][i] * ev[beta][j]) *
                  (ev[beta][k] * ev[alpha][l] + ev[beta][l] * ev[alpha][k]) /
                  (eigvals[alpha] - eigvals[beta]);
            }
    }
}

template <typename T>
void
RankTwoTensorTempl<T>::getRUDecompositionRotation(RankTwoTensorTempl<T> &) const
{
  mooseError("getRUDecompositionRotation is only supported for Real valued tensors");
}

template <>
void RankTwoTensor::getRUDecompositionRotation(RankTwoTensor & rot) const;

template <typename T>
void
RankTwoTensorTempl<T>::initRandom(unsigned int rand_seed)
{
  MooseRandom::seed(rand_seed);
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::genRandomTensor(T stddev, T mean)
{
  RankTwoTensorTempl<T> result;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      result(i, j) = (MooseRandom::rand() + mean) * stddev;
  return result;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::genRandomSymmTensor(T stddev, T mean)
{
  auto r = [&]() { return (MooseRandom::rand() + mean) * stddev; };
  return RankTwoTensorTempl<T>(r(), r(), r(), r(), r(), r());
}

template <typename T>
void
RankTwoTensorTempl<T>::vectorOuterProduct(const libMesh::TypeVector<T> & v1,
                                          const libMesh::TypeVector<T> & v2)
{
  RankTwoTensorTempl<T> & a = *this;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      a(i, j) = v1(i) * v2(j);
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::outerProduct(const libMesh::TypeVector<T> & v1,
                                    const libMesh::TypeVector<T> & v2)
{
  RankTwoTensorTempl<T> result;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      result(i, j) = v1(i) * v2(j);
  return result;
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::selfOuterProduct(const libMesh::TypeVector<T> & v)
{
  RankTwoTensorTempl<T> result(RankTwoTensorTempl<T>::initNone);
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      result(i, j) = v(i) * v(j);
  return result;
}

template <typename T>
void
RankTwoTensorTempl<T>::fillRealTensor(libMesh::TensorValue<T> & tensor)
{
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      tensor(i, j) = (*this)(i, j);
}

template <typename T>
void
RankTwoTensorTempl<T>::fillRow(unsigned int r, const libMesh::TypeVector<T> & v)
{
  for (const auto i : make_range(N))
    (*this)(r, i) = v(i);
}

template <typename T>
void
RankTwoTensorTempl<T>::fillColumn(unsigned int c, const libMesh::TypeVector<T> & v)
{
  for (const auto i : make_range(N))
    (*this)(i, c) = v(i);
}

template <typename T>
RankTwoTensorTempl<T>
RankTwoTensorTempl<T>::initialContraction(const RankFourTensorTempl<T> & b) const
{
  RankTwoTensorTempl<T> result;
  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        for (const auto l : make_range(N))
          result(k, l) += (*this)(i, j) * b(i, j, k, l);
  return result;
}

template <typename T>
void
RankTwoTensorTempl<T>::setToIdentity()
{
  mooseAssert(N2 == 9, "RankTwoTensorTempl is currently only tested for 3 dimensions.");
  for (const auto i : make_range(N2))
    _coords[i] = identityCoords[i];
}
