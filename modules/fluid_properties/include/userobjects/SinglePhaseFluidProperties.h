//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SINGLEPHASEFLUIDPROPERTIES_H
#define SINGLEPHASEFLUIDPROPERTIES_H

#include "FluidProperties.h"

class SinglePhaseFluidProperties;

template <>
InputParameters validParams<SinglePhaseFluidProperties>();

/**
 * Common class for single phase fluid properties
 */
class SinglePhaseFluidProperties : public FluidProperties
{
public:
  SinglePhaseFluidProperties(const InputParameters & parameters);
  virtual ~SinglePhaseFluidProperties();

  /**
   * Fluid name
   * @return string representing fluid name
   */
  virtual std::string fluidName() const;

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real p_from_v_e(Real v, Real e) const;

  /**
   * Pressure and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] p       pressure
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_de   derivative of pressure w.r.t. specific internal energy
   */
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @return sound speed
   */
  virtual Real T_from_v_e(Real v, Real e) const;

  /**
   * Temperature and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] T       temperature
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_de   derivative of temperature w.r.t. specific internal energy
   */
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const;

  /**
   * Sound speed from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real c_from_v_e(Real v, Real e) const;

  /**
   * Sound speed and its derivatives from specific volume and specific internal energy
   *
   * @param[in]  v       specific volume
   * @param[in]  e       specific internal energy
   * @param[out] dc_dv   derivative of sound speed w.r.t. specific volume
   * @param[out] dc_de   derivative of sound speed w.r.t. specific internal energy
   */
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const;

  /**
   * Isobaric (constant-pressure) specific heat from specific volume and specific
   * internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real cp_from_v_e(Real v, Real e) const;

  /**
   * Isochoric (constant-volume) specific heat from specific volume and specific
   * internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real cv_from_v_e(Real v, Real e) const;

  /**
   * Dynamic viscosity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real mu_from_v_e(Real v, Real e) const;

  /**
   * Thermal conductivity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real k_from_v_e(Real v, Real e) const;

  /**
   * Specific entropy from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real s_from_v_e(Real v, Real e) const;

  /**
   * Specific entropy and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[out] s       specific entropy
   * @param[out] ds_dv   derivative of specific entropy w.r.t. specific volume
   * @param[out] ds_de   derivative of specific entropy w.r.t. specific internal energy
   */
  virtual void s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const;

  /**
   * Specific entropy from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real s_from_p_T(Real p, Real T) const;
  virtual Real s(Real pressure, Real temperature) const;

  /**
   * Specific entropy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure
   * @param[in] T        temperature
   * @param[out] s       specific entropy
   * @param[out] ds_dp   derivative of specific entropy w.r.t. pressure
   * @param[out] ds_dT   derivative of specific entropy w.r.t. temperature
   */
  virtual void s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const;

  /**
   * Specific entropy from specific enthalpy and pressure
   *
   * @param[in] h   specific enthalpy
   * @param[in] p   pressure
   */
  virtual Real s_from_h_p(Real h, Real p) const;

  /**
   * Specific entropy and its derivatives from specific enthalpy and pressure
   *
   * @param[in] h        specific enthalpy
   * @param[in] p        pressure
   * @param[out] s       specific entropy
   * @param[out] ds_dh   derivative of specific entropy w.r.t. specific enthalpy
   * @param[out] ds_dp   derivative of specific entropy w.r.t. pressure
   */
  virtual void s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const;

  /**
   * Density from pressure and specific entropy
   *
   * @param[in] p   pressure
   * @param[in] s   specific entropy
   */
  virtual Real rho_from_p_s(Real p, Real s) const;

