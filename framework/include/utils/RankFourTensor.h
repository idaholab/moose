//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKFOURTENSOR_H
#define RANKFOURTENSOR_H

// MOOSE includes
#include "DataIO.h"

#include "libmesh/tensor_value.h"
#include "libmesh/libmesh.h"
#include "libmesh/vector_value.h"

// Forward declarations
class MooseEnum;
class RankTwoTensor;
class RankFourTensor;

template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<RankFourTensor>(RankFourTensor & v);

/**
 * RankFourTensor is designed to handle any N-dimensional fourth order tensor, C.
 *
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 *
 * Since N is hard-coded to 3, RankFourTensor holds 81 separate C_ijkl entries.
 * Within the code i = 0, 1, 2, but this object provides methods to extract the entries
 * with i = 1, 2, 3, and some of the documentation is also written in this way.
 */
class RankFourTensor
{
public:
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

  /// Default constructor; fills to zero
  RankFourTensor();

  /// Select specific initialization pattern
  RankFourTensor(const InitMethod);

  /// Fill from vector
  RankFourTensor(const std::vector<Real> &, FillMethod);

  // Named constructors
  static RankFourTensor Identity() { return RankFourTensor(initIdentity); }
  static RankFourTensor IdentityFour() { return RankFourTensor(initIdentityFour); };

  /// Gets the value for the index specified.  Takes index = 0,1,2
  inline Real & operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
  {
    return _vals[((i * LIBMESH_DIM + j) * LIBMESH_DIM + k) * LIBMESH_DIM + l];
  }

  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   * used for const
   */
  inline Real operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
  {
    return _vals[((i * LIBMESH_DIM + j) * LIBMESH_DIM + k) * LIBMESH_DIM + l];
  }

  /// Zeros out the tensor.
  void zero();

  /// Print the rank four tensor
  void print(std::ostream & stm = Moose::out) const;

  /// copies values from a into this tensor
  RankFourTensor & operator=(const RankFourTensor & a);

  /// C_ijkl*a_kl
  RankTwoTensor operator*(const RankTwoTensor & a) const;

  /// C_ijkl*a_kl
  RealTensorValue operator*(const RealTensorValue & a) const;

  /// C_ijkl*a
  RankFourTensor operator*(const Real a) const;

  /// C_ijkl *= a
  RankFourTensor & operator*=(const Real a);

  /// C_ijkl/a
  RankFourTensor operator/(const Real a) const;

  /// C_ijkl /= a  for all i, j, k, l
  RankFourTensor & operator/=(const Real a);

  /// C_ijkl += a_ijkl  for all i, j, k, l
  RankFourTensor & operator+=(const RankFourTensor & a);

  /// C_ijkl + a_ijkl
  RankFourTensor operator+(const RankFourTensor & a) const;

  /// C_ijkl -= a_ijkl
  RankFourTensor & operator-=(const RankFourTensor & a);

  /// C_ijkl - a_ijkl
  RankFourTensor operator-(const RankFourTensor & a) const;

  /// -C_ijkl
  RankFourTensor operator-() const;

  /// C_ijpq*a_pqkl
  RankFourTensor operator*(const RankFourTensor & a) const;

  /// sqrt(C_ijkl*C_ijkl)
  Real L2norm() const;

  /**
   * This returns A_ijkl such that C_ijkl*A_klmn = 0.5*(de_im de_jn + de_in de_jm)
   * This routine assumes that C_ijkl = C_jikl = C_ijlk
   */
  RankFourTensor invSymm() const;

  /**
   * Rotate the tensor using
   * C_ijkl = R_im R_in R_ko R_lp C_mnop
   */
  template <class T>
  void rotate(const T & R);

  /**
   * Rotate the tensor using
   * C_ijkl = R_im R_in R_ko R_lp C_mnop
   */
  void rotate(const RealTensorValue & R);

  /**
   * Rotate the tensor using
   * C_ijkl = R_im R_jn R_ko R_lp C_mnop
   */
  void rotate(const RankTwoTensor & R);

  /**
   * Transpose the tensor by swapping the first pair with the second pair of indices
   * @return C_klji
   */
  RankFourTensor transposeMajor() const;

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
  void surfaceFillFromInputVector(const std::vector<Real> & input);

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes some number of inputs to fill
   * the Rank-4 tensor.
   * @param input the numbers that will be placed in the tensor
   * @param fill_method this can be:
   *             antisymmetric (use fillAntisymmetricFromInputVector)
   *             symmetric9 (use fillSymmetricFromInputVector with all=false)
   *             symmetric21 (use fillSymmetricFromInputVector with all=true)
   *             general_isotropic (use fillGeneralIsotropicFrominputVector)
   *             symmetric_isotropic (use fillSymmetricIsotropicFromInputVector)
   *             antisymmetric_isotropic (use fillAntisymmetricIsotropicFromInputVector)
   *             axisymmetric_rz (use fillAxisymmetricRZFromInputVector)
   *             general (use fillGeneralFromInputVector)
   *             principal (use fillPrincipalFromInputVector)
   */
  void fillFromInputVector(const std::vector<Real> & input, FillMethod fill_method);

  ///@{ Vector-less fill API functions. See docs of the corresponding ...FromInputVector methods
  void fillGeneralIsotropic(Real i0, Real i1, Real i2);
  void fillAntisymmetricIsotropic(Real i0);
  void fillSymmetricIsotropic(Real i0, Real i1);
  void fillSymmetricIsotropicEandNu(Real E, Real nu);
  ///@}

  /// Inner product of the major transposed tensor with a rank two tensor
  RankTwoTensor innerProductTranspose(const RankTwoTensor &) const;

  /// Calculates the sum of Ciijj for i and j varying from 0 to 2
  Real sum3x3() const;

