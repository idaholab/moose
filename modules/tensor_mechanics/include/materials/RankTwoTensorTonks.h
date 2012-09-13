/**
 * RankTwoTensorTonks is designed to handle the Stress or Strain TensorTonks for a fully anisotropic material.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen
 *
 * RankTwoTensorTonks holds the 9 separate Sigma_ij or Epsilon_ij entries; the entries are accessed by
 * index, with i, j equal to 1, 2, or 3.
 *  
 */

#ifndef RANKTWOTENSORTONKS_H
#define RANKTWOTENSORTONKS_H

// Any requisite includes here
#include <vector>
#include "libmesh.h"
#include "vector_value.h"
#include "tensor_value.h"

class RankTwoTensorTonks
{
public:

 /**
  * Default constructor; fills to zero
  */
  RankTwoTensorTonks();
  
 /**
   * Constructor that takes in 3 vectors and uses them to create rows
   */
  RankTwoTensorTonks(const TypeVector<Real> & row1, const TypeVector<Real> & row2, const TypeVector<Real> & row3);

  /**
   * Copy constructor
   */
  RankTwoTensorTonks(const RankTwoTensorTonks &a);

  /**
   * Copy constructor from RealTensorValue
   */
  RankTwoTensorTonks(const TypeTensor<Real> &a);
  
  ~RankTwoTensorTonks() {}
  
  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   */
  Real & operator()(unsigned int i, unsigned int j);
  /**
   * Gets the value for the index specified.  Takes index = 0,1,2, used for const
   */
  Real operator()(unsigned int i, unsigned int j) const;

  /**
  * fillFromInputVector takes 6 or 9 inputs to fill in the Rank-2 tensor. If 6 inputs, the appropriate crystal
  * symmetries are maintained.  I.e., S_ij = S_ji
  */
  void fillFromInputVector(const std::vector<Real> input);

  /**
  * Sets the value for the index specified.  Takes index = 1,2,3
  */
  void setValue(Real val, unsigned int i, unsigned int j);

  /**
   *@return A Real value for the index specified. Takes index = 1,2,3
   */
  Real getValue(unsigned int i, unsigned int j) const;

  TypeVector<Real> row(const unsigned int r) const;

  /**
   *rotates the tensor data given the rotation tensor
   */
  virtual void rotate(RealTensorValue &R);

  /**
   *rotates the tensor data around the z-axis given an angle in radians
   */
  virtual RankTwoTensorTonks rotateXyPlane(const Real a);
  
  void zero();

  /**
   * Returns a matrix that is the transpose of the matrix this
   * was called on.
   */
  RankTwoTensorTonks transpose();

  RankTwoTensorTonks & operator= (const RankTwoTensorTonks &a);

  RankTwoTensorTonks & operator+= (const RankTwoTensorTonks &a);

  RankTwoTensorTonks operator+ (const RankTwoTensorTonks &a) const;

  RankTwoTensorTonks & operator-= (const RankTwoTensorTonks &a);

  RankTwoTensorTonks operator- (const RankTwoTensorTonks &a) const;
  /**
   * Return the opposite of a tensor
   */
  RankTwoTensorTonks operator - () const;

  RankTwoTensorTonks & operator*= (const Real &a);

  RankTwoTensorTonks operator* (const Real &a) const;

  RankTwoTensorTonks & operator/= (const Real &a);

  RankTwoTensorTonks operator/ (const Real &a) const;
  
  //Defines multiplication with another RankTwoTensor
  RankTwoTensorTonks operator* (const RankTwoTensorTonks &a) const;
  
  //Defines multiplication with a TypeTensor<Real>
  RankTwoTensorTonks operator* (const TypeTensor<Real> &a) const;

  Real doubleContraction(const RankTwoTensorTonks &a);

  Real trace();

  //Calculate the determine of the tensor
  Real det();

  //Calculate the inverse of the tensor
  RankTwoTensorTonks inverse();

protected:
  
private:
  static const unsigned int N = 3;
  
  Real _vals[N][N];
};


#endif //RANKTWOTENSORTONKS_H
