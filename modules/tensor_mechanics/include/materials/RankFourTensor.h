/**
 * RankFourTensor is designed to handle any fourth order tensor.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 *
 * RankFourTensor holds the 81 separate C_ijkl entries; the entries are accessed by index, with
 * i, j, k, and l equal to 0, 1, 2
 *
 */

#ifndef RANKFOURTENSOR_H
#define RANKFOURTENSOR_H

// Any requisite includes here
#include "libmesh/tensor_value.h"
#include <vector>
#include "libmesh/libmesh.h"
#include "libmesh/vector_value.h"
#include "RankTwoTensor.h"
#include "PermutationTensor.h"

class RankFourTensor
{
public:

  /**
   * Default constructor; fills to zero
   */
  RankFourTensor();

  /**
   * Copy constructor
   */
  RankFourTensor(const RankFourTensor &a);

  ~RankFourTensor() {}

  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   */
  Real & operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l);


  /**
   * Gets the value for the index specified.  Takes index = 0,1,2,
   * used for const
   */
  Real operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const;

  /**
   * Sets the value for the index specified
   */
  void setValue(Real val, unsigned int i, unsigned int j, unsigned int k, unsigned int l);

  /**
   * Gets the value for the index specified.  Takes index = 1,2,3
   */
  Real getValue(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const;

  /**
  * Zeros out the tensor.
  */
  void zero();

  RankFourTensor & operator=(const RankFourTensor &a);

  RankTwoTensor operator*(const RankTwoTensor &a);

  RealTensorValue operator*(const RealTensorValue &a);

  RankFourTensor operator*(const Real &a);

  RankFourTensor & operator*=(const Real &a);

  RankFourTensor operator/(const Real &a);

  RankFourTensor & operator/=(const Real &a);

  RankFourTensor & operator+=(const RankFourTensor &a);

  RankFourTensor operator+(const RankFourTensor &a) const;

  RankFourTensor & operator-=(const RankFourTensor &a);

  RankFourTensor operator-(const RankFourTensor &a) const;

  RankFourTensor operator - () const;

  RankFourTensor operator*(const RankFourTensor &a) const;//Added

  RankFourTensor invSymm();//Added

  virtual void rotate(RealTensorValue &R);
  /**
   * Print the tensor
   */
  void print();

  RankFourTensor transposeMajor();


//  int MatrixInversion(double *, int, double* );//Added

  int MatrixInversion(double *, int);//Added

  virtual void surfaceFillFromInputVector(const std::vector<Real> input);

   /**
  * fillSymmetricFromInputVector takes either 21 (all=true) or 9 (all=false) inputs to fill in
  * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
  * C_ijkl = C_ijlk, C_ijkl = C_jikl
  * @param input If all==true then this is
                   C1111 C1122 C1133 C2222 C2233 C3333 C2323 C1313 C1212
                   In the isotropic case this is (la is first Lame constant, mu is second (shear) Lame constant)
                   la+2mu la la la+2mu la la+2mu mu mu mu
                 If all==false then this is
                   C1111 C1122 C1133 C1123 C1113 C1112 C2222 C2233 C2223 C2213 C2212 C3333 C3323 C3313 C3312 C2323 C2313 C2312 C1313 C1312 C1212
  */
  void fillSymmetricFromInputVector(const std::vector<Real> input, bool all);

  /**
   * fillAntisymmetricFromInputVector takes 6 inputs to fill the
   * the antisymmetric Rank-4 tensor with the appropriate symmetries maintained.
   * I.e., B_ijkl = -B_jikl = -B_ijlk = B_klij
   * @param input this is B1212, B1213, B1223, B1313, B1323, B2323.
   */
  void fillAntisymmetricFromInputVector(const std::vector<Real> input);

  /**
   * fillGeneralIsotropicFromInputVector takes 3 inputs to fill the
   * Rank-4 tensor with symmetries C_ijkl = C_klij, and isotropy, ie
   * C_ijkl = la*de_ij*de_kl + mu*(de_ik*de_jl + de_il*de_jk) + a*ep_ijm*ep_klm
   * where la is the first Lame modulus, mu is the second (shear) Lame modulus,
   * and a is the antisymmetric shear modulus, and ep is the permutation tensor
   * @param input this is la, mu, a in the above formula
   */
  void fillGeneralIsotropicFromInputVector(const std::vector<Real> input);

  /**
   * fillAntisymmetricIsotropicFromInputVector takes 1 inputs to fill the
   * the antisymmetric Rank-4 tensor with the appropriate symmetries maintained.
   * I.e., C_ijkl = a * ep_ijm * ep_klm, where epsilon is the permutation tensor (and sum on m)
   * @param input this is a in the above formula
   */
  void fillAntisymmetricIsotropicFromInputVector(const std::vector<Real> input);

  /**
   * fillSymmetricIsotropicFromInputVector takes 2 inputs to fill the
   * the symmetric Rank-4 tensor with the appropriate symmetries maintained.
   * C_ijkl = la*de_ij*de_kl + mu*(de_ik*de_jl + de_il*de_jk)
   * where la is the first Lame modulus, mu is the second (shear) Lame modulus,
   * @param input this is la and mu in the above formula
   */
  void fillSymmetricIsotropicFromInputVector(const std::vector<Real> input);

  /**
   * fillGeneralFromInputVector takes 81 inputs to fill the Rank-4 tensor
   * No symmetries are explicitly maintained
   * @param input  C[i][j][k][l] = input[i*N*N*N + j*N*N + k*N + l]
   */
  void fillGeneralFromInputVector(const std::vector<Real> input);


  /**
  * fillFromInputVector takes some number of inputs to fill
  * the Rank-4 tensor.
  * @param input the numbers that will be placed in the tensor
  * @param fill_method this can be:
               antisymmetric (use fillAntisymmetricFromInputVector)
               symmetric9 (use fillSymmetricFromInputVector with all=false)
               symmetric21 (use fillSymmetricFromInputVector with all=true)
	       general_isotropic (use fillGeneralIsotropicFrominputVector)
	       symmetric_isotropic (use fillSymmetricIsotropicFromInputVector)
	       antisymmetric_isotropic (use fillAntisymmetricIsotropicFromInputVector)
	       general (use fillGeneralFromInputVector)
  */
  void fillFromInputVector(const std::vector<Real> input, std::string fill_method);

protected:

/**
 * Contains the actual data for the Rank Four tensor.
 */
  static const unsigned int N = 3;

  Real _vals[N][N][N][N];

private:

};

#endif //RANKFOURTENSOR_H
