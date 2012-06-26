/**
 * RankTwoTensor is designed to handle the Stress or Strain Tensor for a fully anisotropic material.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen
 *
 * RankTwoTensor holds the 9 separate Sigma_ij or Epsilon_ij entries; the entries are accessed by
 * index, with i, j equal to 1, 2, or 3.
 *  
 */

#ifndef RANKTWOTENSOR_H
#define RANKTWOTENSOR_H

// Any requisite includes here
#include <vector>
#include "libmesh.h"
#include "vector_value.h"

class RankTwoTensor
{
public:

 /**
  * Default constructor; fills to zero
  */
  RankTwoTensor();

  /**
   * Copy constructor
   */
  RankTwoTensor(const RankTwoTensor &a);
  
  ~RankTwoTensor() {}
  
  /**
  * fillFromInputVector takes 6 inputs to fill in the Rank-2 tensor, with the appropriate crystal
  * symmetries maintained.  I.e., S_ij = S_ji
  */
  void fillFromInputVector(const std::vector<Real> input);
  
  /**
  * Sets the value for the index specified.  Takes index = 1,2,3
  */
  void setValue(Real val, int i, int j);

  /**
   * Gets the value for the index specified.  Takes index = 1,2,3
   */
  Real getValue(int i, int j) const;

  Real rowDot(const unsigned int r, const libMesh::TypeVector<Real> & v) const;

  virtual void selfRotate(const Real a1, const Real a2, const Real a3);

  virtual RankTwoTensor rotate(const Real a1, const Real a2, const Real a3) const;

  void setFirstEulerAngle(const Real a1);

  void setSecondEulerAngle(const Real a2);

  void setThirdEulerAngle(const Real a3);

  Real firstEulerAngle() const;

  Real secondEulerAngle() const;

  Real thirdEulerAngle() const;
  

protected:
  
private:

  std::vector<std::vector<Real> > _vals;

  std::vector<Real> _euler_angle;
};

#endif //RANKTWOTENSOR_H
