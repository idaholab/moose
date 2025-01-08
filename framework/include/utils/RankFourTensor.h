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
#include "MooseTypes.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"
#include "ADRankThreeTensorForward.h"
#include "MooseError.h"

#include "libmesh/libmesh.h"
#include "libmesh/tuple_of.h"
#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"

#include <petscsys.h>

#include <Eigen/Core>
#include <Eigen/Dense>

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
 * Since N is hard-coded to 3, RankFourTensorTempl holds 81 separate C_ijkl entries,
 * with i,j,k,l = 0, 1, 2.
 */
template <typename T>
class RankFourTensorTempl
{
public:
  ///@{ tensor dimension and powers of the dimension
  static constexpr unsigned int N = Moose::dim;
  static constexpr unsigned int N2 = N * N;
  static constexpr unsigned int N3 = N * N * N;
  static constexpr unsigned int N4 = N * N * N * N;
  ///@}

  typedef tuple_of<4, unsigned int> index_type;
  typedef T value_type;

  /// Initialization method
  enum InitMethod
  {
    initNone,
    initIdentity,
    initIdentityFour,
    initIdentitySymmetricFour,
    initIdentityDeviatoric
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
    principal,
    orthotropic
  };

  template <template <typename> class Tensor, typename Scalar>
  struct TwoTensorMultTraits
  {
    static const bool value = false;
  };
  template <typename Scalar>
  struct TwoTensorMultTraits<RankTwoTensorTempl, Scalar>
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

  /**
   * The conversion operator from a `SymmetricRankFourTensorTempl`
   */
  template <typename T2>
  RankFourTensorTempl(const SymmetricRankFourTensorTempl<T2> & t)
  {
    for (const auto a : make_range(SymmetricRankFourTensorTempl<T2>::N))
      for (const auto b : make_range(SymmetricRankFourTensorTempl<T2>::N))
      {
        const auto & idx = SymmetricRankFourTensorTempl<T2>::full_index[a][b];
        const auto i = idx[0];
        const auto j = idx[1];
        const auto k = idx[2];
        const auto l = idx[3];

        // Rijkl = Rjikl = Rijlk = Rjilk
        (*this)(i, j, k, l) = t(a, b) / SymmetricRankFourTensorTempl<T2>::mandelFactor(a, b);
        (*this)(j, i, k, l) = t(a, b) / SymmetricRankFourTensorTempl<T2>::mandelFactor(a, b);
        (*this)(i, j, l, k) = t(a, b) / SymmetricRankFourTensorTempl<T2>::mandelFactor(a, b);
        (*this)(j, i, l, k) = t(a, b) / SymmetricRankFourTensorTempl<T2>::mandelFactor(a, b);
      }
  }

  // Named constructors
  static RankFourTensorTempl<T> Identity() { return RankFourTensorTempl<T>(initIdentity); }
  static RankFourTensorTempl<T> IdentityFour() { return RankFourTensorTempl<T>(initIdentityFour); };
  /// Identity of type \delta_{ik} \delta_{jl} - \delta_{ij} \delta_{kl} / 3
  static RankFourTensorTempl<T> IdentityDeviatoric()
  {
    return RankFourTensorTempl<T>(initIdentityDeviatoric);
  };

  /// Gets the value for the indices specified. Takes indices ranging from 0-2 for i, j, k, and l.
  inline T & operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
  {
    return _vals[i * N3 + j * N2 + k * N + l];
  }

  /**
   * Gets the value for the indices specified. Takes indices ranging from 0-2 for i, j, k, and l.
   * used for const
   */
  inline const T & operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
  {
    return _vals[i * N3 + j * N2 + k * N + l];
  }

  /// Zeros out the tensor.
  void zero();

  /// Print the rank four tensor
  void print(std::ostream & stm = Moose::out) const;

  /// Print the values of the rank four tensor
  void printReal(std::ostream & stm = Moose::out) const;

  /// copies values from a into this tensor
  RankFourTensorTempl<T> & operator=(const RankFourTensorTempl<T> & a) = default;