  /**
   * Density and its derivatives from pressure and specific entropy
   *
   * @param[in] p          pressure
   * @param[in] s          specific entropy
   * @param[out] rho       density
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_ds   derivative of density w.r.t. specific entropy
   */
  virtual void rho_from_p_s(Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const;

  /**
   * Specific internal energy as a function of specific volume and specific enthalpy
   *
   * @param[in] v   specific volume
   * @param[in] h   specific enthalpy
   */
  virtual Real e_from_v_h(Real v, Real h) const;

  /**
   * Specific internal energy and derivatives as a function of specific volume and specific enthalpy
   *
   * @param[in]  v       specific volume
   * @param[in]  h       specific enthalpy
   * @param[out] de_dv   derivative of specific internal energy w.r.t. specific volume
   * @param[out] de_dh   derivative of specific internal energy w.r.t. specific enthalpy
   */
  virtual void e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return density (kg/m^3)
   */
  virtual Real rho_from_p_T(Real p, Real T) const;
  virtual Real rho(Real p, Real T) const;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure (Pa)
   * @param[in] T          temperature (K)
   * @param[out] rho       density (kg/m^3)
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   */
  virtual void rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const;
  virtual void
  rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const;

  /**
   * Specific volume from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real v_from_p_T(Real p, Real T) const;

  /**
   * Specific volume and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure
   * @param[in] T          temperature
   * @param[out] v         specific volume
   * @param[out] dv_dp     derivative of specific volume w.r.t. pressure
   * @param[out] dv_dT     derivative of specific volume w.r.t. temperature
   */
  virtual void v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const;

  /**
   * Specific internal energy from pressure and density
   *
   * @param[in] p     pressure
   * @param[in] rho   density
   */
  virtual Real e_from_p_rho(Real p, Real rho) const;

  /**
   * Specific internal energy and its derivatives from pressure and density
   *
   * @param[in] p          pressure
   * @param[in] rho        density
   * @param[out] e         specific internal energy
   * @param[out] de_dp     derivative of specific internal energy w.r.t. pressure
   * @param[out] de_drho   derivative of specific internal energy w.r.t. density
   */
  virtual void e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const;

  /**
   * Pressure from temperature and specific volume
   *
   * @param[in] T     temperature
   * @param[in] v     specific volume
   */
  virtual Real p_from_T_v(Real T, Real v) const;

  /**
   * Specific enthalpy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return h (J/kg)
   */
  virtual Real h_from_p_T(Real p, Real T) const;
  virtual Real h(Real p, Real T) const;

  /**
   * Specific enthalpy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] h       specific enthalpy (J/kg)
   * @param[out] dh_dp   derivative of specific enthalpy w.r.t. pressure
   * @param[out] dh_dT   derivative of specific enthalpy w.r.t. temperature
   */
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const;
  virtual void h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const;

  /**
   * Internal energy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return internal energy (J/kg)
   */
  virtual Real e_from_p_T(Real p, Real T) const;
  virtual Real e(Real pressure, Real temperature) const;

  /**
   * Internal energy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] e       internal energy (J/kg)
   * @param[out] de_dp   derivative of internal energy w.r.t. pressure
   * @param[out] de_dT   derivative of internal energy w.r.t. temperature
   */
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const;
  virtual void e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const;

  /**
   * Pressure from specific enthalpy and specific entropy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual Real p_from_h_s(Real h, Real s) const;

  /**
   * Pressure and its derivatives from specific enthalpy and specific entropy
   *
   * @param[in] h   specific enthalpy
   * @param[in] s   specific entropy
   */
  virtual void p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const;

  /**
   * Gibbs free energy from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   */
  virtual Real g_from_v_e(Real v, Real e) const;

  /**
   * Thermal expansion coefficient from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return beta (1/K)
   */
  virtual Real beta_from_p_T(Real p, Real T) const;
  virtual Real beta(Real pressure, Real temperature) const;

  /**
   * Temperature from pressure and specific enthalpy
   *
   * @param[in] pressure   pressure (Pa)
   * @param[in] enthalpy   enthalpy (J/kg)
   * @return Temperature (K)
   */
  virtual Real T_from_p_h(Real pressure, Real enthalpy) const;

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
   * Triple point pressure
   * @return triple point pressure (Pa)
   */
  virtual Real triplePointPressure() const;

  /**
   * Triple point temperature
   * @return triple point temperature (K)
   */
  virtual Real triplePointTemperature() const;

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
                         Real & de_dT) const;

