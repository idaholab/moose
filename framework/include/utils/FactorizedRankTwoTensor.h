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
#include "SymmetricRankTwoTensor.h"

// forward declarations
template <typename>
class FactorizedRankTwoTensorTempl;

/**
 * FactorizedRankTwoTensorTempl is designed to perform the spectral decomposition of an
 * underlying symmetric second order tensor and reuse its bases for future operations if possible.
 *
 * FactorizedRankTwoTensorTempl templates on the underlying second order tensor.
 * IMPORTANT: the underlying second order tensor must be symmetric. A check is only performed in
 * debug mode.
 *
 * Only operations that reuses the known factorization are provided. Otherwise, you will need to
 * first retrieve the underlying RankTwoTensorTempl to perform the operation.
 *
 * TODO? Although I am calling it factorization, it only really refers to eigenvalue decompositon at
 * this point. I am not sure if in the future we need to add other types of similarity
 * transformation.
 */
template <typename T>
class FactorizedRankTwoTensorTempl
{
public:
  /// For generic programming
  typedef typename T::value_type value_type;

  /// No default constructor
  FactorizedRankTwoTensorTempl() = delete;

  /// Copy constructor
  FactorizedRankTwoTensorTempl(const FactorizedRankTwoTensorTempl<T> & A) = default;

  /// Constructor if the factorization isn't known a priori
  FactorizedRankTwoTensorTempl(const T & A)
  {
    if (!A.isSymmetric())
      mooseError("The tensor is not symmetric.");
    A.symmetricEigenvaluesEigenvectors(_eigvals, _eigvecs);
  }

  // Construct from the factorization
  // Assume that regardless of the type of T, the eigenvectors are always stored as as a full second
  // order tensor, e.g. the eigenvector matrix of a symmetric second order tensor isn't symmetric in
  // general. Hence we don't take advantage of orthonormal and/or unitary second order tensors.
  FactorizedRankTwoTensorTempl(const std::vector<typename T::value_type> & eigvals,
                               const RankTwoTensorTempl<typename T::value_type> & eigvecs)
    : _eigvals(eigvals), _eigvecs(eigvecs)
  {
  }

  // @{ getters
  template <typename T2 = T>
  T2 get() const
  {
    return static_cast<T2>(assemble());
  }
  template <typename T2 = T>
  T2 get()
  {
    return static_cast<T2>(assemble());
  }
  const std::vector<typename T::value_type> & eigvals() const { return _eigvals; }
  std::vector<typename T::value_type> eigvals() { return _eigvals; }
  const RankTwoTensorTempl<typename T::value_type> & eigvecs() const { return _eigvecs; }
  RankTwoTensorTempl<typename T::value_type> eigvecs() { return _eigvecs; }
  // @}

  void print(std::ostream & stm = Moose::out) const;

  // Returns _A rotated by R.
  FactorizedRankTwoTensorTempl<T>
  rotated(const RankTwoTensorTempl<typename T::value_type> & R) const;

  /// Returns the transpose of _A.
  FactorizedRankTwoTensorTempl<T> transpose() const;

  // @{ Assignment operators
  FactorizedRankTwoTensorTempl<T> & operator=(const FactorizedRankTwoTensorTempl<T> & A);
  FactorizedRankTwoTensorTempl<T> & operator=(const T & A);
  // @}

  /// performs _A *= a in place, also updates eigen values
  FactorizedRankTwoTensorTempl<T> & operator*=(const typename T::value_type & a);

  /// returns _A * a, also updates eigen values
  template <typename T2>
  FactorizedRankTwoTensorTempl<T> operator*(const T2 & a) const;

  /// performs _A /= a in place, also updates eigen values
  FactorizedRankTwoTensorTempl<T> & operator/=(const typename T::value_type & a);

  /// returns _A / a, also updates eigen values
  template <typename T2>
  FactorizedRankTwoTensorTempl<T> operator/(const T2 & a) const;

  /// Defines logical equality with another second order tensor
  bool operator==(const T & A) const;

  /// Defines logical equality with another FactorizedRankTwoTensorTempl<T>
  bool operator==(const FactorizedRankTwoTensorTempl<T> & A) const;

  /// inverse of _A
  FactorizedRankTwoTensorTempl<T> inverse() const;

