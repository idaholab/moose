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
#include "tensor_value.h"
#include <vector>
#include "libmesh.h"
#include "vector_value.h"
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
  
  RealTensorValue operator*(const RankTwoTensor &a);
  
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
  
//  int MatrixInversion(double *, int, double* );//Added

  int MatrixInversion(double *, int);//Added
  
  
protected:

/**
 * Contains the actual data for the Rank Four tensor. 
 */
  static const unsigned int N = 3;
  
  Real _vals[N][N][N][N];

private:

};

#endif //RANKFOURTENSOR_H