  /**
   * Speed of sound
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return speed of sound (m/s)
   */
  virtual Real c_from_p_T(Real pressure, Real temperature) const;
  virtual Real c(Real pressure, Real temperature) const;

  /**
   * Isobaric specific heat capacity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cp (J/kg/K)
   */
  virtual Real cp_from_p_T(Real pressure, Real temperature) const;

  /**
   * Isochoric specific heat
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cv (J/kg/K)
   */
  virtual Real cv_from_p_T(Real pressure, Real temperature) const;

  /**
   * Adiabatic index - ratio of specific heats
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return gamma (-)
   */
  virtual Real gamma_from_p_T(Real pressure, Real temperature) const;

  /**
   * Dynamic viscosity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real pressure, Real temperature) const;
  virtual Real mu_from_p_T(Real pressure, Real temperature) const;

  /**
   * Dynamic viscosity and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_dp derivative of viscosity wrt pressure
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   */
  virtual void
  mu_dpT(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const;
  virtual void
  mu_from_p_T(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const;

  /**
   * Dynamic viscosity as a function of density and temperature
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu_from_rho_T(Real density, Real temperature) const;

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
                                   Real & dmu_dT) const;

  /**
   * Density and viscosity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] mu viscosity (Pa.s)
   */
  virtual void rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const;

  /**
   * Density and viscosity and their derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_dp derivative of viscosity wrt pressure
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   */
  virtual void rho_mu_dpT(Real pressure,
                          Real temperature,
                          Real & rho,
                          Real & drho_dp,
                          Real & drho_dT,
                          Real & mu,
                          Real & dmu_dp,
                          Real & dmu_dT) const;
  /**
   * Thermal conductivity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return thermal conductivity  (W/m/K)
   */
  virtual Real k_from_p_T(Real pressure, Real temperature) const;
  virtual Real k(Real pressure, Real temperature) const;

  /**
   * Thermal conductivity and its derivatives wrt pressure and temperature
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] thermal conductivity  (W/m/K)
   * @param[out] derivative of thermal conductivity wrt pressure
   * @param[out] derivative of thermal conductivity wrt temperature
   */
  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const;
  virtual void k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const;

  /**
   * Thermal conductivity as a function of density and temperature
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return thermal conductivity  (W/m/K)
   */
  virtual Real k_from_rho_T(Real density, Real temperature) const;

  /**
   * Henry's law constant for dissolution in water
   * @param temperature fluid temperature (K)
   * @return Henry's constant
   */
  virtual Real henryConstant(Real temperature) const;

  /**
   * Henry's law constant for dissolution in water and derivative wrt temperature
   * @param temperature fluid temperature (K)
   * @param[out] Kh Henry's constant
   * @param[out] dKh_dT derivative of Kh wrt temperature
   */
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const;

  /**
   * Vapor pressure. Used to delineate liquid and gas phases.
   * Valid for temperatures between the triple point temperature
   * and the critical temperature
   *
   * @param temperature water temperature (K)
   * @return saturation pressure (Pa)
   */
  virtual Real vaporPressure(Real temperature) const;

  /**
   * Vapor pressure. Used to delineate liquid and gas phases.
   * Valid for temperatures between the triple point temperature
   * and the critical temperature
   *
   * @param temperature water temperature (K)
   * @param[out] saturation pressure (Pa)
   * @param[out] derivative of saturation pressure wrt temperature (Pa/K)
   */
  virtual void vaporPressure_dT(Real temperature, Real & psat, Real & dpsat_dT) const;

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
  /// Conversion of temperature from Celsius to Kelvin
  const Real _T_c2k;
};

#endif /* SINGLEPHASEFLUIDPROPERTIES_H */
