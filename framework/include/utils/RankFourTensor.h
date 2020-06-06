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

#include "libmesh/libmesh.h"
#include "libmesh/tuple_of.h"

#include "metaphysicl/raw_type.h"

#include <petscsys.h>

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

namespace MathUtils
{
template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankFourTensor>(RankFourTensor & v);
template <>
void mooseSetToZero<ADRankFourTensor>(ADRankFourTensor & v);
}

/**
 * RankFourTensorTempl is designed to handle any N-dimensional fourth order tensor, C.
 *
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 *
 * Since N is hard-coded to 3, RankFourTensorTempl holds 81 separate C_ijkl entries.
 * Within the code i = 0, 1, 2, but this object provides methods to extract the entries
 * with i = 1, 2, 3, and some of the documentation is also written in this way.
 */
template <typename T>
class RankFourTensorTempl
{
public:
  typedef tuple_of<4, unsigned int> index_type;

  /// Initialization method
  enum InitMethod
  {
    initNone,
    initIdentity,
    initIdentityFour,
    initIdentitySymmetricFour
  };

  /**
   * To fill up the 81 entries in the 4th-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    antisymmetric,
    symmetric9,
    symmetric21,
    general_isotropic,
    symmetric_isotropic,
    symmetric_isotropic_E_nu,
    antisymmetric_isotropic,
    axisymmetric_rz,
    general,
    principal
  };

  template <template <typename> class Tensor, typename Scalar>
  struct TwoTensorMultTraits
  {
    static const bool value = false;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<RankTwoTensorTempl, Scalar>
  {
    static const bool value = ScalarTraits<Scalar>::value;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<TensorValue, Scalar>
  {
    static const bool value = ScalarTraits<Scalar>::value;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<TypeTensor, Scalar>
  {
    static const bool value = ScalarTraits<Scalar>::value;
  };

  /// Default constructor; fills to zero
  RankFourTensorTempl();

  /// Select specific initialization pattern
  RankFourTensorTempl(const InitMethod);

  /// Fill from vector
  RankFourTensorTempl(const std::vector<T> &, FillMethod);

  /// Copy assignment operator must be defined if used
  RankFourTensorTempl(const RankFourTensorTempl<T> & a) = default;

  /**
   * Copy constructor
   */
  template <typename T2>
  RankFourTensorTempl(const RankFourTensorTempl<T2> & copy);

  // Named constructors
  static RankFourTensorTempl<T> Identity() { return RankFourTensorTempl<T>(initIdentity); }
  static RankFourTensorTempl<T> IdentityFour() { return RankFourTensorTempl<T>(initIdentityFour); };

  /// Gets the value for the index specified.  Takes index = 0,1,2
  inline T & operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
  {
    return _vals[((i * LIBMESH_DIM + j) * LIBMESH_DIM + k) * LIBMESH_DIM + l];
  }

  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   * used for const
   */
  inline T operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
  {
    return _vals[((i * LIBMESH_DIM + j) * LIBMESH_DIM + k) * LIBMESH_DIM + l];
  }

  /// Zeros out the tensor.
  void zero();

  /// Print the rank four tensor
  void print(std::ostream & stm = Moose::out) const;

  /// copies values from a into this tensor
  RankFourTensorTempl<T> & operator=(const RankFourTensorTempl<T> & a);

