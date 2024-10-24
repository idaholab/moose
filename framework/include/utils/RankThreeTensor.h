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
#include "ADRankTwoTensorForward.h"
#include "ADRankThreeTensorForward.h"
#include "ADRankFourTensorForward.h"

#include "libmesh/libmesh.h"
#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"

namespace libMesh
{
template <typename>
class TensorValue;
template <typename>
class TypeTensor;
template <typename>
class VectorValue;
}

// Forward declarations
class MooseEnum;

namespace MathUtils
{
template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankThreeTensorTempl<Real>>(RankThreeTensorTempl<Real> & v);
template <>
void mooseSetToZero<RankThreeTensorTempl<ADReal>>(RankThreeTensorTempl<ADReal> & v);
}

/**
 * RankThreeTensor is designed to handle any N-dimensional third order tensor, r.
 *
 */
template <typename T>
class RankThreeTensorTempl
{
public:
  ///@{ tensor dimension and powers of the dimension
  static constexpr unsigned int N = Moose::dim;
  static constexpr unsigned int N2 = N * N;
  static constexpr unsigned int N3 = N * N * N;
  ///@}

  /// Initialization method
  enum InitMethod
  {
    initNone
  };

  /**
   * To fill up the 27 entries in the 3rd-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    automatic,
    general,
    plane_normal
  };

  /// Default constructor; fills to zero
  RankThreeTensorTempl();

  /// Copy assignment operator must be defined if used
  RankThreeTensorTempl(const RankThreeTensorTempl<T> & a) = default;

  /**
   * Construct from other class template instantiation
   */
  template <typename T2>
  RankThreeTensorTempl(const RankThreeTensorTempl<T2> & copy);

  /// Select specific initialization pattern
  RankThreeTensorTempl(const InitMethod);

  /// Fill from vector
  RankThreeTensorTempl(const std::vector<T> &, FillMethod method = automatic);

  /// Gets the value for the index specified.  Takes index = 0,1,2
  inline T & operator()(unsigned int i, unsigned int j, unsigned int k)
  {
    return _vals[((i * N + j) * N + k)];
  }

  /// Gets the value for the index specified.  Takes index = 0,1,2. Used for const
  inline T operator()(unsigned int i, unsigned int j, unsigned int k) const
  {
    return _vals[((i * N + j) * N + k)];
  }

  /// Assignment-from-scalar operator.
  RankThreeTensorTempl<T> & operator=(const T & value);

  /// Zeros out the tensor.
  void zero();

  /// Print the rank three tensor
  void print(std::ostream & stm = Moose::out) const;

  /// copies values from "a" into this tensor
  RankThreeTensorTempl<T> & operator=(const RankThreeTensorTempl<T> & a);

  template <typename T2>
  RankThreeTensorTempl<T> & operator=(const RankThreeTensorTempl<T2> & a);

  /// b_i = r_ijk * a_jk
  libMesh::VectorValue<T> operator*(const RankTwoTensorTempl<T> & a) const;

  /// r_ijk*a
  RankThreeTensorTempl<T> operator*(const T a) const;

  /// r_ijk *= a
  RankThreeTensorTempl<T> & operator*=(const T a);

  /// r_ijk/a
  RankThreeTensorTempl<T> operator/(const T a) const;

  /// r_ijk /= a  for all i, j, k
  RankThreeTensorTempl<T> & operator/=(const T a);

  /// r_ijk += a_ijk  for all i, j, k
  RankThreeTensorTempl<T> & operator+=(const RankThreeTensorTempl<T> & a);

  /// r_ijkl + a_ijk
  RankThreeTensorTempl<T> operator+(const RankThreeTensorTempl<T> & a) const;

  /// r_ijk -= a_ijk
  RankThreeTensorTempl<T> & operator-=(const RankThreeTensorTempl<T> & a);

  /// r_ijk - a_ijk
  RankThreeTensorTempl<T> operator-(const RankThreeTensorTempl<T> & a) const;

  /// -r_ijk
  RankThreeTensorTempl<T> operator-() const;

  /// \sqrt(r_ijk*r_ijk)
  T L2norm() const;

  /**
   * Rotate the tensor using
   * r_ijk = R_im R_in R_ko r_mno
   */
  template <class T2>
  void rotate(const T2 & R);

