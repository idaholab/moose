//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AnisotropicReturnCreepStressUpdateBase.h"

/**
 * This class uses the stress update material in an anisotropic return isotropic creep
 * model.  This class is one of the basic radial return constitutive models; more complex
 * constitutive models combine creep and plasticity.
 *
 * This class inherits from AnisotropicReturnCreepStressUpdateBase and must be used
 * in conjunction with ComputeMultipleInelasticStress.  This class calculates
 * creep based on stress, temperature, and time effects.  This class also
 * computes the creep strain as a stateful material property.
 *
 * This class extends the usage of PowerLawCreep to transversely isotropic (i.e. anisotropic)
 * cases. For more information, consult, e.g. Stewart et al, "An anisotropic tertiary creep
 * damage constitutive model for anistropic materials", International Journal of Pressure Vessels
 * and Piping 88 (2011) 356--364.
 */

class TransverselyIsotropicCreepStressUpdate : public AnisotropicReturnCreepStressUpdateBase
{
public:
  static InputParameters validParams();

  TransverselyIsotropicCreepStressUpdate(const InputParameters & parameters);

  virtual Real
  computeStrainEnergyRateDensity(const MaterialProperty<RankTwoTensor> & stress,
                                 const MaterialProperty<RankTwoTensor> & strain_rate) override;

protected:
  virtual void computeStressInitialize(const RankTwoTensor & effective_trial_stress,
                                       const RankFourTensor & elasticity_tensor) override;
  virtual Real computeResidual(const RankTwoTensor & effective_trial_stress,
                               const Real scalar) override;
  virtual Real computeDerivative(const RankTwoTensor & effective_trial_stress,
                                 const Real scalar) override;

  /// Flag to determine if temperature is supplied by the user
  const bool _has_temp;

  /// Temperature variable value
  const VariableValue & _temperature;

  /// Leading coefficient
  const Real _coefficient;

  /// Exponent on the effective stress
  const Real _n_exponent;

  /// Exponent on time
  const Real _m_exponent;

  /// Activation energy for exp term
  const Real _activation_energy;

  /// Gas constant for exp term
  const Real _gas_constant;

  /// Simulation start time
  const Real _start_time;

  /// Exponential calculated from activation, gas constant, and temperature
  Real _exponential;

  /// Exponential calculated from current time
  Real _exp_time;

  /// Hill constants for orthotropic creep
  std::vector<Real> _hill_constants;
};