  /**
   * Assignment-from-scalar operator.  Used only to zero out the tensor.
   *
   * \returns A reference to *this.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, RankFourTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p))
  {
    libmesh_assert_equal_to(p, Scalar(0));
    this->zero();
    return *this;
  }

  /// C_ijkl*a_kl
  template <template <typename> class Tensor, typename T2>
  auto operator*(const Tensor<T2> & a) const ->
      typename std::enable_if<TwoTensorMultTraits<Tensor, T2>::value,
                              RankTwoTensorTempl<decltype(T() * T2())>>::type;

  /// C_ijkl*a
  template <typename T2>
  auto operator*(const T2 & a) const ->
      typename std::enable_if<ScalarTraits<T2>::value,
                              RankFourTensorTempl<decltype(T() * T2())>>::type;

  /// C_ijkl *= a
  RankFourTensorTempl<T> & operator*=(const T & a);

  /// C_ijkl/a
  template <typename T2>
  auto operator/(const T2 & a) const ->
      typename std::enable_if<ScalarTraits<T2>::value,
                              RankFourTensorTempl<decltype(T() / T2())>>::type;

  /// C_ijkl /= a  for all i, j, k, l
  RankFourTensorTempl<T> & operator/=(const T & a);

  /// C_ijkl += a_ijkl  for all i, j, k, l
  RankFourTensorTempl<T> & operator+=(const RankFourTensorTempl<T> & a);

  /// C_ijkl + a_ijkl
  template <typename T2>
  auto operator+(const RankFourTensorTempl<T2> & a) const
      -> RankFourTensorTempl<decltype(T() + T2())>;

  /// C_ijkl -= a_ijkl
  RankFourTensorTempl<T> & operator-=(const RankFourTensorTempl<T> & a);

  /// C_ijkl - a_ijkl
  template <typename T2>
  auto operator-(const RankFourTensorTempl<T2> & a) const
      -> RankFourTensorTempl<decltype(T() - T2())>;

  /// -C_ijkl
  RankFourTensorTempl<T> operator-() const;

  /// C_ijpq*a_pqkl
  template <typename T2>
  auto operator*(const RankFourTensorTempl<T2> & a) const
      -> RankFourTensorTempl<decltype(T() * T2())>;

  /// sqrt(C_ijkl*C_ijkl)
  T L2norm() const;

  /**
   * This returns A_ijkl such that C_ijkl*A_klmn = 0.5*(de_im de_jn + de_in de_jm)
   * This routine assumes that C_ijkl = C_jikl = C_ijlk
   */
  RankFourTensorTempl<T> invSymm() const;

  /**
   * Rotate the tensor using
   * C_ijkl = R_im R_in R_ko R_lp C_mnop
   */
  void rotate(const TypeTensor<T> & R);

  /**
   * Transpose the tensor by swapping the first pair with the second pair of indices
   * @return C_klji
   */
  RankFourTensorTempl<T> transposeMajor() const;

  /**
   * Fills the tensor entries ignoring the last dimension (ie, C_ijkl=0 if any of i, j, k, or l =
   * 3).
   * Fill method depends on size of input
   * Input size = 2.  Then C_1111 = C_2222 = input[0], and C_1122 = input[1], and C_1212 = (input[0]
   * - input[1])/2,
   *                  and C_ijkl = C_jikl = C_ijlk = C_klij, and C_1211 = C_1222 = 0.
   * Input size = 9.  Then C_1111 = input[0], C_1112 = input[1], C_1122 = input[3],
   *                       C_1212 = input[4], C_1222 = input[5], C_1211 = input[6]
   *                       C_2211 = input[7], C_2212 = input[8], C_2222 = input[9]
   *                       and C_ijkl = C_jikl = C_ijlk
   */
  void surfaceFillFromInputVector(const std::vector<T> & input);

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes some number of inputs to fill
   * the Rank-4 tensor.
   * @param input the numbers that will be placed in the tensor
   * @param fill_method this can be:
   *             antisymmetric (use fillAntisymmetricFromInputVector)
   *             symmetric9 (use fillSymmetric9FromInputVector)
   *             symmetric21 (use fillSymmetric21FromInputVector)
   *             general_isotropic (use fillGeneralIsotropicFrominputVector)
   *             symmetric_isotropic (use fillSymmetricIsotropicFromInputVector)
   *             antisymmetric_isotropic (use fillAntisymmetricIsotropicFromInputVector)
   *             axisymmetric_rz (use fillAxisymmetricRZFromInputVector)
   *             general (use fillGeneralFromInputVector)
   *             principal (use fillPrincipalFromInputVector)
   */
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method);

  ///@{ Vector-less fill API functions. See docs of the corresponding ...FromInputVector methods
  void fillGeneralIsotropic(T i0, T i1, T i2);
  void fillAntisymmetricIsotropic(T i0);
  void fillSymmetricIsotropic(T i0, T i1);
  void fillSymmetricIsotropicEandNu(T E, T nu);
  ///@}

  /// Inner product of the major transposed tensor with a rank two tensor
  RankTwoTensorTempl<T> innerProductTranspose(const RankTwoTensorTempl<T> &) const;

  /// Calculates the sum of Ciijj for i and j varying from 0 to 2
  T sum3x3() const;

  /// Calculates the vector a[i] = sum over j Ciijj for i and j varying from 0 to 2
  VectorValue<T> sum3x1() const;

  /// checks if the tensor is symmetric
  bool isSymmetric() const;

  /// checks if the tensor is isotropic
  bool isIsotropic() const;

