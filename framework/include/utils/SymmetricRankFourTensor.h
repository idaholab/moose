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
#include "ADRankFourTensorForward.h"
#include "ADSymmetricRankTwoTensorForward.h"
#include "ADSymmetricRankFourTensorForward.h"
#include "MooseUtils.h"
#include "MathUtils.h"

#include "libmesh/libmesh.h"
#include "libmesh/tuple_of.h"
#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"

#include <petscsys.h>

// Eigen includes
#include <Eigen/Core>
#include <Eigen/Dense>

#include <array>

using libMesh::Real;
using libMesh::tuple_of;
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

namespace boostcopy = libMesh::boostcopy;

namespace MathUtils
{
/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<SymmetricRankFourTensor>(SymmetricRankFourTensor & v);
template <>
void mooseSetToZero<ADSymmetricRankFourTensor>(ADSymmetricRankFourTensor & v);
}

/**
 * SymmetricRankFourTensorTempl is designed to handle an N-dimensional fourth order tensor with
 * minor symmetry, C. Since N is hard-coded to 3, SymmetricRankFourTensorTempl holds 36 separate
 * C_ij entries. Within the code i,j = 0, .., 5.
 */
template <typename T>
class SymmetricRankFourTensorTempl
{
public:
  ///@{ tensor dimension, Mandel matrix dimension, and Mandel matrix size
  static constexpr unsigned int Ndim = LIBMESH_DIM;
  static constexpr unsigned int N = Ndim + Ndim * (Ndim - 1) / 2;
  static constexpr unsigned int N2 = N * N;
  ///@}

  // Full tensor indices in the Mandel/Voigt representation
  static constexpr unsigned int full_index[6][6][4] = {
      {{0, 0, 0, 0}, {0, 0, 1, 1}, {0, 0, 2, 2}, {0, 0, 1, 2}, {0, 0, 0, 2}, {0, 0, 0, 1}},
      {{1, 1, 0, 0}, {1, 1, 1, 1}, {1, 1, 2, 2}, {1, 1, 1, 2}, {1, 1, 0, 2}, {1, 1, 0, 1}},
      {{2, 2, 0, 0}, {2, 2, 1, 1}, {2, 2, 2, 2}, {2, 2, 1, 2}, {2, 2, 0, 2}, {2, 2, 0, 1}},
      {{1, 2, 0, 0}, {1, 2, 1, 1}, {1, 2, 2, 2}, {1, 2, 1, 2}, {1, 2, 0, 2}, {1, 2, 0, 1}},
      {{0, 2, 0, 0}, {0, 2, 1, 1}, {0, 2, 2, 2}, {0, 2, 1, 2}, {0, 2, 0, 2}, {0, 2, 0, 1}},
      {{0, 1, 0, 0}, {0, 1, 1, 1}, {0, 1, 2, 2}, {0, 1, 1, 2}, {0, 1, 0, 2}, {0, 1, 0, 1}}};

  /// returns the 1, sqrt(2), or 2 prefactor in the Mandel notation for the indices i,j ranging from 0-5.
  static constexpr Real mandelFactor(unsigned int i, unsigned int j)
  {
    return SymmetricRankTwoTensorTempl<T>::mandelFactor(i) *
           SymmetricRankTwoTensorTempl<T>::mandelFactor(j);
  }

  /// Initialization method
  enum InitMethod
  {
    initNone,
    initIdentity,
    initIdentitySymmetricFour
  };

