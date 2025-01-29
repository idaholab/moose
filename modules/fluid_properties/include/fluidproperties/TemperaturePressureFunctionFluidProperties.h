//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"
#include "Function.h"

/**
 * Fluid properties provided as multiple-variable functions of temperature and pressure.
 * Temperature is passed as the first spatial coordinate, x, to the function, while pressure
 * is passed as the second spatial coordinate, y.
 */
class TemperaturePressureFunctionFluidProperties : public SinglePhaseFluidProperties
{
public:
  TemperaturePressureFunctionFluidProperties(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Fluid name
   *
   * @return "TemperaturePressureFunctionFluidProperties"
   */
  virtual std::string fluidName() const override;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return temperature (K)
   */
  virtual Real T_from_v_e(Real v, Real e) const override;

  /**
   * Temperature from pressure and density
   *
   * @param[in] p          pressure (Pa)
   * @param[in] rho        density (kg/m$^3$)
   * @return temperature (T)
   */
  virtual Real T_from_p_rho(Real p, Real rho) const;

  /**
   * Temperature from pressure and specific enthalpy
   *
   * @param[in] p          pressure (Pa)
   * @param[in] h          specific enthalpy (J/kg)
   * @return temperature (T)
   */
  virtual Real T_from_p_h(Real p, Real h) const override;

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return pressure (Pa)
   */
  virtual Real p_from_v_e(Real v, Real e) const override;

  /**
   * Isobaric specific heat from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return isobaric specific heat (J/kg.K)
   */
  virtual Real cp_from_v_e(Real v, Real e) const override;
  /**
   * Isobaric specific heat capacity and its derivatives from specific volume and energy
   *
   * @param[in] v       specific volume
   * @param[in] e       specific energy
   * @param[out] cp     isobaric specific heat (J/kg/K)
   * @param[out] dcp_dv derivative of isobaric specific heat w.r.t. specific volume
   * @param[out] dcp_de derivative of isobaric specific heat w.r.t. specific energy
   */
  virtual void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;

  /**
   * Isochoric specific heat from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return isochoric specific heat (J/kg.K)
   */
  virtual Real cv_from_v_e(Real v, Real e) const override;
  /**
   * Isochoric specific heat capacity and its derivatives from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] cv     isochoric specific heat (J/kg/K)
   * @param[out] dcv_dv derivative of isochoric specific heat w.r.t. specific volume
   * @param[out] dcv_de derivative of isochoric specific heat w.r.t. specific energy
   */
  virtual void cv_from_v_e(Real p, Real T, Real & cv, Real & dcv_dv, Real & dcv_de) const override;

  /**
   * Dynamic viscosity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return dynamic viscosity (Pa.s)
   */
  virtual Real mu_from_v_e(Real v, Real e) const override;

  /**
   * Thermal conductivity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return thermal conductivity (W/m.K)
   */
  virtual Real k_from_v_e(Real v, Real e) const override;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return density (kg/m$^3$)
   */
  virtual Real rho_from_p_T(Real p, Real T) const override;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p          pressure (Pa)
   * @param[in] T          temperature (K)
   * @param[out] rho       density (kg/m$^3$)
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   */
  virtual void
  rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual void rho_from_p_T(const ADReal & pressure,
                            const ADReal & temperature,
                            ADReal & rho,
                            ADReal & drho_dp,
                            ADReal & drho_dT) const override;

  /**
   * Specific volume from pressure and temperature
   * @param[in] p    pressure (Pa)
   * @param[in] T    temperature (K)
   * @return specific volume (m$^3$/kg)
   */
  virtual Real v_from_p_T(Real p, Real T) const override;

  /**
   * Specific volume and its derivatives from pressure and temperature
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] v      specific volume (m$^3$/kg)
   * @param[out] dv_dp  derivative of specific volume with respect to pressure
   * @param[out] dv_dT  derivative of specific volume with respect to temperature
   */
  virtual void v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const override;

  /**
   * Specific enthalpy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return specific enthalpy (J/kg)
   */
  virtual Real h_from_p_T(Real p, Real T) const override;

  /**
   * Specific enthalpy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] h       specific enthalpy (J/kg)
   * @param[out] dh_dp   derivative of specific enthalpy w.r.t. pressure
   * @param[out] dh_dT   derivative of specific enthalpy w.r.t. temperature
   */
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;

  /**
   * Specific internal energy from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return specific internal energy (J/kg)
   */
  virtual Real e_from_p_T(Real p, Real T) const override;

  /**
   * Specific internal energy and its derivatives from pressure and temperature
   *
   * @param[in] p        pressure (Pa)
   * @param[in] T        temperature (K)
   * @param[out] e       specific internal energy (J/kg)
   * @param[out] de_dp   derivative of specific internal energy w.r.t. pressure
   * @param[out] de_dT   derivative of specific internal energy w.r.t. temperature
   */
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;

  /**
   * Specific internal energy from pressure and density
   *
   * @param[in] p        pressure (Pa)
   * @param[in] rho      density (kg/m$^3$)
   * @param[out] e       specific internal energy (J/kg)
   */
  virtual Real e_from_p_rho(Real p, Real rho) const override;

  /**
   * Thermal expansion coefficient from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return thermal expansion coefficient (1/K)
   */
  virtual Real beta_from_p_T(Real p, Real T) const override;

  /**
   * Isobaric specific heat capacity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return isobaric specific heat (J/kg/.K)
   */
  virtual Real cp_from_p_T(Real p, Real T) const override;

  /**
   * Isobaric specific heat capacity and its derivatives from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] cp     isobaric specific heat (J/kg/K)
   * @param[out] dcp_dp derivative of isobaric specific heat w.r.t. pressure (J/kg/K/Pa)
   * @param[out] dcp_dT derivative of isobaric specific heat w.r.t. temperature (J/kg/K/K)
   */
  virtual void cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  /**
   * Isochoric specific heat capacity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return isochoric specific heat (J/kg.K)
   */
  virtual Real cv_from_p_T(Real p, Real T) const override;
  /**
   * Isochoric specific heat capacity and its derivatives from pressure and temperature
   *
   * @param[in] p       pressure (Pa)
   * @param[in] T       temperature (K)
   * @param[out] cv     isochoric specific heat (J/kg/K)
   * @param[out] dcv_dp derivative of isochoric specific heat w.r.t. pressure (J/kg/K/Pa)
   */
  virtual void cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const override;

  /**
   * Thermal conductivity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return thermal conductivity  (W/m.K)
   */
  virtual Real k_from_p_T(Real p, Real T) const override;

  /**
   * Thermal conductivity and its derivatives wrt pressure and temperature
   *
   * @param p           pressure (Pa)
   * @param T           temperature (K)
   * @param[out]  k     thermal conductivity  (W/m.K)
   * @param[out]  dk_dp derivative of thermal conductivity wrt pressure
   * @param[out]  dk_dT derivative of thermal conductivity wrt temperature
   */
  virtual void k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const override;

  /**
   * Dynamic viscosity from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return dynamic viscosity (Pa.s)
   */
  virtual Real mu_from_p_T(Real p, Real T) const override;

  /**
   * Dynamic viscosity and its derivatives wrt pressure and temperature
   *
   * @param p             pressure (Pa)
   * @param T             temperature (K)
   * @param[out] mu       viscosity (Pa.s)
   * @param[out] dmu_dp   derivative of viscosity wrt pressure
   * @param[out] dmu_dT   derivative of viscosity wrt temperature
   */
  virtual void
  mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_drho, Real & dmu_dT) const override;