protected:
  /// Dimensionality of rank-four tensor
  static constexpr unsigned int N = LIBMESH_DIM;
  static constexpr unsigned int N2 = N * N;
  static constexpr unsigned int N3 = N * N * N;
  static constexpr unsigned int N4 = N * N * N * N;

  /// The values of the rank-four tensor stored by
  /// index=(((i * LIBMESH_DIM + j) * LIBMESH_DIM + k) * LIBMESH_DIM + l)
  T _vals[N4];

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
  void fillSymmetric9FromInputVector(const std::vector<T> & input);

  /**
   * fillSymmetric21FromInputVector takes either 21 inputs to fill in
   * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
   * C_ijkl = C_ijlk, C_ijkl = C_jikl
   * @param input is
   *                C1111 C1122 C1133 C1123 C1113 C1112 C2222 C2233 C2223 C2213 C2212 C3333 C3323
   * C3313 C3312 C2323 C2313 C2312 C1313 C1312 C1212
   */
  void fillSymmetric21FromInputVector(const std::vector<T> & input);

  /**
   * fillAntisymmetricFromInputVector takes 6 inputs to fill the
   * the antisymmetric Rank-4 tensor with the appropriate symmetries maintained.
   * I.e., B_ijkl = -B_jikl = -B_ijlk = B_klij
   * @param input this is B1212, B1213, B1223, B1313, B1323, B2323.
   */
  void fillAntisymmetricFromInputVector(const std::vector<T> & input);

  /**
   * fillGeneralIsotropicFromInputVector takes 3 inputs to fill the
   * Rank-4 tensor with symmetries C_ijkl = C_klij, and isotropy, ie
   * C_ijkl = la*de_ij*de_kl + mu*(de_ik*de_jl + de_il*de_jk) + a*ep_ijm*ep_klm
   * where la is the first Lame modulus, mu is the second (shear) Lame modulus,
   * and a is the antisymmetric shear modulus, and ep is the permutation tensor
   * @param input this is la, mu, a in the above formula
   */
  void fillGeneralIsotropicFromInputVector(const std::vector<T> & input);

  /**
   * fillAntisymmetricIsotropicFromInputVector takes 1 input to fill the
   * the antisymmetric Rank-4 tensor with the appropriate symmetries maintained.
   * I.e., C_ijkl = a * ep_ijm * ep_klm, where epsilon is the permutation tensor (and sum on m)
   * @param input this is a in the above formula
   */
  void fillAntisymmetricIsotropicFromInputVector(const std::vector<T> & input);

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
   * fillGeneralFromInputVector takes 81 inputs to fill the Rank-4 tensor
   * No symmetries are explicitly maintained
   * @param input  C(i,j,k,l) = input[i*N*N*N + j*N*N + k*N + l]
   */
  void fillGeneralFromInputVector(const std::vector<T> & input);

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
  template <class T2>
  friend void dataStore(std::ostream &, RankFourTensorTempl<T2> &, void *);

  template <class T2>
  friend void dataLoad(std::istream &, RankFourTensorTempl<T2> &, void *);

  template <typename T2>
  friend class RankTwoTensorTempl;
  template <typename T2>
  friend class RankFourTensorTempl;
  template <typename T2>
  friend class RankThreeTensorTempl;
};

namespace MetaPhysicL
{
template <typename T>
struct RawType<RankFourTensorTempl<T>>
{
  typedef RankFourTensorTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const RankFourTensorTempl<T> & in)
  {
    value_type ret;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
        for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
          for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
            ret(i, j, k, l) = raw_value(in(i, j, k, l));

    return ret;
  }
};
}

template <typename T1, typename T2>
inline auto operator*(const T1 & a, const RankFourTensorTempl<T2> & b) ->
    typename std::enable_if<ScalarTraits<T1>::value,
                            RankFourTensorTempl<decltype(T1() * T2())>>::type
{
  return b * a;
}

template <typename T>
template <typename T2>
RankFourTensorTempl<T>::RankFourTensorTempl(const RankFourTensorTempl<T2> & copy)
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] = copy._vals[i];
}

template <typename T>
template <typename T2>
auto RankFourTensorTempl<T>::operator*(const T2 & b) const ->
    typename std::enable_if<ScalarTraits<T2>::value,
                            RankFourTensorTempl<decltype(T() * T2())>>::type
{
  typedef decltype(T() * T2()) ValueType;
  RankFourTensorTempl<ValueType> result;

  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = _vals[i] * b;

  return result;
}

template <typename T>
template <typename T2>
auto
RankFourTensorTempl<T>::operator/(const T2 & b) const ->
    typename std::enable_if<ScalarTraits<T2>::value,
                            RankFourTensorTempl<decltype(T() / T2())>>::type
{
  RankFourTensorTempl<decltype(T() / T2())> result;
  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = _vals[i] / b;
  return result;
}