  /**
   * Assignment-from-scalar operator.  Used only to zero out the tensor.
   *
   * \returns A reference to *this.
   */
  template <typename Scalar>
  typename libMesh::boostcopy::enable_if_c<libMesh::ScalarTraits<Scalar>::value,
                                           RankFourTensorTempl &>::type
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
      typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                              RankFourTensorTempl<decltype(T() * T2())>>::type;

  /// C_ijkl *= a
  RankFourTensorTempl<T> & operator*=(const T & a);

  /// C_ijkl/a
  template <typename T2>
  auto operator/(const T2 & a) const ->
      typename std::enable_if<libMesh::ScalarTraits<T2>::value,
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
   * This returns A_ijkl such that C_ijkl*A_klmn = de_im de_jn
   * i.e. the general rank four inverse
   */
  RankFourTensorTempl<T> inverse() const;

  /**
   * Rotate the tensor using
   * C_ijkl = R_im R_jn R_ko R_lp C_mnop
   */
  void rotate(const TypeTensor<T> & R);

  /**
   * Transpose the tensor by swapping the first pair with the second pair of indices
   * @return C_klji
   */
  RankFourTensorTempl<T> transposeMajor() const;

  /**
   * Transpose the tensor by swapping the first two indeces
   * @return C_jikl
   */
  RankFourTensorTempl<T> transposeIj() const;

  /**
   * Transpose the tensor by swapping the last two indeces
   * @return C_ijlk
   */
  RankFourTensorTempl<T> transposeKl() const;

  /**
   * single contraction of a RankFourTensor with a vector over index m
   * @return C_xxx = a_ijkl*b_m where m={i,j,k,l} and xxx the remaining indices
   */
  template <int m>
  RankThreeTensorTempl<T> contraction(const libMesh::VectorValue<T> & b) const;

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
  void fillGeneralIsotropic(const T & i0, const T & i1, const T & i2);
  void fillAntisymmetricIsotropic(const T & i0);
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
   * fillSymmetric21FromInputVector takes either 21 inputs to fill in
   * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
   * C_ijkl = C_ijlk, C_ijkl = C_jikl
   * @param input is
   *                C1111 C1122 C1133 C1123 C1113 C1112 C2222 C2233 C2223 C2213 C2212 C3333 C3323
   * C3313 C3312 C2323 C2313 C2312 C1313 C1312 C1212
   */
  template <typename T2>
  void fillSymmetric21FromInputVector(const T2 & input);

  /// Inner product of the major transposed tensor with a rank two tensor
  RankTwoTensorTempl<T> innerProductTranspose(const RankTwoTensorTempl<T> &) const;

  /// Sum C_ijkl M_kl for a given i,j
  T contractionIj(unsigned int, unsigned int, const RankTwoTensorTempl<T> &) const;

  /// Sum M_ij C_ijkl for a given k,l
  T contractionKl(unsigned int, unsigned int, const RankTwoTensorTempl<T> &) const;

  /// Calculates the sum of Ciijj for i and j varying from 0 to 2
  T sum3x3() const;

  /// Calculates the vector a[i] = sum over j Ciijj for i and j varying from 0 to 2
  libMesh::VectorValue<T> sum3x1() const;

  /// Calculates C_imnt A_jm B_kn C_lt
  RankFourTensorTempl<T> tripleProductJkl(const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &) const;
  /// Calculates C_mjnt A_im B_kn C_lt
  RankFourTensorTempl<T> tripleProductIkl(const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &) const;
  /// Calculates C_mnkt A_im B_jn C_lt
  RankFourTensorTempl<T> tripleProductIjl(const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &) const;
  /// Calculates C_mntl A_im B_jn C_kt
  RankFourTensorTempl<T> tripleProductIjk(const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &,
                                          const RankTwoTensorTempl<T> &) const;

