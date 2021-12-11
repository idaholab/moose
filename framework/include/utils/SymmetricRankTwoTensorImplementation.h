//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetricRankTwoTensor.h"

// MOOSE includes
#include "MooseEnum.h"
#include "ColumnMajorMatrix.h"
#include "MooseRandom.h"
#include "SymmetricRankFourTensor.h"
#include "Conversion.h"
#include "MooseArray.h"
#include "MathUtils.h"

#include "libmesh/libmesh.h"
#include "libmesh/vector_value.h"
#include "libmesh/utility.h"

// PETSc includes
#include <petscblaslapack.h>

// C++ includes/sqrt
#include <iomanip>
#include <ostream>
#include <vector>
#include <array>

template <typename T>
constexpr std::array<Real, SymmetricRankTwoTensorTempl<T>::N>
    SymmetricRankTwoTensorTempl<T>::identityCoords;

namespace MathUtils
{
template <>
void mooseSetToZero<SymmetricRankTwoTensor>(SymmetricRankTwoTensor & v);
template <>
void mooseSetToZero<ADSymmetricRankTwoTensor>(ADSymmetricRankTwoTensor & v);
}

template <typename T>
MooseEnum
SymmetricRankTwoTensorTempl<T>::fillMethodEnum()
{
  return MooseEnum("autodetect=0 isotropic1=1 diagonal3=3 symmetric6=6", "autodetect");
}

template <typename T>
SymmetricRankTwoTensorTempl<T>::SymmetricRankTwoTensorTempl()
{
  std::fill(_vals.begin(), _vals.end(), 0.0);
}

template <typename T>
SymmetricRankTwoTensorTempl<T>::SymmetricRankTwoTensorTempl(const InitMethod init)
{
  switch (init)
  {
    case initNone:
      break;

    case initIdentity:
      setToIdentity();
      break;

    default:
      mooseError("Unknown SymmetricRankTwoTensorTempl initialization pattern.");
  }
}

