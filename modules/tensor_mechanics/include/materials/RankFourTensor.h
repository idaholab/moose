/**
 * RankFourTensor is designed to handle the Stiffness Tensor for a fully anisotropic material.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen
 *
 * RankFourTensor holds the 81 separate C_ijkl entries; the entries are accessed by index, with
 * i, j, k, and l equal to 1, 2, or 3.
 * 
 */

#ifndef RANKFOURTENSOR_H
#define RANKFOURTENSOR_H

// Any requisite includes here
#include "RankTwoTensor.h"
#include <vector>
#include "libmesh.h"
#include "vector_value.h"

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
  * fillFromInputVector takes either 21 (all=true) or 9 (all=false) inputs to fill in
  * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
  * C_ijkl = C_ijlk, C_ijkl = C_jikl
  */
  void fillFromInputVector(const std::vector<Real> input, bool all);
  
  /**
   * Sets the value for the index specified
   */
  void setValue(Real val, int i, int j, int k, int l);

  /**
   * Gets the value for the index specified.  Takes index = 1,2,3
   */
  Real getValue(int i, int j, int k, int l) const;

  /**
  * Zeros out the tensor.
  */
  void zero();

  RankTwoTensor operator*(const RankTwoTensor &a);

  RankFourTensor operator*(const Real &a);

  RankFourTensor & operator+=(const RankFourTensor &a);
  
  RankFourTensor operator+(const RankFourTensor &a) const;

  virtual Real stiffness( const int i, const int j,
                          const RealGradient & t,
                          const RealGradient & p);

  virtual void selfRotate(const Real a1, const Real a2, const Real a3);

  virtual RankFourTensor rotate(const Real a1, const Real a2, const Real a3);

  virtual void setRotationMatrix();

  void setFirstEulerAngle(const Real a1);

  void setSecondEulerAngle(const Real a2);

  void setThirdEulerAngle(const Real a3);

  Real firstEulerAngle() const;

  Real secondEulerAngle() const;

  Real thirdEulerAngle() const;

  RankFourTensor & operator= (const RankFourTensor &a);
  
  
protected:

private:

/**
 * Contains the actual data for the Rank Four tensor.  Takes index= 1,2,3
 */
  std::vector<std::vector<std::vector<std::vector<Real> > > > _vals;

  std::vector<Real> _euler_angle;

  std::vector<std::vector<Real> > _rotation_matrix;

};

#endif //RANKFOURTENSOR_H
