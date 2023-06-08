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
 * Base class for fluid properties used with HEM
 */
class HEMFluidProperties : public FluidProperties
{
public:
  static InputParameters validParams();

  HEMFluidProperties(const InputParameters & parameters);

  /**
   * Pressure as a function of specific internal energy and specific volume
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real pressure(Real v, Real e) const = 0;

  /**
   * Derivative of pressure w.r.t internal energy and specific volume
   */
  virtual void
  dp_duv(Real v, Real e, Real & dp_dv, Real & dp_de, Real & dT_dv, Real & dT_de) const = 0;

  /**
   * Temperature as a function of specific internal energy and specific volume
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real temperature(Real v, Real e) const = 0;

  /**
   * Ratio of specific heats
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real gamma(Real v, Real e) const = 0;

  /**
   * Quality as a function of specific volume and specific internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real quality(Real v, Real e) const = 0;

  /**
   * Quality as a function of saturation temperature and enthalpy
   * @param Tsat Saturation temperature
   * @param h Enthalpy
   */
  virtual Real quality_Tsat_h(Real Tsat, Real h) const = 0;

  /**
   * Sound speed as a function of specific volume and specific internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real c(Real v, Real e) const = 0;

  /**
   * Constant pressure specific heat capacity as a function of specific volume
   * and specific internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real cp(Real v, Real e) const = 0;

  /**
   * Constant volume specific heat capacity as a function of specific volume and
   * specific internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real cv(Real v, Real e) const = 0;

  /**
   * Thermal expansion coefficient [1/K] as a function of pressure and
   * temperature
   * @param pressure pressure
   * @param temperature temperature
   */
  virtual Real beta(Real pressure, Real temperature) const = 0;

  /**
   * Dynamic viscosity, [Pa-s], as a function of specific volume and specific
   * internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real mu(Real v, Real e) const = 0;

  /**
   * Thermal conductivity, [W/K-m], as a function of specific volume and
   * specific internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real k(Real v, Real e) const = 0;

  /**
   * Vapor void fraction as a function of specific volume and specific
   * internal energy
   * @param v Specific volume
   * @param e Specific internal energy
   */
  virtual Real alpha_vapor(Real v, Real e) const = 0;

  /**
   * dT/dp along the saturation line
   */
  virtual Real dT_dP_saturation(Real pressure) const = 0;

  /**
   * Density as a function of pressure and temperature
   * @param pressure pressure
   * @param temperature temperature
   * @param quality quality
   */
  virtual Real rho(Real pressure, Real temperature, Real quality) const = 0;

  /**
   * Density derivative as a function of pressure and temperature
   * @param pressure pressure
   * @param temperature temperature
   */
  virtual void
  rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const = 0;

  /**
   * Density and internal energy as a function of pressure and temperature
   * @param pressure pressure
   * @param temperature temperature
   */
  virtual void rho_e(Real pressure, Real temperature, Real & rho, Real & e) const = 0;

  /**
   * Internal energy as a function of pressure and density
   * @param pressure pressure
   * @param rho density
   */
  virtual Real e(Real pressure, Real rho) const = 0;

  /**
   * Saturation temperature
   * @param pressure pressure
   */
  virtual Real saturation_T(Real pressure) const = 0;

  /**
   * Saturation pressure
   * @param temperature temperature
   */
  virtual Real saturation_P(Real temperature) const = 0;

  /**
   * Surface Tension
   * @param temperature temperature
   */
  virtual Real surfaceTension(Real temperature) const = 0;

  /**
   * Enthalpy
   * @param pressure pressure
   * @param temperature temperature
   * @param quality quality
   */
  virtual Real h(Real pressure, Real temperature, Real quality) const = 0;

  /**
   * Derivative of Enthalpy as a function of temperature and pressure
   * @param pressure pressure
   * @param temperature temperature
   */
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const = 0;

  /**
   * Critical pressure
   */
  virtual Real p_critical() const = 0;

  /**
   * Latent enthalpy as a function of temperature
   */
  virtual void h_lat(Real temperature, Real & hf, Real & hg, Real & hfg) const = 0;

  /**
   * Specific volume as a function of pressure and enthalpy
   * @param pressure pressure
   * @param enthalpy enthalpy
   */
  virtual Real v_ph(Real pressure, Real enthalpy) const = 0;

  /**
   * Molar mass [kg/mol]
   *
   * @return molar mass
   */
  virtual Real molarMass() const;

  /**
   * Critical pressure
   * @return critical pressure (Pa)
   */
  virtual Real criticalPressure() const;

  /**
   * Critical temperature
   * @return critical temperature (K)
   */
  virtual Real criticalTemperature() const;

  /**
   * Critical density
   * @return critical density (kg/m^3)
   */
  virtual Real criticalDensity() const;

  /**
   * Critical specific internal energy
   * @return specific internal energy (J/kg)
   */
  virtual Real criticalInternalEnergy() const;

  /**
   * Triple point pressure
   * @return triple point pressure (Pa)
   */
  virtual Real triplePointPressure() const;

  /**
   * Triple point temperature
   * @return triple point temperature (K)
   */
  virtual Real triplePointTemperature() const;
};