  /**
   * Rotate the tensor using
   * r_ijk = R_im R_in R_ko r_mno
   */
  void rotate(const libMesh::TensorValue<T> & R);

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes some number of inputs to fill
   * the Rank-3 tensor.
   * @param input the numbers that will be placed in the tensor
   * @param fill_method this can be:
   *             general (use fillGeneralFromInputVector)
   *             more fill_methods to be implemented soon!
   */
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method = automatic);

  /**
   * Fills RankThreeTensor from plane normal vectors
   * ref. Kuhl et. al. Int. J. Solids Struct. 38(2001) 2933-2952
   * @param input plane normal vector
   */
  void fillFromPlaneNormal(const libMesh::VectorValue<T> & input);

  /**
   * Creates fourth order tensor D_{ijkl}=A_{mij}*b_{mn}*A_{nkl} where A is rank 3 and b is rank 2
   * @param a RankThreeTensor A in the equation above
   */
  RankFourTensorTempl<T> mixedProductRankFour(const RankTwoTensorTempl<T> & a) const;

  /**
   * Creates a vector from the double contraction of a rank three and rank two tensor.
   *
   * c_i = A_{ijk} * b_{jk}
   */
  libMesh::VectorValue<T> doubleContraction(const RankTwoTensorTempl<T> & b) const;

protected:
  /// The values of the rank-three tensor stored by index=((i * LIBMESH_DIM + j) * LIBMESH_DIM + k)
  T _vals[N3];

  void fillGeneralFromInputVector(const std::vector<T> & input);

  template <class T2>
  friend void dataStore(std::ostream &, RankThreeTensorTempl<T2> &, void *);

  template <class T2>
  friend void dataLoad(std::istream &, RankThreeTensorTempl<T2> &, void *);

  template <class T2>
  friend class RankTwoTensorTempl;

  template <class T2>
  friend class RankThreeTensorTempl;

  template <class T2>
  friend class RankFourTensorTempl;
};

namespace MetaPhysicL
{
template <typename T>
struct RawType<RankThreeTensorTempl<T>>
{
  typedef RankThreeTensorTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const RankThreeTensorTempl<T> & in)
  {
    value_type ret;
    for (const auto i : libMesh::make_range(RankThreeTensorTempl<T>::N))
      for (const auto j : libMesh::make_range(RankThreeTensorTempl<T>::N))
        for (const auto k : libMesh::make_range(RankThreeTensorTempl<T>::N))
          ret(i, j, k) = raw_value(in(i, j, k));

    return ret;
  }
};
}

template <typename T>
template <typename T2>
RankThreeTensorTempl<T>::RankThreeTensorTempl(const RankThreeTensorTempl<T2> & copy)
{
  for (const auto i : libMesh::make_range(N3))
    _vals[i] = copy._vals[i];
}

template <typename T>
template <class T2>
void
RankThreeTensorTempl<T>::rotate(const T2 & R)
{
  unsigned int index = 0;
  for (const auto i : libMesh::make_range(N))
    for (const auto j : libMesh::make_range(N))
      for (const auto k : libMesh::make_range(N))
      {
        unsigned int index2 = 0;
        T sum = 0.0;
        for (const auto m : libMesh::make_range(N))
        {
          T a = R(i, m);
          for (const auto n : libMesh::make_range(N))
          {
            T ab = a * R(j, n);
            for (const auto o : libMesh::make_range(N))
              sum += ab * R(k, o) * _vals[index2++];
          }
        }
        _vals[index++] = sum;
      }
}

template <typename T>
RankThreeTensorTempl<T>
operator*(T a, const RankThreeTensorTempl<T> & b)
{
  return b * a;
}

///r=v*A where r is rank 2, v is vector and A is rank 3
template <typename T>
RankTwoTensorTempl<T>
operator*(const libMesh::VectorValue<T> & p, const RankThreeTensorTempl<T> & b)
{
  static_assert(RankThreeTensorTempl<T>::N == RankTwoTensorTempl<T>::N,
                "RankTwoTensor and RankThreeTensor have to have the same dimension N.");
  RankTwoTensorTempl<T> result;

  for (const auto i : libMesh::make_range(RankThreeTensorTempl<T>::N))
    for (const auto j : libMesh::make_range(RankThreeTensorTempl<T>::N))
      for (const auto k : libMesh::make_range(RankThreeTensorTempl<T>::N))
        result(i, j) += p(k) * b(k, i, j);

  return result;
}

template <typename T>
template <typename T2>
RankThreeTensorTempl<T> &
RankThreeTensorTempl<T>::operator=(const RankThreeTensorTempl<T2> & a)
{
  for (const auto i : libMesh::make_range(N))
    for (const auto j : libMesh::make_range(N))
      for (const auto k : libMesh::make_range(N))
        (*this)(i, j, k) = a(i, j, k);

  return *this;
}
