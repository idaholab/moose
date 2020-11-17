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
 * in conjunction with ComputeMultipleInelasticStress.  This class calculates
 * creep based on stress, temperature, and time effects.  This class also
 * computes the creep strain as a stateful material property.
 */
class PowerLawCreepStressUpdate : public RadialReturnCreepStressUpdateBase
{
public:
  static InputParameters validParams();

  PowerLawCreepStressUpdate(const InputParameters & parameters);

  virtual Real
  computeStrainEnergyRateDensity(const MaterialProperty<RankTwoTensor> & stress,
                                 const MaterialProperty<RankTwoTensor> & strain_rate,
                                 const bool is_incremental,
                                 const MaterialProperty<RankTwoTensor> & strain_rate_old) override;

  virtual bool substeppingCapabilityEnabled() override;

protected:
  virtual void computeStressInitialize(const Real & effective_trial_stress,
                                       const RankFourTensor & elasticity_tensor) override;
  virtual Real computeCreepRate(const Real effective_trial_stress);
  virtual Real computeResidual(const Real effective_trial_stress, const Real scalar) override;
  virtual Real computeDerivative(const Real effective_trial_stress, const Real scalar) override;

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

  /// Exponential calculated from activiaction, gas constant, and temperature
  Real _exponential;

  /// Exponential calculated from current time
  Real _exp_time;

public:
  virtual Real trapezoidalRule(Real a, Real b, Real tol, std::size_t max_refinements)
  {
    if (a >= b)
      mooseError("Ends of interval do not fulfill requirement b > a in trapezoidalRule");

    Real ya = computeCreepRate(a);
    Real yb = computeCreepRate(b);

    Real h = (b - a) * 0.5;
    Real interval_0 = (ya + yb) * h;
    Real interval_length_0 = (abs(ya) + abs(yb)) * h;

    Real yh = computeCreepRate(a + h);
    Real interval_1 = interval_0 * 0.5 + yh * h;
    Real interval_length_1 = interval_length_0 * 0.5 + abs(yh) * h;

    // The recursion is:
    // I_k = 1/2 I_{k-1} + 1/2^k \sum_{j=1; j odd, j < 2^k} f(a + j(b-a)/2^k)
    std::size_t iteration_number = 2;
    Real error = abs(interval_0 - interval_1);

    while (iteration_number < 8 ||
           (iteration_number < max_refinements && error > tol * interval_length_1))
    {
      interval_0 = interval_1;
      interval_length_0 = interval_length_1;

      interval_1 = interval_0 * 0.5;
      interval_length_1 = interval_length_0 * 0.5;

      std::size_t p = static_cast<std::size_t>(1u) << iteration_number;

      h *= 0.5;
      Real sum = 0;
      Real absum = 0;

      for (std::size_t j = 1; j < p; j += 2)
      {
        Real y = computeCreepRate(a + j * h);
        sum += y;
        absum += abs(y);
      }

      interval_1 += sum * h;
      interval_length_1 += absum * h;
      ++iteration_number;
      error = abs(interval_0 - interval_1);
    }

    return interval_1;
  }
};