  /// Calculates C_mjkl A_im
  RankFourTensorTempl<T> singleProductI(const RankTwoTensorTempl<T> &) const;
  /// Calculates C_imkl A_jm
  RankFourTensorTempl<T> singleProductJ(const RankTwoTensorTempl<T> &) const;
  /// Calculates C_ijml A_km
  RankFourTensorTempl<T> singleProductK(const RankTwoTensorTempl<T> &) const;
  /// Calculates C_ijkm A_lm
  RankFourTensorTempl<T> singleProductL(const RankTwoTensorTempl<T> &) const;

  /// checks if the tensor is symmetric
  bool isSymmetric() const;

  /// checks if the tensor is isotropic
  bool isIsotropic() const;

protected:
  /// The values of the rank-four tensor stored by
  /// index=(((i * LIBMESH_DIM + j) * LIBMESH_DIM + k) * LIBMESH_DIM + l)
  T _vals[N4];

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

  /**
   * fillGeneralOrhotropicFromInputVector takes 10  inputs to fill the Rank-4 tensor
   * It defines a general orthotropic tensor for which some constraints among
   * elastic parameters exist
   * @param input  Ea, Eb, Ec, Gab, Gbc, Gca, nuba, nuca, nucb, nuab, nuac, nubc
   */
  void fillGeneralOrthotropicFromInputVector(const std::vector<T> & input);

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
    constexpr auto N = RankFourTensorTempl<T>::N;
    value_type ret;
    for (auto i : libMesh::make_range(N))
      for (auto j : libMesh::make_range(N))
        for (auto k : libMesh::make_range(N))
          for (auto l : libMesh::make_range(N))
            ret(i, j, k, l) = raw_value(in(i, j, k, l));

    return ret;
  }
};
}

template <typename T1, typename T2>
inline auto
operator*(const T1 & a, const RankFourTensorTempl<T2> & b) ->
    typename std::enable_if<libMesh::ScalarTraits<T1>::value,
                            RankFourTensorTempl<decltype(T1() * T2())>>::type
{
  return b * a;
}

template <typename T>
template <typename T2>
RankFourTensorTempl<T>::RankFourTensorTempl(const RankFourTensorTempl<T2> & copy)
{
  for (auto i : libMesh::make_range(N4))
    _vals[i] = copy._vals[i];
}

template <typename T>
template <typename T2>
auto
RankFourTensorTempl<T>::operator*(const T2 & b) const ->
    typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                            RankFourTensorTempl<decltype(T() * T2())>>::type
{
  typedef decltype(T() * T2()) ValueType;
  RankFourTensorTempl<ValueType> result;

  for (auto i : libMesh::make_range(N4))
    result._vals[i] = _vals[i] * b;

  return result;
}

template <typename T>
template <typename T2>
auto
RankFourTensorTempl<T>::operator/(const T2 & b) const ->
    typename std::enable_if<libMesh::ScalarTraits<T2>::value,
                            RankFourTensorTempl<decltype(T() / T2())>>::type
{
  RankFourTensorTempl<decltype(T() / T2())> result;
  for (auto i : libMesh::make_range(N4))
    result._vals[i] = _vals[i] / b;
  return result;
}