  /**
   * To fill up the 36 entries in the 4th-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    symmetric9,               // fillSymmetric9FromInputVector
    symmetric21,              // fillSymmetric21FromInputVector
    symmetric_isotropic,      // fillSymmetricIsotropicFromInputVector
    symmetric_isotropic_E_nu, // fillSymmetricIsotropicEandNu
    axisymmetric_rz,          // fillAxisymmetricRZFromInputVector
    principal,                // fillPrincipalFromInputVector
    orthotropic               // fillGeneralOrthotropicFromInputVector
  };

  template <template <typename> class Tensor, typename Scalar>
  struct TwoTensorMultTraits
  {
    static const bool value = false;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<SymmetricRankTwoTensorTempl, Scalar>
  {
    static const bool value = libMesh::ScalarTraits<Scalar>::value;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<TensorValue, Scalar>
  {
    static const bool value = libMesh::ScalarTraits<Scalar>::value;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<TypeTensor, Scalar>
  {
    static const bool value = libMesh::ScalarTraits<Scalar>::value;
  };

  /// Default constructor; fills to zero
  SymmetricRankFourTensorTempl();

  /// Select specific initialization pattern
  SymmetricRankFourTensorTempl(const InitMethod);

  /// Fill from vector
  SymmetricRankFourTensorTempl(const std::vector<T> &, FillMethod);

  /// Copy assignment operator must be defined if used
  SymmetricRankFourTensorTempl(const SymmetricRankFourTensorTempl<T> & a) = default;

  /**
   * Copy constructor
   */
  template <typename T2>
  SymmetricRankFourTensorTempl(const SymmetricRankFourTensorTempl<T2> & copy);

  /// Copy constructor from RankFourTensorTempl<T>
  explicit SymmetricRankFourTensorTempl(const RankFourTensorTempl<T> & a);

  /// The conversion operator to `RankFourTensorTempl`
  explicit operator RankFourTensorTempl<T>();

  // Named constructors
  static SymmetricRankFourTensorTempl<T> identity()
  {
    return SymmetricRankFourTensorTempl<T>(initIdentity);
  }
  static SymmetricRankFourTensorTempl<T> identitySymmetricFour()
  {
    return SymmetricRankFourTensorTempl<T>(initIdentitySymmetricFour);
  };

  /// Gets the value for the indices specified. Takes indices ranging from 0-5 for i and j.
  inline T & operator()(unsigned int i, unsigned int j) { return _vals[i * N + j]; }

  /**
   * Gets the value for the indices specified. Takes indices ranging from 0-5 for i and j.
   * used for const
   */
  inline const T & operator()(unsigned int i, unsigned int j) const { return _vals[i * N + j]; }

  /// Zeros out the tensor.
  void zero();

  /// Print the rank four tensor
  void print(std::ostream & stm = Moose::out) const;

  /// Print the values of the rank four tensor
  void printReal(std::ostream & stm = Moose::out) const;

  /// copies values from a into this tensor
  SymmetricRankFourTensorTempl<T> & operator=(const SymmetricRankFourTensorTempl<T> & a) = default;

  /**
   * Assignment-from-scalar operator.  Used only to zero out the tensor.
   *
   * \returns A reference to *this.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<libMesh::ScalarTraits<Scalar>::value,
                                  SymmetricRankFourTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p))
  {
    libmesh_assert_equal_to(p, Scalar(0));
    this->zero();
    return *this;
  }

  /// C_ijkl*a_kl
  template <typename T2>
  auto operator*(const SymmetricRankTwoTensorTempl<T2> & b) const
      -> SymmetricRankTwoTensorTempl<decltype(T() * T2())>;

  /// C_ijkl*a
  template <typename T2>
  auto operator*(const T2 & a) const ->
      typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                              SymmetricRankFourTensorTempl<decltype(T() * T2())>>::type;

  /// C_ijkl *= a
  SymmetricRankFourTensorTempl<T> & operator*=(const T & a);

  /// C_ijkl/a
  template <typename T2>
  auto operator/(const T2 & a) const ->
      typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                              SymmetricRankFourTensorTempl<decltype(T() / T2())>>::type;

  /// C_ijkl /= a  for all i, j, k, l
  SymmetricRankFourTensorTempl<T> & operator/=(const T & a);

  /// C_ijkl += a_ijkl  for all i, j, k, l
  SymmetricRankFourTensorTempl<T> & operator+=(const SymmetricRankFourTensorTempl<T> & a);

  /// C_ijkl + a_ijkl
  template <typename T2>
  auto operator+(const SymmetricRankFourTensorTempl<T2> & a) const
      -> SymmetricRankFourTensorTempl<decltype(T() + T2())>;

  /// C_ijkl -= a_ijkl
  SymmetricRankFourTensorTempl<T> & operator-=(const SymmetricRankFourTensorTempl<T> & a);

  /// C_ijkl - a_ijkl
  template <typename T2>
  auto operator-(const SymmetricRankFourTensorTempl<T2> & a) const
      -> SymmetricRankFourTensorTempl<decltype(T() - T2())>;

  /// -C_ijkl
  SymmetricRankFourTensorTempl<T> operator-() const;

  /// C_ijpq*a_pqkl
  template <typename T2>
  auto operator*(const SymmetricRankFourTensorTempl<T2> & a) const
      -> SymmetricRankFourTensorTempl<decltype(T() * T2())>;

  /// sqrt(C_ijkl*C_ijkl)
  T L2norm() const;

  /**
   * This returns A_ijkl such that C_ijkl*A_klmn = 0.5*(de_im de_jn + de_in de_jm)
   * This routine assumes that C_ijkl = C_jikl = C_ijlk
   */
  SymmetricRankFourTensorTempl<T> invSymm() const;

