//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ISOTROPICPOWERLAWHARDENINGSTRESSUPDATE_H
#define ISOTROPICPOWERLAWHARDENINGSTRESSUPDATE_H

#include "IsotropicPlasticityStressUpdate.h"
#include "MooseMesh.h"

class IsotropicPowerLawHardeningStressUpdate;

template <>
InputParameters validParams<IsotropicPowerLawHardeningStressUpdate>();

/**
 * This class uses the Discrete material in a radial return isotropic plasticity
 * model.  This class is one of the basic radial return constitutive models;
 * more complex constitutive models combine creep and plasticity.
 *
 * This class models power law hardening by using the relation
 * \f$ \sigma = \sigma_y + K \epsilon^n \f$
 * where \f$ \sigma_y \f$ is the yield stress. This class solves for the yield
 * stress as the intersection of the power law relation curve and Hooke's law:
 * \f$ \epsilon_y = \frac{\sigma_y}{E} = \left( \frac{\sigma_y}{K} \right)^n \f$
 * where \f$epsilon_y \f$ is the total strain at the yield point and the stress
 * \f$ \sigma_y \f$ is the von Mises stress.
 * Parameters from the parent class, IsotropicPlasticityStressUpdate, are
 * suppressed to enable this class to solve for yield stress:
 * \f$ \sigma_y = \left( \frac{E^n}{K} \right)^{1/(n-1)} \f$
 */
class IsotropicPowerLawHardeningStressUpdate : public IsotropicPlasticityStressUpdate
{
public:
  IsotropicPowerLawHardeningStressUpdate(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(const Real effective_trial_stress,
                                       const RankFourTensor & elasticity_tensor) override;
  virtual void computeYieldStress(const RankFourTensor & elasticity_tensor) override;
  virtual Real computeHardeningDerivative(Real scalar) override;

  ///@{ Power law hardening coefficients
  Real _K;
  Real _strain_hardening_exponent;
  ///@}

  /// Elastic constants
  Real _youngs_modulus;

  ///
  Real _effective_trial_stress;

  Real getIsotropicLameLambda(const RankFourTensor & elasticity_tensor);
};

template <>
InputParameters validParams<IsotropicPowerLawHardeningStressUpdate>();

#endif // ISOTROPICPOWERLAWHARDENINGSTRESSUPDATE_H
