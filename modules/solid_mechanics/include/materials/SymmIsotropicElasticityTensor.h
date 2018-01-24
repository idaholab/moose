/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SYMMISOTROPICELASTICITYTENSOR_H
#define SYMMISOTROPICELASTICITYTENSOR_H

#include "SymmElasticityTensor.h"

/**
 * Defines an Isotropic Elasticity Tensor.
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
class SymmIsotropicElasticityTensor : public SymmElasticityTensor
{
public:
  SymmIsotropicElasticityTensor(const bool constant = true);

  virtual ~SymmIsotropicElasticityTensor() {}

  void unsetConstants() { _lambda_set = _mu_set = _E_set = _nu_set = _k_set = false; }

  /**
   * Set the first Lame Coefficient.
   */
  void setLambda(const Real lambda);

  /**
   * Set the second Lame Coefficient.
   */
  void setMu(const Real mu);

  /**
   * Set the Young's Modulus
   */
  void setYoungsModulus(const Real E);

  /**
   * Set Poissons Ratio
   */
  void setPoissonsRatio(const Real nu);

  /**
   * Set the Bulk Modulus
   */
  void setBulkModulus(const Real k);

  /**
   * Set the shear modulus... same thing as Mu
   */
  void setShearModulus(const Real k);

  /**
   * Return Mu
   */
  Real mu() const;

  /**
   * Return the shear modulus... same thing as Mu
   */
  Real shearModulus() const;

  /**
   * Return the youngs  modulus
   */
  Real youngsModulus() const;

  virtual Real stiffness(const unsigned i,
                         const unsigned j,
                         const RealGradient & test,
                         const RealGradient & phi) const;

  virtual void multiply(const SymmTensor & x, SymmTensor & b) const;

  virtual void adjustForCracking(const RealVectorValue & crack_flags);
  virtual void adjustForCrackingWithShearRetention(const RealVectorValue & crack_flags);

protected:
  bool _lambda_set, _mu_set, _E_set, _nu_set, _k_set;

  Real _lambda, _mu, _E, _nu, _k;

  /**
   * Fill in the matrix.
   */
  virtual void calculateEntries(unsigned int qp);
  void setEntries(Real C11, Real C12, Real C44);

  /**
   * Calculates lambda and mu based on what has been set.
   *
   * These are based on Michael Tonks's's notes
   */
  void calculateLameCoefficients();
};

#endif // ISOTROPICELASTICITYTENSOR_H
