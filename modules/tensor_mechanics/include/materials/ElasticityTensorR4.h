/**
 * ElasticityTensorR4 is designed as a general elastiicty tensor that can handle
 * any symmetry.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 *
 * ElasticityTensorR4 holds the 81 separate C_ijkl entries; the entries are accessed by index, with
 * i, j, k, and l equal to 0, 1, 2
 *
 */

#ifndef ELASTICITYTENSORR4_H
#define ELASTICITYTENSORR4_H

// Any requisite includes here
#include "RankFourTensor.h"

class ElasticityTensorR4 : public RankFourTensor
{
public:


  virtual ~ElasticityTensorR4() {}



  /**
 * calculates the Jacobian of the elastic stiffness tensor.
 */
  virtual Real elasticJacobian( const unsigned int i, const unsigned int k,
                                const RealGradient & grad_test,
                                const RealGradient & grad_phi);

  virtual Real momentJacobian( const unsigned int comp1, const unsigned int comp2,
                                const Real & test,
                                const RealGradient & grad_phi);

  ElasticityTensorR4 & operator=(const ElasticityTensorR4 &a);

  ElasticityTensorR4 operator/(const Real &a);

  ElasticityTensorR4 operator+(const ElasticityTensorR4 &a) const;

  ElasticityTensorR4 operator-(const ElasticityTensorR4 &a) const;

  ElasticityTensorR4 operator - () const;

  ElasticityTensorR4 operator*(const Real &a);

  RankTwoTensor operator*(const RankTwoTensor &a);

  ElasticityTensorR4 operator*(const RankFourTensor &a) const;

 protected:

private:

};

#endif //ELASTICITYTENSORR4_H