template <typename T>
template <typename T2>
void
RankFourTensorTempl<T>::fillSymmetric9FromInputVector(const T2 & input)
{
  mooseAssert(input.size() == 9,
              "To use fillSymmetric9FromInputVector, your input must have size 9.");
  zero();

  (*this)(0, 0, 0, 0) = input[0]; // C1111
  (*this)(1, 1, 1, 1) = input[3]; // C2222
  (*this)(2, 2, 2, 2) = input[5]; // C3333

  (*this)(0, 0, 1, 1) = input[1]; // C1122
  (*this)(1, 1, 0, 0) = input[1];

  (*this)(0, 0, 2, 2) = input[2]; // C1133
  (*this)(2, 2, 0, 0) = input[2];

  (*this)(1, 1, 2, 2) = input[4]; // C2233
  (*this)(2, 2, 1, 1) = input[4];

  (*this)(1, 2, 1, 2) = input[6]; // C2323
  (*this)(2, 1, 2, 1) = input[6];
  (*this)(2, 1, 1, 2) = input[6];
  (*this)(1, 2, 2, 1) = input[6];

  (*this)(0, 2, 0, 2) = input[7]; // C1313
  (*this)(2, 0, 2, 0) = input[7];
  (*this)(2, 0, 0, 2) = input[7];
  (*this)(0, 2, 2, 0) = input[7];

  (*this)(0, 1, 0, 1) = input[8]; // C1212
  (*this)(1, 0, 1, 0) = input[8];
  (*this)(1, 0, 0, 1) = input[8];
  (*this)(0, 1, 1, 0) = input[8];
}
template <typename T>
template <typename T2>
void
RankFourTensorTempl<T>::fillSymmetric21FromInputVector(const T2 & input)
{
  mooseAssert(input.size() == 21,
              "To use fillSymmetric21FromInputVector, your input must have size 21.");

  (*this)(0, 0, 0, 0) = input[0];  // C1111
  (*this)(1, 1, 1, 1) = input[6];  // C2222
  (*this)(2, 2, 2, 2) = input[11]; // C3333

  (*this)(0, 0, 1, 1) = input[1]; // C1122
  (*this)(1, 1, 0, 0) = input[1];

  (*this)(0, 0, 2, 2) = input[2]; // C1133
  (*this)(2, 2, 0, 0) = input[2];

  (*this)(1, 1, 2, 2) = input[7]; // C2233
  (*this)(2, 2, 1, 1) = input[7];

  (*this)(0, 0, 0, 2) = input[4]; // C1113
  (*this)(0, 0, 2, 0) = input[4];
  (*this)(0, 2, 0, 0) = input[4];
  (*this)(2, 0, 0, 0) = input[4];

  (*this)(0, 0, 0, 1) = input[5]; // C1112
  (*this)(0, 0, 1, 0) = input[5];
  (*this)(0, 1, 0, 0) = input[5];
  (*this)(1, 0, 0, 0) = input[5];

  (*this)(1, 1, 1, 2) = input[8]; // C2223
  (*this)(1, 1, 2, 1) = input[8];
  (*this)(1, 2, 1, 1) = input[8];
  (*this)(2, 1, 1, 1) = input[8];

  (*this)(1, 1, 1, 0) = input[10];
  (*this)(1, 1, 0, 1) = input[10];
  (*this)(1, 0, 1, 1) = input[10];
  (*this)(0, 1, 1, 1) = input[10]; // C2212 //flipped for filling purposes

  (*this)(2, 2, 2, 1) = input[12];
  (*this)(2, 2, 1, 2) = input[12];
  (*this)(2, 1, 2, 2) = input[12];
  (*this)(1, 2, 2, 2) = input[12]; // C3323 //flipped for filling purposes

  (*this)(2, 2, 2, 0) = input[13];
  (*this)(2, 2, 0, 2) = input[13];
  (*this)(2, 0, 2, 2) = input[13];
  (*this)(0, 2, 2, 2) = input[13]; // C3313 //flipped for filling purposes

  (*this)(0, 0, 1, 2) = input[3]; // C1123
  (*this)(0, 0, 2, 1) = input[3];
  (*this)(1, 2, 0, 0) = input[3];
  (*this)(2, 1, 0, 0) = input[3];

  (*this)(1, 1, 0, 2) = input[9];
  (*this)(1, 1, 2, 0) = input[9];
  (*this)(0, 2, 1, 1) = input[9]; // C2213  //flipped for filling purposes
  (*this)(2, 0, 1, 1) = input[9];

  (*this)(2, 2, 0, 1) = input[14];
  (*this)(2, 2, 1, 0) = input[14];
  (*this)(0, 1, 2, 2) = input[14]; // C3312 //flipped for filling purposes
  (*this)(1, 0, 2, 2) = input[14];

  (*this)(1, 2, 1, 2) = input[15]; // C2323
  (*this)(2, 1, 2, 1) = input[15];
  (*this)(2, 1, 1, 2) = input[15];
  (*this)(1, 2, 2, 1) = input[15];

  (*this)(0, 2, 0, 2) = input[18]; // C1313
  (*this)(2, 0, 2, 0) = input[18];
  (*this)(2, 0, 0, 2) = input[18];
  (*this)(0, 2, 2, 0) = input[18];

  (*this)(0, 1, 0, 1) = input[20]; // C1212
  (*this)(1, 0, 1, 0) = input[20];
  (*this)(1, 0, 0, 1) = input[20];
  (*this)(0, 1, 1, 0) = input[20];

  (*this)(1, 2, 0, 2) = input[16];
  (*this)(0, 2, 1, 2) = input[16]; // C2313 //flipped for filling purposes
  (*this)(2, 1, 0, 2) = input[16];
  (*this)(1, 2, 2, 0) = input[16];
  (*this)(2, 0, 1, 2) = input[16];
  (*this)(0, 2, 2, 1) = input[16];
  (*this)(2, 1, 2, 0) = input[16];
  (*this)(2, 0, 2, 1) = input[16];

  (*this)(1, 2, 0, 1) = input[17];
  (*this)(0, 1, 1, 2) = input[17]; // C2312 //flipped for filling purposes
  (*this)(2, 1, 0, 1) = input[17];
  (*this)(1, 2, 1, 0) = input[17];
  (*this)(1, 0, 1, 2) = input[17];
  (*this)(0, 1, 2, 1) = input[17];
  (*this)(2, 1, 1, 0) = input[17];
  (*this)(1, 0, 2, 1) = input[17];

  (*this)(0, 2, 0, 1) = input[19];
  (*this)(0, 1, 0, 2) = input[19]; // C1312 //flipped for filling purposes
  (*this)(2, 0, 0, 1) = input[19];
  (*this)(0, 2, 1, 0) = input[19];
  (*this)(1, 0, 0, 2) = input[19];
  (*this)(0, 1, 2, 0) = input[19];
  (*this)(2, 0, 1, 0) = input[19];
  (*this)(1, 0, 2, 0) = input[19];
}