  /// Calculates the vector a[i] = sum over j Ciijj for i and j varying from 0 to 2
  RealGradient sum3x1() const;

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
  Real _vals[N4];

  /**
   * fillSymmetricFromInputVector takes either 21 (all=true) or 9 (all=false) inputs to fill in
   * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
   * C_ijkl = C_ijlk, C_ijkl = C_jikl
   * @param input If all==true then this is
   *                C1111 C1122 C1133 C2222 C2233 C3333 C2323 C1313 C1212
   *                In the isotropic case this is (la is first Lame constant, mu is second (shear)
   * Lame constant)
   *                la+2mu la la la+2mu la la+2mu mu mu mu
   *              If all==false then this is
   *                C1111 C1122 C1133 C1123 C1113 C1112 C2222 C2233 C2223 C2213 C2212 C3333 C3323
   * C3313 C3312 C2323 C2313 C2312 C1313 C1312 C1212
   * @param all Determines the compoinents passed in vis the input parameter
   */
  void fillSymmetricFromInputVector(const std::vector<Real> & input, bool all);

  /**
   * fillAntisymmetricFromInputVector takes 6 inputs to fill the
   * the antisymmetric Rank-4 tensor with the appropriate symmetries maintained.
   * I.e., B_ijkl = -B_jikl = -B_ijlk = B_klij
   * @param input this is B1212, B1213, B1223, B1313, B1323, B2323.
   */
  void fillAntisymmetricFromInputVector(const std::vector<Real> & input);

  /**
   * fillGeneralIsotropicFromInputVector takes 3 inputs to fill the
   * Rank-4 tensor with symmetries C_ijkl = C_klij, and isotropy, ie
   * C_ijkl = la*de_ij*de_kl + mu*(de_ik*de_jl + de_il*de_jk) + a*ep_ijm*ep_klm
   * where la is the first Lame modulus, mu is the second (shear) Lame modulus,
   * and a is the antisymmetric shear modulus, and ep is the permutation tensor
   * @param input this is la, mu, a in the above formula
   */
  void fillGeneralIsotropicFromInputVector(const std::vector<Real> & input);

  /**
   * fillAntisymmetricIsotropicFromInputVector takes 1 input to fill the
   * the antisymmetric Rank-4 tensor with the appropriate symmetries maintained.
   * I.e., C_ijkl = a * ep_ijm * ep_klm, where epsilon is the permutation tensor (and sum on m)
   * @param input this is a in the above formula
   */
  void fillAntisymmetricIsotropicFromInputVector(const std::vector<Real> & input);

  /**
   * fillSymmetricIsotropicFromInputVector takes 2 inputs to fill the
   * the symmetric Rank-4 tensor with the appropriate symmetries maintained.
   * C_ijkl = lambda*de_ij*de_kl + mu*(de_ik*de_jl + de_il*de_jk)
   * where lambda is the first Lame modulus, mu is the second (shear) Lame modulus,
   * @param input this is lambda and mu in the above formula
   */
  void fillSymmetricIsotropicFromInputVector(const std::vector<Real> & input);

  /**
   * fillSymmetricIsotropicEandNuFromInputVector is a variation of the
   * fillSymmetricIsotropicFromInputVector which takes as inputs the
   * more commonly used Young's modulus (E) and Poisson's ratio (nu)
   * constants to fill the isotropic elasticity tensor. Using well-known formulas,
   * E and nu are used to calculate lambda and mu and then the vector is passed
   * to fillSymmetricIsotropicFromInputVector.
   * @param input Young's modulus (E) and Poisson's ratio (nu)
   */
  void fillSymmetricIsotropicEandNuFromInputVector(const std::vector<Real> & input);

  /**
   * fillAxisymmetricRZFromInputVector takes 5 inputs to fill the axisymmetric
   * Rank-4 tensor with the appropriate symmetries maintatined for use with
   * axisymmetric problems using coord_type = RZ.
   * I.e. C1111 = C2222, C1133 = C2233, C2323 = C3131 and C1212 = 0.5*(C1111-C1122)
   * @param input this is C1111, C1122, C1133, C3333, C2323.
   */
  void fillAxisymmetricRZFromInputVector(const std::vector<Real> & input);

  /**
   * fillGeneralFromInputVector takes 81 inputs to fill the Rank-4 tensor
   * No symmetries are explicitly maintained
   * @param input  C(i,j,k,l) = input[i*N*N*N + j*N*N + k*N + l]
   */
  void fillGeneralFromInputVector(const std::vector<Real> & input);

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

  void fillPrincipalFromInputVector(const std::vector<Real> & input);
  template <class T>
  friend void dataStore(std::ostream &, T &, void *);

  template <class T>
  friend void dataLoad(std::istream &, T &, void *);

  friend class RankTwoTensor;
  friend class RankThreeTensor;
};

template <>
void dataStore(std::ostream &, RankFourTensor &, void *);

template <>
void dataLoad(std::istream &, RankFourTensor &, void *);

inline RankFourTensor operator*(Real a, const RankFourTensor & b) { return b * a; }

template <class T>
void
RankFourTensor::rotate(const T & R)
{
  RankFourTensor old = *this;

  int index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          Real sum = 0.0;
          int index2 = 0;
          for (unsigned int m = 0; m < N; ++m)
          {
            Real a = R(i, m);
            for (unsigned int n = 0; n < N; ++n)
            {
              Real ab = a * R(j, n);
              for (unsigned int o = 0; o < N; ++o)
              {
                Real abc = ab * R(k, o);
                for (unsigned int p = 0; p < N; ++p)
                  sum += abc * R(l, p) * old._vals[index2++];
              }
            }
          }
          _vals[index++] = sum;
        }
}

#endif // RANKFOURTENSOR_H
