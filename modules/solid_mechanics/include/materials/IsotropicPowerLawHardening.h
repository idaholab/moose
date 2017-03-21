/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ISOTROPICPOWERLAWHARDENING_H
#define ISOTROPICPOWERLAWHARDENING_H

#include "IsotropicPlasticity.h"

/**
 * This class creates an Isotropic power law hardening plasticity model.
 * Before yield, stress is youngs modulus* strain. After yield, stress is
 * K*pow(strain, n) where K is the strength coefficient, n is the strain
 * rate exponent and strain is the total strain.
 **/

class IsotropicPowerLawHardening : public IsotropicPlasticity
{
public:
  IsotropicPowerLawHardening(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(unsigned qp,
                                       Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor);

  virtual Real computeHardeningDerivative(unsigned qp, Real scalar);

  virtual void computeYieldStress(unsigned qp);

  Real _youngs_modulus;
  Real _effectiveTrialStress;

  // Coefficients
  Real _K;
  Real _n;

private:
};

template <>
InputParameters validParams<IsotropicPowerLawHardening>();

#endif // ISOTROPICPOWERLAWHARDENING_H
