#ifndef ISOTROPICELASTICITYTENSORRZ_H
#define ISOTROPICELASTICITYTENSORRZ_H

#include "IsotropicElasticityTensor.h"

/**
 * Defines an Axisymmetric Isotropic Elasticity Tensor.
 *
 * The input is any two of the following:
 *
 * Youngs Modulus
 * Poissons Ration
 * First and Second Lame Coefficients (lambda and mu)
 * Bulk Modulus
 *
 * Internally this class uses the Lame Coefficients.
 * Everything is is transformed to these coefficients.
 *
 * Note that by default this tensor is _constant_... meaning
 * that it never changes after the first time it is computed.
 *
 * If you want to modify this behavior you can pass in
 * false to the constructor.
 */
class IsotropicElasticityTensorRZ : public IsotropicElasticityTensor
{
public:
  IsotropicElasticityTensorRZ(const bool constant = true);

protected:

  /**
   * Fill in the matrix.
   */
  virtual void calculateEntries(unsigned int qp);

  /**
   * Calculates lambda and mu based on what has been set.
   *
   * These are based on Michael Tonks's's notes
   */
  void calculateLameCoefficients();

};

#endif //ISOTROPICELASTICITYTENSORRZ_H