  /**
   * Build a 6x6 rotation matrix
   * MEHRABADI, MORTEZA M.; COWIN, STEPHEN C.  (1990). EIGENTENSORS OF LINEAR ANISOTROPIC ELASTIC
   * MATERIALS. The Quarterly Journal of Mechanics and Applied Mathematics, 43(1), 15-41.
   * doi:10.1093/qjmam/43.1.15
   */
  static SymmetricRankFourTensorTempl<T> rotationMatrix(const TypeTensor<T> & R);

  /**
   * Rotate the tensor using
   * C_ijkl = R_im R_jn R_ko R_lp C_mnop
   */
  void rotate(const TypeTensor<T> & R);

  /**
   * Transpose the tensor by swapping the first pair with the second pair of indices
   * This amounts to a regular transpose of the 6x6 matrix
   * @return C_klji
   */
  SymmetricRankFourTensorTempl<T> transposeMajor() const;

  /**
   * Transpose the tensor by swapping the first two indices - a no-op
   */
  SymmetricRankFourTensorTempl<T> transposeIj() const { return *this; }

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes some number of inputs to fill
   * the Rank-4 tensor.
   * @param input the numbers that will be placed in the tensor
   * @param fill_method See FillMethod
   */
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method);

  ///@{ Vector-less fill API functions. See docs of the corresponding ...FromInputVector methods
  void fillSymmetricIsotropic(const T & i0, const T & i1);
  void fillSymmetricIsotropicEandNu(const T & E, const T & nu);
  ///@}

  /**
   * fillSymmetric9FromInputVector takes 9 inputs to fill in
   * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
   * C_ijkl = C_ijlk, C_ijkl = C_jikl
   * @param input is:
   *                C1111 C1122 C1133 C2222 C2233 C3333 C2323 C1313 C1212
   *                In the isotropic case this is (la is first Lame constant, mu is second (shear)
   * Lame constant)
   *                la+2mu la la la+2mu la la+2mu mu mu mu
   */
  template <typename T2>
  void fillSymmetric9FromInputVector(const T2 & input);

  /**
   * fillSymmetric21FromInputVector takes 21 inputs to fill in the Rank-4 tensor with the
   * appropriate crystal symmetries maintained.
   * I.e., C_ijkl = C_klij, C_ijkl = C_ijlk, C_ijkl = C_jikl
   *
   * @param input is C1111 C1122 C1133 C1123 C1113 C1112 C2222 C2233 C2223 C2213 C2212 C3333 C3323
   * C3313 C3312 C2323 C2313 C2312 C1313 C1312 C1212
   */
  template <typename T2>
  void fillSymmetric21FromInputVector(const T2 & input);

  /// Calculates the sum of Ciijj for i and j varying from 0 to 2
  T sum3x3() const;

  /// Calculates the vector a[i] = sum over j Ciijj for i and j varying from 0 to 2
  libMesh::VectorValue<T> sum3x1() const;

  /// checks if the tensor is symmetric
  bool isSymmetric() const;

  /// checks if the tensor is isotropic
  bool isIsotropic() const;

