//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadialReturnCreepStressUpdateBase.h"

/**
 * This class uses the stress update material in a radial return isotropic creep
 * model.  This class is one of the basic radial return constitutive models; more complex
 * constitutive models combine creep and plasticity.
 *
 * This class inherits from RadialReturnCreepStressUpdateBase and must be used
 * in conjunction with ComputeMultipleInelasticStress. It calculates
 * creep based on stress, temperature, and time effects. Particularly, it allows
 * the user to provide multiple power law parameters which are chosen within this
 * object depending on the von Mises stress level at the quadrature point.
 */
class ADMultiplePowerLawCreepStressUpdate : public ADRadialReturnCreepStressUpdateBase
{
public:
  static InputParameters validParams();

  ADMultiplePowerLawCreepStressUpdate(const InputParameters & parameters);

  virtual Real
  computeStrainEnergyRateDensity(const ADMaterialProperty<RankTwoTensor> & stress,
                                 const ADMaterialProperty<RankTwoTensor> & strain_rate) override;

  virtual bool substeppingCapabilityEnabled() override;

protected:
  virtual void computeStressInitialize(const ADReal & effective_trial_stress,
                                       const ADRankFourTensor & elasticity_tensor) override;
  virtual ADReal computeResidual(const ADReal & effective_trial_stress,
                                 const ADReal & scalar) override;
  virtual ADReal computeDerivative(const ADReal & effective_trial_stress,
                                   const ADReal & scalar) override;

  std::size_t stressIndex(const ADReal & effective_trial_stress);

  /// Temperature variable value
  const ADVariableValue * const _temperature;

  /// Leading coefficient vector
  const std::vector<Real> _coefficient;

  /// Exponent on the effective stress vector
  const std::vector<Real> _n_exponent;

  /// Exponent on time vector
  const std::vector<Real> _m_exponent;

  /// Stress thresholds vector
  const std::vector<Real> _stress_thresholds;

  /// Activation energy for exp term vector
  const std::vector<Real> _activation_energy;

  /// Gas constant for exp term
  const Real _gas_constant;

  /// Simulation start time
  const Real _start_time;

  /// Exponential calculated from activation, gas constant, and temperature
  ADReal _exponential;

  /// Exponential calculated from current time
  Real _exp_time;

  /// Quadrature-point value pointing to the right power law parameters
  unsigned int _stress_index;

  /// Total number of models provided by the user
  const unsigned int _number_of_models;
};