  /// add identity times a to _A
  void addIa(const typename T::value_type & a);

  /// trace of _A
  typename T::value_type trace() const;

  /// determinant of _A
  typename T::value_type det() const;

private:
  // Assemble the tensor from the factorization
  RankTwoTensorTempl<typename T::value_type> assemble() const
  {
    typedef RankTwoTensorTempl<typename T::value_type> R2T;
    return _eigvals[0] * R2T::selfOuterProduct(_eigvecs.column(0)) +
           _eigvals[1] * R2T::selfOuterProduct(_eigvecs.column(1)) +
           _eigvals[2] * R2T::selfOuterProduct(_eigvecs.column(2));
  }

  // The eigen values of _A;
  std::vector<typename T::value_type> _eigvals;

  // The eigen vectors of _A;
  RankTwoTensorTempl<typename T::value_type> _eigvecs;
};

namespace MathUtils
{
#define FactorizedRankTwoTensorOperatorMapBody(operator)                                           \
  {                                                                                                \
    std::vector<typename T::value_type> op_eigvals;                                                \
    for (const auto & eigval : A.eigvals())                                                        \
      op_eigvals.push_back(operator);                                                              \
    return FactorizedRankTwoTensorTempl<T>(op_eigvals, A.eigvecs());                               \
  }

#define FactorizedRankTwoTensorOperatorMapUnary(operatorname, operator)                            \
  template <typename T>                                                                            \
  FactorizedRankTwoTensorTempl<T> operatorname(const FactorizedRankTwoTensorTempl<T> & A)          \
  {                                                                                                \
    FactorizedRankTwoTensorOperatorMapBody(operator)                                               \
  }                                                                                                \
  static_assert(true, "")

#define FactorizedRankTwoTensorOperatorMapBinary(operatorname, operator)                           \
  template <typename T, typename T2>                                                               \
  FactorizedRankTwoTensorTempl<T> operatorname(const FactorizedRankTwoTensorTempl<T> & A,          \
                                               const T2 & arg)                                     \
  {                                                                                                \
    if constexpr (libMesh::ScalarTraits<T2>::value)                                                \
    {                                                                                              \
      FactorizedRankTwoTensorOperatorMapBody(operator)                                             \
    }                                                                                              \
  }                                                                                                \
  static_assert(true, "")

#define FactorizedRankTwoTensorOperatorMapDerivativeBody(operator, derivative)                     \
  {                                                                                                \
    std::vector<typename T::value_type> op_eigvals, op_derivs;                                     \
    for (const auto & eigval : A.eigvals())                                                        \
    {                                                                                              \
      op_eigvals.push_back(operator);                                                              \
      op_derivs.push_back(derivative);                                                             \
    }                                                                                              \
                                                                                                   \
    RankFourTensorTempl<typename T::value_type> P, Gab, Gba;                                       \
    typedef RankTwoTensorTempl<typename T::value_type> R2T;                                        \
                                                                                                   \
    for (auto a : make_range(3))                                                                   \
    {                                                                                              \
      const auto Ma = R2T::selfOuterProduct(A.eigvecs().column(a));                                \
      P += op_derivs[a] * Ma.outerProduct(Ma);                                                     \
    }                                                                                              \
                                                                                                   \
    for (auto a : make_range(3))                                                                   \
      for (auto b : make_range(a))                                                                 \
      {                                                                                            \
        const auto Ma = R2T::selfOuterProduct(A.eigvecs().column(a));                              \
        const auto Mb = R2T::selfOuterProduct(A.eigvecs().column(b));                              \
                                                                                                   \
        usingTensorIndices(i_, j_, k_, l_);                                                        \
        Gab = Ma.template times<i_, k_, j_, l_>(Mb) + Ma.template times<i_, l_, j_, k_>(Mb);       \
        Gba = Mb.template times<i_, k_, j_, l_>(Ma) + Mb.template times<i_, l_, j_, k_>(Ma);       \
                                                                                                   \
        Real theta_ab;                                                                             \
        if (!MooseUtils::absoluteFuzzyEqual(A.eigvals()[a], A.eigvals()[b]))                       \
          theta_ab = 0.5 * (op_eigvals[a] - op_eigvals[b]) / (A.eigvals()[a] - A.eigvals()[b]);    \
        else                                                                                       \
          theta_ab = 0.25 * (op_derivs[a] + op_derivs[b]);                                         \
                                                                                                   \
        P += theta_ab * (Gab + Gba);                                                               \
      }                                                                                            \
    return P;                                                                                      \
  }

#define FactorizedRankTwoTensorOperatorMapDerivativeUnary(derivativename, operator, derivative)    \
  template <typename T>                                                                            \
  RankFourTensorTempl<typename T::value_type> derivativename(                                      \
      const FactorizedRankTwoTensorTempl<T> & A)                                                   \
  {                                                                                                \
    FactorizedRankTwoTensorOperatorMapDerivativeBody(operator, derivative)                         \
  }                                                                                                \
  static_assert(true, "")

#define FactorizedRankTwoTensorOperatorMapDerivativeBinary(derivativename, operator, derivative)   \
  template <typename T, typename T2>                                                               \
  RankFourTensorTempl<typename T::value_type> derivativename(                                      \
      const FactorizedRankTwoTensorTempl<T> & A, const T2 & arg)                                   \
  {                                                                                                \
    if constexpr (libMesh::ScalarTraits<T2>::value)                                                \
    {                                                                                              \
      FactorizedRankTwoTensorOperatorMapDerivativeBody(operator, derivative)                       \
    }                                                                                              \
  }                                                                                                \
  static_assert(true, "")

// TODO: While the macros are here, in the future we could instantiate other operator maps like
// trignometry functions.
// @{
FactorizedRankTwoTensorOperatorMapUnary(log, std::log(eigval));
FactorizedRankTwoTensorOperatorMapDerivativeUnary(dlog, std::log(eigval), 1 / eigval);

FactorizedRankTwoTensorOperatorMapUnary(exp, std::exp(eigval));
FactorizedRankTwoTensorOperatorMapDerivativeUnary(dexp, std::exp(eigval), std::exp(eigval));

FactorizedRankTwoTensorOperatorMapUnary(sqrt, std::sqrt(eigval));
FactorizedRankTwoTensorOperatorMapDerivativeUnary(dsqrt,
                                                  std::sqrt(eigval),
                                                  std::pow(eigval, -1. / 2.) / 2.);

FactorizedRankTwoTensorOperatorMapUnary(cbrt, std::cbrt(eigval));
FactorizedRankTwoTensorOperatorMapDerivativeUnary(dcbrt,
                                                  std::cbrt(eigval),
                                                  std::pow(eigval, -2. / 3.) / 3.);

FactorizedRankTwoTensorOperatorMapBinary(pow, std::pow(eigval, arg));
FactorizedRankTwoTensorOperatorMapDerivativeBinary(dpow,
                                                   std::pow(eigval, arg),
                                                   arg * std::pow(eigval, arg - 1));
// @}
} // end namespace MathUtils