protected:
  /// The values of the rank-four tensor
  std::array<T, N2> _vals;

  /**
   * fillSymmetricIsotropicFromInputVector takes 2 inputs to fill the
   * the symmetric Rank-4 tensor with the appropriate symmetries maintained.
   * C_ijkl = lambda*de_ij*de_kl + mu*(de_ik*de_jl + de_il*de_jk)
   * where lambda is the first Lame modulus, mu is the second (shear) Lame modulus,
   * @param input this is lambda and mu in the above formula
   */
  void fillSymmetricIsotropicFromInputVector(const std::vector<T> & input);

  /**
   * fillSymmetricIsotropicEandNuFromInputVector is a variation of the
   * fillSymmetricIsotropicFromInputVector which takes as inputs the
   * more commonly used Young's modulus (E) and Poisson's ratio (nu)
   * constants to fill the isotropic elasticity tensor. Using well-known formulas,
   * E and nu are used to calculate lambda and mu and then the vector is passed
   * to fillSymmetricIsotropicFromInputVector.
   * @param input Young's modulus (E) and Poisson's ratio (nu)
   */
  void fillSymmetricIsotropicEandNuFromInputVector(const std::vector<T> & input);

  /**
   * fillAxisymmetricRZFromInputVector takes 5 inputs to fill the axisymmetric
   * Rank-4 tensor with the appropriate symmetries maintatined for use with
   * axisymmetric problems using coord_type = RZ.
   * I.e. C1111 = C2222, C1133 = C2233, C2323 = C3131 and C1212 = 0.5*(C1111-C1122)
   * @param input this is C1111, C1122, C1133, C3333, C2323.
   */
  void fillAxisymmetricRZFromInputVector(const std::vector<T> & input);

  /**
   * fillPrincipalFromInputVector takes 9 inputs to fill a Rank-4 tensor
   * C1111 = input0
   * C1122 = input1
   * C1133 = input2
   * C2211 = input3
   * C2222 = input4
   * C2233 = input5
   * C3311 = input6
   * C3322 = input7
   * C3333 = input8
   * with all other components being zero
   */

  void fillPrincipalFromInputVector(const std::vector<T> & input);

  /**
   * fillGeneralOrhotropicFromInputVector takes 10  inputs to fill the Rank-4 tensor
   * It defines a general orthotropic tensor for which some constraints among
   * elastic parameters exist
   * @param input  Ea, Eb, Ec, Gab, Gbc, Gca, nuba, nuca, nucb, nuab, nuac, nubc
   */
  void fillGeneralOrthotropicFromInputVector(const std::vector<T> & input);

  template <class T2>
  friend void dataStore(std::ostream &, SymmetricRankFourTensorTempl<T2> &, void *);

  template <class T2>
  friend void dataLoad(std::istream &, SymmetricRankFourTensorTempl<T2> &, void *);

  template <typename T2>
  friend class SymmetricRankTwoTensorTempl;
  template <typename T2>
  friend class SymmetricRankFourTensorTempl;
  template <typename T2>
  friend class RankThreeTensorTempl;
};

namespace MetaPhysicL
{
template <typename T>
struct RawType<SymmetricRankFourTensorTempl<T>>
{
  typedef SymmetricRankFourTensorTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const SymmetricRankFourTensorTempl<T> & in)
  {
    value_type ret;
    for (unsigned int i = 0; i < SymmetricRankFourTensorTempl<T>::N; ++i)
      for (unsigned int j = 0; j < SymmetricRankFourTensorTempl<T>::N; ++j)
        ret(i, j) = raw_value(in(i, j));

    return ret;
  }
};
}

template <typename T1, typename T2>
inline auto
operator*(const T1 & a, const SymmetricRankFourTensorTempl<T2> & b) ->
    typename std::enable_if<libMesh::ScalarTraits<T1>::value,
                            SymmetricRankFourTensorTempl<decltype(T1() * T2())>>::type
{
  return b * a;
}

template <typename T>
template <typename T2>
SymmetricRankFourTensorTempl<T>::SymmetricRankFourTensorTempl(
    const SymmetricRankFourTensorTempl<T2> & copy)
{
  for (const auto i : make_range(N2))
    _vals[i] = copy._vals[i];
}

template <typename T>
template <typename T2>
auto
SymmetricRankFourTensorTempl<T>::operator*(const T2 & b) const ->
    typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                            SymmetricRankFourTensorTempl<decltype(T() * T2())>>::type
{
  typedef decltype(T() * T2()) ValueType;
  SymmetricRankFourTensorTempl<ValueType> result;

  for (const auto i : make_range(N2))
    result._vals[i] = _vals[i] * b;

  return result;
}