template <typename T>
RankFourTensorTempl<T>
RankFourTensorTempl<T>::inverse() const
{

  // The inverse of a 3x3x3x3 in the C_ijkl*A_klmn = de_im de_jn sense is
  // simply the inverse of the 9x9 matrix of the tensor entries.
  // So all we need to do is inverse _vals (with the appropriate row-major
  // storage)

  RankFourTensorTempl<T> result;

  if constexpr (RankFourTensorTempl<T>::N4 * sizeof(T) > EIGEN_STACK_ALLOCATION_LIMIT)
  {
    // Allocate on the heap if you're going to exceed the stack size limit
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> mat(9, 9);
    for (auto i : libMesh::make_range(9 * 9))
      mat(i) = _vals[i];

    mat = mat.inverse();

    for (auto i : libMesh::make_range(9 * 9))
      result._vals[i] = mat(i);
  }
  else
  {
    // Allocate on the stack if small enough
    const Eigen::Map<const Eigen::Matrix<T, 9, 9, Eigen::RowMajor>> mat(&_vals[0]);
    Eigen::Map<Eigen::Matrix<T, 9, 9, Eigen::RowMajor>> res(&result._vals[0]);
    res = mat.inverse();
  }

  return result;
}

template <typename T>
template <int m>
RankThreeTensorTempl<T>
RankFourTensorTempl<T>::contraction(const libMesh::VectorValue<T> & b) const
{
  RankThreeTensorTempl<T> result;
  static constexpr std::size_t z[4][3] = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};
  std::size_t x[4];
  for (x[0] = 0; x[0] < N; ++x[0])
    for (x[1] = 0; x[1] < N; ++x[1])
      for (x[2] = 0; x[2] < N; ++x[2])
        for (x[3] = 0; x[3] < N; ++x[3])
          result(x[z[m][0]], x[z[m][1]], x[z[m][2]]) += (*this)(x[0], x[1], x[2], x[3]) * b(x[m]);

  return result;
}
