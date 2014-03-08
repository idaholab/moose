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
  * fillFromInputVector takes either 21 (all=true) or 9 (all=false) inputs to fill in
  * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
  * C_ijkl = C_ijlk, C_ijkl = C_jikl
  */
  void fillFromInputVector(const std::vector<Real> input, bool all);

protected:

/**
 * Contains the actual data for the Rank Four tensor.
 */
  static const unsigned int N = 3;

  Real _vals[N][N][N][N];

private:

};

#endif //RANKFOURTENSOR_H