  // Need those to avoid running the infinite loop of s_pT calling s_ve calling s_pT in SinglePhase
  Real s_from_p_T(Real /*p*/, Real /*T*/) const override { mooseError("Not implemented"); }
  void s_from_p_T(Real, Real, Real &, Real &, Real &) const override
  {
    mooseError("Not implemented");
  }

  // This is done to avoid hiding the AD implementations from the template
  // with the regular implementations defined here
  using SinglePhaseFluidProperties::beta_from_p_T;
  using SinglePhaseFluidProperties::cp_from_p_T;
  using SinglePhaseFluidProperties::cp_from_v_e;
  using SinglePhaseFluidProperties::cv_from_p_T;
  using SinglePhaseFluidProperties::cv_from_v_e;
  using SinglePhaseFluidProperties::e_from_p_rho;
  using SinglePhaseFluidProperties::e_from_p_T;
  using SinglePhaseFluidProperties::h_from_p_T;
  using SinglePhaseFluidProperties::k_from_p_T;
  using SinglePhaseFluidProperties::k_from_v_e;
  using SinglePhaseFluidProperties::mu_from_p_T;
  using SinglePhaseFluidProperties::mu_from_v_e;
  using SinglePhaseFluidProperties::p_from_v_e;
  using SinglePhaseFluidProperties::rho_from_p_T;
  using SinglePhaseFluidProperties::s_from_p_T;
  using SinglePhaseFluidProperties::T_from_p_h;
  using SinglePhaseFluidProperties::T_from_v_e;
  using SinglePhaseFluidProperties::v_from_p_T;

protected:
  /// Functions are constructed after fluid properties, so we delay the getting of the Function
  void initialSetup() override;

  /// whether the object is initialized, eg, the functions have been retrieved from the problem
  bool _initialized;

  /// function defining thermal conductivity as a function of temperature and pressure
  const Function * _k_function;

  /// function defining density as a function of temperature and pressure
  const Function * _rho_function;

  /// function defining dynamic viscosity as a function of temperature and pressure
  const Function * _mu_function;

  /// function defining specific heat as a function of temperature and pressure
  const Function * _cp_function;

  /// constant isochoric specific heat
  const Real _cv;
  /// whether a constant isochoric specific heat is used
  const bool _cv_is_constant;
  /// Reference specific energy
  const Real _e_ref;
  /// Reference temperature for the reference specific energy
  const Real _T_ref;
  /// Size of temperature intervals when integrating the specific heat to compute the specific energy
  const Real _integration_dT;
};
