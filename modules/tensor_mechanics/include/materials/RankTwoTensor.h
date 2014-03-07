/**
 * RankTwoTensor is designed to handle the Stress or Strain Tensor for a fully anisotropic material.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M. R. Tonks
 *
 * RankTwoTensor holds the 9 separate Sigma_ij or Epsilon_ij entries; the entries are accessed by
 * index, with i, j equal to 1, 2, or 3.
 *
 */

#ifndef RANKTWOTENSOR_H
#define RANKTWOTENSOR_H

#include "Moose.h"

// Any requisite includes here
#include "libmesh/libmesh.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include <vector>

class RankTwoTensor
{
public:

 /**
  * Default constructor; fills to zero
  */
  RankTwoTensor();

 /**
   * Constructor that takes in 3 vectors and uses them to create rows
   */
  RankTwoTensor(const TypeVector<Real> & row1, const TypeVector<Real> & row2, const TypeVector<Real> & row3);

  /**
   * Copy constructor
   */
  RankTwoTensor(const RankTwoTensor &a);

  /**
   * Copy constructor from RealTensorValue
   */
  RankTwoTensor(const TypeTensor<Real> &a);

  ~RankTwoTensor() {}

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
   *rotates the tensor data given a rank two tensor rotation tensor
   */
  virtual void rotate(RankTwoTensor &R);

  /**
   *rotates the tensor data around the z-axis given an angle in radians
   */
  virtual RankTwoTensor rotateXyPlane(const Real a);

  void zero();

  /**
   * Returns a matrix that is the transpose of the matrix this
   * was called on.
   */
  RankTwoTensor transpose();

  RankTwoTensor & operator= (const RankTwoTensor &a);

  RankTwoTensor & operator+= (const RankTwoTensor &a);

  RankTwoTensor operator+ (const RankTwoTensor &a) const;

  RankTwoTensor & operator-= (const RankTwoTensor &a);

  RankTwoTensor operator- (const RankTwoTensor &a) const;
  /**
   * Return the opposite of a tensor
   */
  RankTwoTensor operator - () const;

  RankTwoTensor & operator*= (const Real &a);

  RankTwoTensor operator* (const Real &a) const;

  RankTwoTensor & operator/= (const Real &a);

  RankTwoTensor operator/ (const Real &a) const;

  RankTwoTensor & operator*= (const RankTwoTensor &a);

  //Defines multiplication with another RankTwoTensor
  RankTwoTensor operator* (const RankTwoTensor &a) const;

  //Defines multiplication with a TypeTensor<Real>
  RankTwoTensor operator* (const TypeTensor<Real> &a) const;

  Real doubleContraction(const RankTwoTensor &a);

  Real trace();

  //Calculate the determine of the tensor
  Real det();

  //Calculate the inverse of the tensor
  RankTwoTensor inverse();

  //Print the rank two tensor
  void print();

  //Add identity times a
  void addIa(const Real &a);

  Real L2norm();
  void surfaceFillFromInputVector(const std::vector<Real> input);

protected:

private:
  static const unsigned int N = 3;

  Real _vals[N][N];
};


#endif //RANKTWOTENSOR_H