template <typename T>
SymmetricRankTwoTensorTempl<T>::SymmetricRankTwoTensorTempl(
    const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12)
{
  _vals[0] = S11;
  _vals[1] = S22;
  _vals[2] = S33;
  _vals[3] = MathUtils::sqrt2 * S23;
  _vals[4] = MathUtils::sqrt2 * S13;
  _vals[5] = MathUtils::sqrt2 * S12;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>::operator RankTwoTensorTempl<T>()
{
  return RankTwoTensorTempl<T>(_vals[0],
                               _vals[5] / MathUtils::sqrt2,
                               _vals[4] / MathUtils::sqrt2,
                               _vals[5] / MathUtils::sqrt2,
                               _vals[1],
                               _vals[3] / MathUtils::sqrt2,
                               _vals[4] / MathUtils::sqrt2,
                               _vals[3] / MathUtils::sqrt2,
                               _vals[2]);
}

template <typename T>
SymmetricRankTwoTensorTempl<T>::SymmetricRankTwoTensorTempl(const T & S11,
                                                            const T & S21,
                                                            const T & S31,
                                                            const T & S12,
                                                            const T & S22,
                                                            const T & S32,
                                                            const T & S13,
                                                            const T & S23,
                                                            const T & S33)
{
  _vals[0] = S11;
  _vals[1] = S22;
  _vals[2] = S33;
  _vals[3] = (S23 + S32) / MathUtils::sqrt2;
  _vals[4] = (S13 + S31) / MathUtils::sqrt2;
  _vals[5] = (S12 + S21) / MathUtils::sqrt2;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::fromRawComponents(
    const T & S11, const T & S22, const T & S33, const T & S23, const T & S13, const T & S12)
{
  SymmetricRankTwoTensorTempl<T> ret(SymmetricRankTwoTensorTempl<T>::initNone);
  ret._vals[0] = S11;
  ret._vals[1] = S22;
  ret._vals[2] = S33;
  ret._vals[3] = S23;
  ret._vals[4] = S13;
  ret._vals[5] = S12;
  return ret;
}

/// named constructor for initializing symmetrically
template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::initializeSymmetric(const TypeVector<T> & v0,
                                                    const TypeVector<T> & v1,
                                                    const TypeVector<T> & v2)
{
  return SymmetricRankTwoTensorTempl<T>(
      v0(0), v1(0), v2(0), v0(1), v1(1), v2(1), v0(2), v1(2), v2(2));
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::fillFromInputVector(const std::vector<T> & input,
                                                    FillMethod fill_method)
{
  if (fill_method != autodetect && fill_method != input.size())
    mooseError("Expected an input vector size of ",
               fill_method,
               " to fill the SymmetricRankTwoTensorTempl");

  switch (input.size())
  {
    case 1:
      _vals[0] = input[0];
      _vals[1] = input[0];
      _vals[2] = input[0];
      _vals[3] = 0.0;
      _vals[4] = 0.0;
      _vals[5] = 0.0;
      break;

    case 3:
      _vals[0] = input[0];
      _vals[1] = input[1];
      _vals[2] = input[2];
      _vals[3] = 0.0;
      _vals[4] = 0.0;
      _vals[5] = 0.0;
      break;

    case 6:
      _vals[0] = input[0];
      _vals[1] = input[1];
      _vals[2] = input[2];
      _vals[3] = MathUtils::sqrt2 * input[3];
      _vals[4] = MathUtils::sqrt2 * input[4];
      _vals[5] = MathUtils::sqrt2 * input[5];
      break;

    default:
      mooseError("Please check the number of entries in the input vector for building "
                 "a SymmetricRankTwoTensorTempl. It must be 1, 3, 6");
  }
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::fillFromScalarVariable(const VariableValue & scalar_variable)
{
  switch (scalar_variable.size())
  {
    case 1:
      _vals[0] = scalar_variable[0];
      _vals[1] = 0.0;
      _vals[2] = 0.0;
      _vals[3] = 0.0;
      _vals[4] = 0.0;
      _vals[5] = 0.0;
      break;

    case 3:
      _vals[0] = scalar_variable[0];
      _vals[1] = scalar_variable[1];
      _vals[2] = 0.0;
      _vals[3] = 0.0;
      _vals[4] = 0.0;
      _vals[5] = MathUtils::sqrt2 * scalar_variable[2];
      break;

    case 6:
      _vals[0] = scalar_variable[0];
      _vals[1] = scalar_variable[1];
      _vals[2] = scalar_variable[2];
      _vals[3] = MathUtils::sqrt2 * scalar_variable[3];
      _vals[4] = MathUtils::sqrt2 * scalar_variable[4];
      _vals[5] = MathUtils::sqrt2 * scalar_variable[5];
      break;

    default:
      mooseError("Only FIRST, THIRD, or SIXTH order scalar variable can be used to build "
                 "a SymmetricRankTwoTensorTempl.");
  }
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::rotate(const TypeTensor<T> & R)
{
  auto M = SymmetricRankFourTensorTempl<T>::rotationMatrix(R);
  *this = M * (*this);
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::transpose() const
{
  return *this;
}

/// multiply vector v with row n of this tensor
template <typename T>
T
SymmetricRankTwoTensorTempl<T>::rowMultiply(std::size_t n, const TypeVector<T> & v) const
{
  switch (n)
  {
    case 0:
      return _vals[0] * v(0) + _vals[5] / MathUtils::sqrt2 * v(1) +
             _vals[4] / MathUtils::sqrt2 * v(2);
    case 1:
      return _vals[5] / MathUtils::sqrt2 * v(0) + _vals[1] * v(1) +
             _vals[3] / MathUtils::sqrt2 * v(2);
    case 2:
      return _vals[4] / MathUtils::sqrt2 * v(0) + _vals[3] / MathUtils::sqrt2 * v(1) +
             _vals[2] * v(2);
    default:
      mooseError("Invalid row");
  }
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::timesTranspose(const RankTwoTensorTempl<T> & a)
{
  return SymmetricRankTwoTensorTempl<T>(a(0, 0) * a(0, 0) + a(0, 1) * a(0, 1) + a(0, 2) * a(0, 2),
                                        a(1, 0) * a(1, 0) + a(1, 1) * a(1, 1) + a(1, 2) * a(1, 2),
                                        a(2, 0) * a(2, 0) + a(2, 1) * a(2, 1) + a(2, 2) * a(2, 2),
                                        a(1, 0) * a(2, 0) + a(1, 1) * a(2, 1) + a(1, 2) * a(2, 2),
                                        a(0, 0) * a(2, 0) + a(0, 1) * a(2, 1) + a(0, 2) * a(2, 2),
                                        a(0, 0) * a(1, 0) + a(0, 1) * a(1, 1) + a(0, 2) * a(1, 2));
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::timesTranspose(const SymmetricRankTwoTensorTempl<T> & a)
{
  return SymmetricRankTwoTensorTempl<T>::fromRawComponents(
      a._vals[0] * a._vals[0] + a._vals[4] * a._vals[4] / 2.0 + a._vals[5] * a._vals[5] / 2.0,
      a._vals[1] * a._vals[1] + a._vals[3] * a._vals[3] / 2.0 + a._vals[5] * a._vals[5] / 2.0,
      a._vals[2] * a._vals[2] + a._vals[3] * a._vals[3] / 2.0 + a._vals[4] * a._vals[4] / 2.0,
      a._vals[1] * a._vals[3] + a._vals[2] * a._vals[3] +
          a._vals[4] * a._vals[5] / MathUtils::sqrt2,
      a._vals[0] * a._vals[4] + a._vals[2] * a._vals[4] +
          a._vals[3] * a._vals[5] / MathUtils::sqrt2,
      a._vals[0] * a._vals[5] + a._vals[1] * a._vals[5] +
          a._vals[3] * a._vals[4] / MathUtils::sqrt2);
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::plusTranspose(const RankTwoTensorTempl<T> & a)
{
  return SymmetricRankTwoTensorTempl<T>(
             a(0, 0), a(0, 1), a(0, 2), a(1, 0), a(1, 1), a(1, 2), a(2, 0), a(2, 1), a(2, 2)) *
         2.0;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::plusTranspose(const SymmetricRankTwoTensorTempl<T> & a)
{
  return a * 2.0;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::sqr() const
{
  return SymmetricRankTwoTensorTempl<T>::timesTranspose(*this);
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::zero()
{
  for (std::size_t i = 0; i < N; ++i)
    _vals[i] = 0.0;
}

template <typename T>
SymmetricRankTwoTensorTempl<T> &
SymmetricRankTwoTensorTempl<T>::operator=(const SymmetricRankTwoTensorTempl<T> & a)
{
  std::copy(a._vals.begin(), a._vals.end(), _vals.begin());
  return *this;
}

template <typename T>
SymmetricRankTwoTensorTempl<T> &
SymmetricRankTwoTensorTempl<T>::operator+=(const SymmetricRankTwoTensorTempl<T> & a)
{
  for (std::size_t i = 0; i < N; ++i)
    _vals[i] += a._vals[i];
  return *this;
}

template <typename T>
SymmetricRankTwoTensorTempl<T> &
SymmetricRankTwoTensorTempl<T>::operator-=(const SymmetricRankTwoTensorTempl<T> & a)
{
  for (std::size_t i = 0; i < N; ++i)
    _vals[i] -= a._vals[i];
  return *this;
}

template <typename T>
template <typename T2>
SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
SymmetricRankTwoTensorTempl<T>::operator+(const SymmetricRankTwoTensorTempl<T2> & a) const
{
  SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype> result;
  for (std::size_t i = 0; i < N; ++i)
    result(i) = _vals[i] + a(i);
  return result;
}

template <typename T>
template <typename T2>
SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
SymmetricRankTwoTensorTempl<T>::operator-(const SymmetricRankTwoTensorTempl<T2> & a) const
{
  SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype> result(
      SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>::initNone);
  for (std::size_t i = 0; i < N; ++i)
    result(i) = _vals[i] - a(i);
  return result;
}

/// returns -_vals
template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::operator-() const
{
  return (*this) * -1.0;
}

template <typename T>
SymmetricRankTwoTensorTempl<T> &
SymmetricRankTwoTensorTempl<T>::operator*=(const T & a)
{
  for (std::size_t i = 0; i < N; ++i)
    _vals[i] *= a;
  return *this;
}

template <typename T>
SymmetricRankTwoTensorTempl<T> &
SymmetricRankTwoTensorTempl<T>::operator/=(const T & a)
{
  for (std::size_t i = 0; i < N; ++i)
    _vals[i] /= a;
  return *this;
}

template <typename T>
template <typename T2>
bool
SymmetricRankTwoTensorTempl<T>::operator==(const SymmetricRankTwoTensorTempl<T2> & a) const
{
  for (std::size_t i = 0; i < N; ++i)
    if (_vals[i] != a._vals[i])
      return false;
  return true;
}

template <typename T>
template <typename T2>
bool
SymmetricRankTwoTensorTempl<T>::operator!=(const SymmetricRankTwoTensorTempl<T2> & a) const
{
  for (std::size_t i = 0; i < N; ++i)
    if (_vals[i] != a._vals[i])
      return true;
  return false;
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::doubleContraction(const SymmetricRankTwoTensorTempl<T> & b) const
{
  T sum = 0;
  for (unsigned int i = 0; i < N; ++i)
    sum += _vals[i] * b._vals[i];
  return sum;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::outerProduct(const SymmetricRankTwoTensorTempl<T> & b) const
{
  SymmetricRankFourTensorTempl<T> result;
  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    const T & a = _vals[i];
    for (unsigned int j = 0; j < N; ++j)
      result._vals[index++] = a * b._vals[j];
  }
  return result;
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::mixedProductIkJl(const SymmetricRankTwoTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::mixedProductIlJk(const SymmetricRankTwoTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::mixedProductJkIl(const SymmetricRankTwoTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::mixedProductIjJklm(
    const SymmetricRankFourTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::mixedProductJmIjkl(
    const SymmetricRankFourTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::mixedProductJkIjlm(
    const SymmetricRankFourTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::deviatoric() const
{
  SymmetricRankTwoTensorTempl<T> deviatoric(*this);
  deviatoric.addIa(-1.0 / 3.0 * this->tr());
  return deviatoric;
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::generalSecondInvariant() const
{
  mooseError("Not implemented");
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::secondInvariant() const
{
  T result;
  result = Utility::pow<2>(_vals[0] - _vals[1]) / 6.0;
  result += Utility::pow<2>(_vals[0] - _vals[2]) / 6.0;
  result += Utility::pow<2>(_vals[1] - _vals[2]) / 6.0;
  result += Utility::pow<2>(MathUtils::sqrt2 * _vals[5]) / 4.0;
  result += Utility::pow<2>(MathUtils::sqrt2 * _vals[4]) / 4.0;
  result += Utility::pow<2>(MathUtils::sqrt2 * _vals[3]) / 4.0;
  return result;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::dsecondInvariant() const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::d2secondInvariant() const
{
  mooseError("Not implemented");
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::trace() const
{
  return _vals[0] + _vals[1] + _vals[2];
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::inverse() const
{
  const auto d = det();
  if (d == 0.0)
    mooseException("Matrix not invertible");
  const SymmetricRankTwoTensorTempl<T> inv(
      _vals[2] * _vals[1] - _vals[3] * _vals[3] / 2.0,
      _vals[2] * _vals[0] - _vals[4] * _vals[4] / 2.0,
      _vals[0] * _vals[1] - _vals[5] * _vals[5] / 2.0,
      _vals[5] * _vals[4] / 2.0 - _vals[0] * _vals[3] / MathUtils::sqrt2,
      _vals[5] * _vals[3] / 2.0 - _vals[4] * _vals[1] / MathUtils::sqrt2,
      _vals[4] * _vals[3] / 2.0 - _vals[2] * _vals[5] / MathUtils::sqrt2);
  return inv * 1.0 / d;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::dtrace() const
{
  return SymmetricRankTwoTensorTempl<T>(1, 1, 1, 0, 0, 0);
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::thirdInvariant() const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::dthirdInvariant() const
{
  mooseError("Not implemented");
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::d2thirdInvariant() const
{
  mooseError("Not implemented");
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::det() const
{
  return _vals[0] * (_vals[2] * _vals[1] - _vals[3] * _vals[3] / 2.0) -
         _vals[5] / MathUtils::sqrt2 *
             (_vals[2] * _vals[5] / MathUtils::sqrt2 - _vals[3] * _vals[4] / 2.0) +
         _vals[4] / MathUtils::sqrt2 *
             (_vals[3] * _vals[5] / 2.0 - _vals[1] * _vals[4] / MathUtils::sqrt2);
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::ddet() const
{
  return SymmetricRankTwoTensorTempl<T>(
      _vals[2] * _vals[1] - _vals[3] * _vals[3] / 2.0,
      _vals[0] * _vals[2] - _vals[4] * _vals[4] / 2.0,
      _vals[0] * _vals[1] - _vals[5] * _vals[5] / 2.0,
      _vals[4] * _vals[5] / 2.0 - _vals[0] * _vals[3] / MathUtils::sqrt2,
      _vals[5] * _vals[3] / 2.0 - _vals[4] * _vals[1] / MathUtils::sqrt2,
      _vals[4] * _vals[3] / 2.0 - _vals[5] * _vals[2] / MathUtils::sqrt2);
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::print(std::ostream & stm) const
{
  for (std::size_t i = 0; i < N; ++i)
    stm << std::setw(15) << _vals[i] << std::endl;
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::printReal(std::ostream & stm) const
{
  for (std::size_t i = 0; i < N; ++i)
    stm << std::setw(15) << MetaPhysicL::raw_value(_vals[i]) << std::endl;
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::addIa(const T & a)
{
  for (unsigned int i = 0; i < 3; ++i)
    _vals[i] += a;
}

template <typename T>
T
SymmetricRankTwoTensorTempl<T>::L2norm() const
{
  T norm = _vals[0] * _vals[0];
  for (unsigned int i = 1; i < N; ++i)
    norm += _vals[i] * _vals[i];
  return norm == 0.0 ? 0.0 : std::sqrt(norm);
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::syev(const char *, std::vector<T> &, std::vector<T> &) const
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::symmetricEigenvalues(std::vector<T> & eigvals) const
{
  SymmetricRankTwoTensorTempl<T> a;
  symmetricEigenvaluesEigenvectors(eigvals, a);
}

template <>
void
SymmetricRankTwoTensor::symmetricEigenvalues(std::vector<Real> & eigvals) const
{
  std::vector<Real> a;
  syev("N", eigvals, a);
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::symmetricEigenvaluesEigenvectors(
    std::vector<T> &, SymmetricRankTwoTensorTempl<T> &) const
{
  mooseError(
      "symmetricEigenvaluesEigenvectors is only available for ordered tensor component types");
}

template <>
void
SymmetricRankTwoTensor::symmetricEigenvaluesEigenvectors(std::vector<Real> & /*eigvals*/,
                                                         SymmetricRankTwoTensor & /*eigvecs*/) const
{
  mooseError("Not implemented");
}

template <>
void
ADSymmetricRankTwoTensor::symmetricEigenvaluesEigenvectors(
    std::vector<DualReal> & /*eigvals*/, ADSymmetricRankTwoTensor & /*eigvecs*/) const
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::dsymmetricEigenvalues(
    std::vector<T> & /*eigvals*/, std::vector<SymmetricRankTwoTensorTempl<T>> & /*deigvals*/) const
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::d2symmetricEigenvalues(
    std::vector<SymmetricRankFourTensorTempl<T>> & /*deriv*/) const
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::getRUDecompositionRotation(SymmetricRankTwoTensorTempl<T> &) const
{
  mooseError("Not implemented");
}

template <>
void
SymmetricRankTwoTensor::getRUDecompositionRotation(SymmetricRankTwoTensor & /*rot*/) const
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::initRandom(unsigned int rand_seed)
{
  MooseRandom::seed(rand_seed);
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::genRandomSymmTensor(T scale, T offset)
{
  SymmetricRankTwoTensorTempl<T> ret;
  for (unsigned int i = 0; i < 3; ++i)
    ret._vals[i] = (MooseRandom::rand() + offset) * scale;
  for (unsigned int i = 3; i < 6; ++i)
    ret._vals[i] = MathUtils::sqrt2 * (MooseRandom::rand() + offset) * scale;
  return ret;
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::vectorSelfOuterProduct(const TypeVector<T> & v)
{
  return SymmetricRankTwoTensorTempl<T>(
      v(0) * v(0), v(1) * v(1), v(2) * v(2), v(1) * v(2), v(0) * v(2), v(0) * v(1));
}

template <typename T>
SymmetricRankTwoTensorTempl<T>
SymmetricRankTwoTensorTempl<T>::initialContraction(
    const SymmetricRankFourTensorTempl<T> & /*b*/) const
{
  mooseError("Not implemented");
}

template <typename T>
void
SymmetricRankTwoTensorTempl<T>::setToIdentity()
{
  std::copy(identityCoords.begin(), identityCoords.end(), _vals.begin());
}
