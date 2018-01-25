//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ISOTROPICPOWERLAWHARDENING_H
#define ISOTROPICPOWERLAWHARDENING_H

#include "IsotropicPlasticity.h"

class IsotropicPowerLawHardening;

template <>
InputParameters validParams<IsotropicPowerLawHardening>();

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
  virtual void computeStressInitialize(Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor);

  virtual Real computeHardeningDerivative(Real scalar);

  virtual void computeYieldStress();

  Real _youngs_modulus;
  Real _effectiveTrialStress;

  // Coefficients
  Real _K;
  Real _n;

private:
};

#endif // ISOTROPICPOWERLAWHARDENING_H
