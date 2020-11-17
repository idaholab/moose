//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADRadialReturnCreepStressUpdateBase.h"

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
class ADPowerLawCreepStressUpdate : public ADRadialReturnCreepStressUpdateBase
{
public:
  static InputParameters validParams();

  ADPowerLawCreepStressUpdate(const InputParameters & parameters);

  virtual Real
  computeStrainEnergyRateDensity(const ADMaterialProperty<RankTwoTensor> & stress,
                                 const ADMaterialProperty<RankTwoTensor> & strain_rate,
                                 const bool is_incremental,
                                 const MaterialProperty<RankTwoTensor> & strain_rate_old) override;
  Real computeCreepRate(const ADReal & effective_trial_stress);

  virtual bool substeppingCapabilityEnabled() override;

protected:
  virtual void computeStressInitialize(const ADReal & effective_trial_stress,
                                       const ADRankFourTensor & elasticity_tensor) override;
  virtual ADReal computeResidual(const ADReal & effective_trial_stress,
                                 const ADReal & scalar) override;
  virtual ADReal computeDerivative(const ADReal & effective_trial_stress,
                                   const ADReal & scalar) override;

  /// Temperature variable value
  const ADVariableValue * const _temperature;

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
  ADReal _exponential;

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

    Moose::out << "Took this many trapezoidal iterations: " << iteration_number << "\n";
    return interval_1;
  }
};
