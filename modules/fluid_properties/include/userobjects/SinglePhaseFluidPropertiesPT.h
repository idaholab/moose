//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SINGLEPHASEFLUIDPROPERTIESPT_H
#define SINGLEPHASEFLUIDPROPERTIESPT_H

#include "FluidProperties.h"

class SinglePhaseFluidPropertiesPT;

template <>
InputParameters validParams<SinglePhaseFluidPropertiesPT>();

/**
 * Common class for single phase fluid properties using a pressure
 * and temperature formulation
 */
class SinglePhaseFluidPropertiesPT : public FluidProperties
{
public:
  SinglePhaseFluidPropertiesPT(const InputParameters & parameters);
  virtual ~SinglePhaseFluidPropertiesPT();

  /**
   * Fluid name
   * @return string representing fluid name
   */
  virtual std::string fluidName() const = 0;

  /**
   * Molar mass
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const = 0;

  /**
   * Density
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return density (kg/m^3)
   */
  virtual Real rho(Real pressure, Real temperature) const = 0;

  /**
   * Density and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   */
  virtual void
  rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const = 0;

  /**
   * Internal energy
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return internal energy (J/kg)
   */
  virtual Real e(Real pressure, Real temperature) const = 0;

  /**
   * Internal energy and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] e internal energy (J/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   */
  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const = 0;

  /**
   * Density and internal energy and their derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] e internal energy (J/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   */
  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const = 0;

  /**
   * Speed of sound
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return speed of sound (m/s)
   */
  virtual Real c(Real pressure, Real temperature) const = 0;

  /**
   * Isobaric specific heat capacity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cp (J/kg/K)
   */
  virtual Real cp(Real pressure, Real temperature) const = 0;

  /**
   * Isochoric specific heat
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cv (J/kg/K)
   */
  virtual Real cv(Real pressure, Real temperature) const = 0;

  /**
   * Adiabatic index - ratio of specific heats
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return gamma (-)
   */
  virtual Real gamma(Real pressure, Real temperature) const;

  /*
   * Dynamic viscosity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real pressure, Real temperature) const = 0;
  /*
   * Dynamic viscosity and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_dp derivative of viscosity wrt pressure
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   */
  virtual void
  mu_dpT(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const = 0;

  /*
   * Dynamic viscosity as a function of density and temperature
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu_from_rho_T(Real density, Real temperature) const = 0;

  /**
   * Dynamic viscosity and its derivatives wrt density and temperature
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @param ddensity_dT derivative of density wrt temperature
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_drho derivative of viscosity wrt density
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   */
  virtual void mu_drhoT_from_rho_T(Real density,
                                   Real temperature,
                                   Real ddensity_dT,
                                   Real & mu,
                                   Real & dmu_drho,
                                   Real & dmu_dT) const = 0;
  /**
   * Thermal conductivity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return thermal conductivity  (W/m/K)
   */
  virtual Real k(Real pressure, Real temperature) const = 0;

  /**
   * Thermal conductivity and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] thermal conductivity  (W/m/K)
   * @param[out] derivative of thermal conductivity wrt pressure
   * @param[out] derivative of thermal conductivity wrt temperature
   */
  virtual void
  k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const = 0;

  /**
   * Thermal conductivity as a function of density and temperature
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return thermal conductivity  (W/m/K)
   */
  virtual Real k_from_rho_T(Real density, Real temperature) const = 0;

  /**
   * Specific entropy
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return s (J/kg/K)
   */
  virtual Real s(Real pressure, Real temperature) const = 0;

  /**
   * Specific enthalpy
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return h (J/kg)
   */
  virtual Real h(Real p, Real T) const = 0;

  /**
   * Enthalpy and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] h (J/kg)
   * @param[out] dh_dp derivative of enthalpy wrt pressure
   * @param[out] dh_dT derivative of enthalpy wrt temperature
   */
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const = 0;

  /**
   * Thermal expansion coefficient
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return beta (1/K)
   */
  virtual Real beta(Real pressure, Real temperature) const = 0;

  /**
   * Henry's law constant for dissolution in water
   * @param temperature fluid temperature (K)
   * @return Henry's constant
   */
  virtual Real henryConstant(Real temperature) const = 0;

  /**
   * Henry's law constant for dissolution in water and derivative wrt temperature
   * @param temperature fluid temperature (K)
   * @param[out] Kh Henry's constant
   * @param[out] dKh_dT derivative of Kh wrt temperature
   */
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const = 0;

protected:
  /**
   * IAPWS formulation of Henry's law constant for dissolution in water
   * From Guidelines on the Henry's constant and vapour
   * liquid distribution constant for gases in H20 and D20 at high
   * temperatures, IAPWS (2004)
   */
  virtual Real henryConstantIAPWS(Real temperature, Real A, Real B, Real C) const;

  /// IAPWS formulation of Henry's law constant for dissolution in water and derivative wrt temperature
  virtual void
  henryConstantIAPWS_dT(Real temperature, Real & Kh, Real & dKh_dT, Real A, Real B, Real C) const;

  /// Universal gas constant (J/mol/K)
  const Real _R;
  /// Conversion of temperature from Celcius to Kelvin
  const Real _T_c2k;
};

#endif /* SINGLEPHASEFLUIDPROPERTIESPT_H */