template <typename T>
template <typename T2>
auto
SymmetricRankFourTensorTempl<T>::operator*(const SymmetricRankTwoTensorTempl<T2> & b) const
    -> SymmetricRankTwoTensorTempl<decltype(T() * T2())>
{
  typedef decltype(T() * T2()) ValueType;
  SymmetricRankTwoTensorTempl<ValueType> result;

  std::size_t index = 0;
  for (const auto i : make_range(N))
  {
    ValueType tmp = 0.0;
    for (const auto j : make_range(N))
      tmp += _vals[index++] * b._vals[j];
    result._vals[i] = tmp;
  }

  return result;
}

template <typename T>
template <typename T2>
auto
SymmetricRankFourTensorTempl<T>::operator/(const T2 & b) const ->
    typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                            SymmetricRankFourTensorTempl<decltype(T() / T2())>>::type
{
  SymmetricRankFourTensorTempl<decltype(T() / T2())> result;
  for (const auto i : make_range(N2))
    result._vals[i] = _vals[i] / b;
  return result;
}

template <typename T>
template <typename T2>
void
SymmetricRankFourTensorTempl<T>::fillSymmetric9FromInputVector(const T2 & input)
{
  mooseAssert(LIBMESH_DIM == 3, "This method assumes LIBMESH_DIM == 3");
  mooseAssert(input.size() == 9,
              "To use fillSymmetric9FromInputVector, your input must have size 9.");
  zero();

  _vals[0] = input[0];  // C1111
  _vals[7] = input[3];  // C2222
  _vals[14] = input[5]; // C3333

  _vals[1] = _vals[6] = input[1];  // C1122
  _vals[2] = _vals[12] = input[2]; // C1133
  _vals[8] = _vals[13] = input[4]; // C2233

  static constexpr std::size_t C2323 = 21;
  static constexpr std::size_t C1313 = 28;
  static constexpr std::size_t C1212 = 35;

  _vals[C2323] = 2.0 * input[6];
  _vals[C1313] = 2.0 * input[7];
  _vals[C1212] = 2.0 * input[8];
}

template <typename T>
template <typename T2>
void
SymmetricRankFourTensorTempl<T>::fillSymmetric21FromInputVector(const T2 & input)
{
  // C1111 C1122 C1133 C1123 C1113 C1112
  //       C2222 C2233 C2223 C2213 C2212
  //             C3333 C3323 C3313 C3312
  //                   C2323 C2313 C2312
  //                         C1313 C1312
  //                               C1212

  mooseAssert(LIBMESH_DIM == 3, "This method assumes LIBMESH_DIM == 3");
  mooseAssert(input.size() == 21,
              "To use fillSymmetric21FromInputVector, your input must have size 21.");
  std::size_t index = 0;
  for (const auto i : make_range(N))
    for (const auto j : make_range(i, N))
    {
      _vals[i + N * j] = mandelFactor(i, j) * input[index];
      _vals[j + N * i] = mandelFactor(j, i) * input[index];
      index++;
    }
}

template <typename T>
SymmetricRankFourTensorTempl<T>
SymmetricRankFourTensorTempl<T>::invSymm() const
{
  SymmetricRankFourTensorTempl<T> result(initNone);

  if constexpr (SymmetricRankFourTensorTempl<T>::N2 * sizeof(T) > EIGEN_STACK_ALLOCATION_LIMIT)
  {
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> mat(N, N);
    for (const auto i : make_range(N))
      for (const auto j : make_range(N))
        mat(i, j) = (*this)(i, j);
    mat = mat.inverse();
    for (const auto i : make_range(N))
      for (const auto j : make_range(N))
        result(i, j) = mat(i, j);
  }
  else
  {
    const Eigen::Map<const Eigen::Matrix<T, N, N, Eigen::RowMajor>> mat(&_vals[0]);
    Eigen::Map<Eigen::Matrix<T, N, N, Eigen::RowMajor>> res(&result._vals[0]);
    res = mat.inverse();
  }

  return result;
}
