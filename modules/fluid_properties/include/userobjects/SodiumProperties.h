//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluidProperties.h"

/**
 * Properties of liquid sodium from ANL/RE-95/2 report "Thermodynamic and Transport Properties of
 * Sodium Liquid and Vapor" from ANL Reactor Engineering Division.
 */
class SodiumProperties : public FluidProperties
{

public:
  static InputParameters validParams();

  SodiumProperties(const InputParameters & parameters);

  /**
   * Thermal conductivity as a function of temperature. From page 181. Valid from 371 to 1500 K.
   * @param T temperature in K
   * @return Thermal conductivity of liquid sodium in W/m/K
   */
  template <typename T>
  T k(T temperature) const
  {
    if (_k)
      return _k;

    const T temperature2 = temperature * temperature;
    const T temperature3 = temperature2 * temperature;
    return 124.67 - 0.11381 * temperature + 5.5226e-5 * temperature2 - 1.1842e-8 * temperature3;
  }

  /**
   * Enthalpy of liquid Na (relative to solid Na at STP) in J/kg as a function of temperature
   * From page 4. Valid from 371 to 2000 K, and relative to enthalpy of solid Na at 298.15.
   * @param T temperature in K
   * @return enthalpy of liquid sodium in J/kg
   */
  template <typename T>
  T h(T temperature) const
  {
    const T temperature2 = temperature * temperature;
    const T temperature3 = temperature2 * temperature;

    // Converted from kJ/kg to J/kg.
    return -365.77e3 + 1.6582e3 * temperature - 4.2395e-1 * temperature2 +
           1.4847e-4 * temperature3 + 2992.6e3 / temperature;
  }

  /**
   * Heat capacity of liquid Na in J/kg-K as a function of temperature. From page 29 (or by
   * differentiating enthalpy). Valid from 371 to 2000 K.
   * @param T temperature in K
   * @ return Heat capacity of liquid sodium in J/kg/K
   */
  template <typename T>
  T heatCapacity(T temperature) const
  {
    if (_cp)
      return _cp;

    const T temperature2 = temperature * temperature;

    // Converted from kJ/kg-K to J/kg-K.
    return 1.6582e3 - 8.4790e-1 * temperature + 4.4541e-4 * temperature2 - 2992.6e3 / temperature2;
  }

  /**
   * Inverse solve for temperature from enthalpy
   * @param h enthalpy in J/kg
   * @return temperature of liquid sodium in K
   */
  template <typename T>
  T temperature(T enthalpy) const
  {
    // Estimate initial guess from linear part of enthalpy.
    T temperature = (enthalpy + 365.77e3) / 1.6582e3;

    // Newton-Raphson for this equation: enthalpy(T) - enthalpy = 0 = residual. This is easy because
    // dResidual/dT is just dH/dT, which is heat capacity.
    for (unsigned iteration = 0; iteration < 10; ++iteration)
    {
      const T residual = h(temperature) - enthalpy;
      temperature -= residual / heatCapacity(temperature);
      if (std::abs(residual / enthalpy) < 1e-6)
        break;
    }
    // If we get here, enthalpy is probably out of bounds. However, due to the nature of the JFNK
    // calculation, we probably just want to ignore the error and spit out a bogus T so that the
    // solver keeps rolling.
    return temperature;
  }

  /**
   * Density as a function of temperature. Valid 371 K < T < 2503.7 K
   * @param T temperature in K
   * @return density of liquid sodium in kg/m^3
   */
  template <typename T>
  T rho(T temperature) const
  {
    const T rhoc = 219.0; // kg/m^3
    const T f = 275.32;
    const T g = 511.58;
    const T Tc = 2503.7; // critical temperature, K
    mooseAssert(temperature < Tc, "Temperature is greater than critical temperature 2503.7 K ");

    return rhoc + f * (1.0 - temperature / Tc) + g * std::sqrt(1.0 - temperature / Tc);
  }

  /**
   * Derivative of density w.r.t temperature. Valid 371 K < T < 2503.7 K
   * @param T temperature in K
   * @return derivative of density of liquid sodium with respect to temperature
   */
  template <typename T>
  T drho_dT(T temperature) const
  {
    const T f = 275.32;
    const T g = 511.58;
    const T Tc = 2503.7; // critical temperature, K
    mooseAssert(temperature < Tc, "Temperature is greater than critical temperature 2503.7 K ");

    return -(f + g * 0.5 / std::sqrt(1.0 - temperature / Tc)) / Tc;
  }

  /**
   * Derivative of density w.r.t enthalpy. Valid 371 K < T < 2503.7 K
   * @param T enthalpy in J/kg
   * @return derivative of density of liquid sodium with respect to enthalpy
   */
  template <typename T>
  T drho_dh(T enthalpy) const
  {
    const T temperature = this->temperature(enthalpy);
    return drho_dT(temperature) / heatCapacity(temperature);
  }

private:
  /// Optional thermal conductivity from input parameters.
  const Real _k;

  /// Optional specific heat from input parameters.
  const Real _cp;
};
