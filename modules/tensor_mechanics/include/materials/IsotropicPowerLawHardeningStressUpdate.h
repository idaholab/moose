/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ISOTROPICPOWERLAWHARDENINGSTRESSUPDATE_H
#define ISOTROPICPOWERLAWHARDENINGSTRESSUPDATE_H

#include "IsotropicPlasticityStressUpdate.h"
#include "MooseMesh.h"

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
  virtual void computeStressInitialize(Real effectiveTrialStress) override;
  virtual void computeYieldStress() override;
  virtual Real computeHardeningDerivative(Real scalar) override;

  ///@{ Power law hardening coefficients
  Real _K;
  Real _strain_hardening_exponent;
  ///@}

  /// Elastic constant
  Real _youngs_modulus;

  ///
  Real _effective_trial_stress;

  Real getIsotropicLameLambda();
};

template <>
InputParameters validParams<IsotropicPowerLawHardeningStressUpdate>();

#endif // ISOTROPICPOWERLAWHARDENINGSTRESSUPDATE_H