template <typename T>
template <typename T2>
FactorizedRankTwoTensorTempl<T>
FactorizedRankTwoTensorTempl<T>::operator*(const T2 & a) const
{
  if constexpr (libMesh::ScalarTraits<T2>::value)
  {
    FactorizedRankTwoTensorTempl<T> A = *this;
    for (auto & eigval : A._eigvals)
      eigval *= a;
    return A;
  }
}

template <typename T>
template <typename T2>
FactorizedRankTwoTensorTempl<T>
FactorizedRankTwoTensorTempl<T>::operator/(const T2 & a) const
{
  if constexpr (libMesh::ScalarTraits<T2>::value)
  {
    FactorizedRankTwoTensorTempl<T> A = *this;
    for (auto & eigval : A._eigvals)
      eigval /= a;
    return A;
  }
}

typedef FactorizedRankTwoTensorTempl<RankTwoTensor> FactorizedRankTwoTensor;
typedef FactorizedRankTwoTensorTempl<ADRankTwoTensor> ADFactorizedRankTwoTensor;
typedef FactorizedRankTwoTensorTempl<SymmetricRankTwoTensor> FactorizedSymmetricRankTwoTensor;
typedef FactorizedRankTwoTensorTempl<ADSymmetricRankTwoTensor> ADFactorizedSymmetricRankTwoTensor;
