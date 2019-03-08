//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKTHREETENSOR_H
#define RANKTHREETENSOR_H

#include "Moose.h"
#include "DualReal.h"

#include "libmesh/libmesh.h"

using libMesh::Real;
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
template <typename>
class RankThreeTensorTempl;

template <typename>
class RankTwoTensorTempl;

template <typename>
class RankFourTensorTempl;

template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankThreeTensorTempl<Real>>(RankThreeTensorTempl<Real> & v);
template <>
void mooseSetToZero<RankThreeTensorTempl<DualReal>>(RankThreeTensorTempl<DualReal> & v);

/**
 * RankThreeTensor is designed to handle any N-dimensional third order tensor, r.
 *
 */
template <typename T>
class RankThreeTensorTempl
{
public:
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
    general
  };

  /// Default constructor; fills to zero
  RankThreeTensorTempl();

  /**
   * Construct from other class template instantiation
   */
  template <typename T2>
  RankThreeTensorTempl(const RankThreeTensorTempl<T2> & copy);

  /// Select specific initialization pattern
  RankThreeTensorTempl(const InitMethod);

  /// Fill from vector
  RankThreeTensorTempl(const std::vector<T> &, FillMethod);

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

  /**
   * Assignment-from-scalar operator.  Used only to zero out the tensor.
   *
   * \returns A reference to *this.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, RankThreeTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p));

  /// Zeros out the tensor.
  void zero();

  /// Print the rank three tensor
  void print(std::ostream & stm = Moose::out) const;

  /// copies values from a into this tensor
  RankThreeTensorTempl<T> & operator=(const RankThreeTensorTempl<T> & a);

  /// b_i = r_ijk * a_jk
  VectorValue<T> operator*(const RankTwoTensorTempl<T> & a) const;

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
  void rotate(const TensorValue<T> & R);

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
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method);

  /**
   * Fills RankThreeTensor from plane normal vectors
   * ref. Kuhl et. al. Int. J. Solids Struct. 38(2001) 2933-2952
   * @param input plane normal vector
   */
  void fillFromPlaneNormal(const VectorValue<T> & input);

  /**
   * Creates fourth order tensor D=A*b*A where A is rank 3 and b is rank 2
   * @param a RankTwoTensor A in the equation above
   */
  RankFourTensorTempl<T> mixedProductRankFour(const RankTwoTensorTempl<T> & a) const;

  /**
   * Creates a vector from the double contraction of a rank three and rank two tensor.
   */
  VectorValue<T> doubleContraction(const RankTwoTensorTempl<T> & b) const;

protected:
  /// Dimensionality of rank-three tensor
  static constexpr unsigned int N = LIBMESH_DIM;
  static constexpr unsigned int N2 = N * N;
  static constexpr unsigned int N3 = N * N * N;

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

typedef RankThreeTensorTempl<Real> RankThreeTensor;
typedef RankThreeTensorTempl<DualReal> DualRankThreeTensor;

template <typename T>
template <typename T2>
RankThreeTensorTempl<T>::RankThreeTensorTempl(const RankThreeTensorTempl<T2> & copy)
{
  for (unsigned int i = 0; i < N3; ++i)
    _vals[i] = copy._vals[i];
}

template <typename T>
template <typename Scalar>
typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, RankThreeTensorTempl<T> &>::type
RankThreeTensorTempl<T>::operator=(const Scalar & libmesh_dbg_var(p))
{
  libmesh_assert_equal_to(p, Scalar(0));
  this->zero();
  return *this;
}

template <typename T>
template <class T2>
void
RankThreeTensorTempl<T>::rotate(const T2 & R)
{
  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
      {
        unsigned int index2 = 0;
        T sum = 0.0;
        for (unsigned int m = 0; m < N; ++m)
        {
          T a = R(i, m);
          for (unsigned int n = 0; n < N; ++n)
          {
            T ab = a * R(j, n);
            for (unsigned int o = 0; o < N; ++o)
              sum += ab * R(k, o) * _vals[index2++];
          }
        }
        _vals[index++] = sum;
      }
}

template <typename T>
RankThreeTensorTempl<T> operator*(T a, const RankThreeTensorTempl<T> & b)
{
  return b * a;
}

///r=v*A where r is rank 2, v is vector and A is rank 3
template <typename T>
RankTwoTensorTempl<T> operator*(const VectorValue<T> & p, const RankThreeTensorTempl<T> & b)
{
  RankTwoTensorTempl<T> result;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        result(i, j) += p(k) * b(k, i, j);

  return result;
}

#endif // RANKTHREETENSOR_H
