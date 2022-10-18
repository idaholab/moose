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

/**
 * Fluid properties for helium \cite petersen \cite harlow
 */
class HeliumFluidProperties : public SinglePhaseFluidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();

  HeliumFluidProperties(const InputParameters & parameters);

  /**
   * Fluid name
   *
   * @return "helium"
   */
  virtual std::string fluidName() const override;

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return pressure (Pa)
   */
  virtual Real p_from_v_e(Real v, Real e) const override;

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return pressure (Pa)
   */
  virtual ADReal p_from_v_e(const ADReal & v, const ADReal & e) const override;

  /**
   * Pressure and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume (m$^3$/kg)
   * @param[in] e        specific internal energy (J/kg)
   * @param[out] p       pressure (Pa)
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_de   derivative of pressure w.r.t. specific internal energy
   */
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;

  void p_from_v_e(const DualReal & v,
                  const DualReal & e,
                  DualReal & p,
                  DualReal & dp_dv,
                  DualReal & dp_de) const override;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return temperature (K)
   */
  virtual Real T_from_v_e(Real v, Real e) const override;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return temperature (K)
   */
  virtual ADReal T_from_v_e(const ADReal & v, const ADReal & e) const override;

  /**
   * Temperature and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume (m$^3$/kg)
   * @param[in] e        specific internal energy (J/kg)
   * @param[out] T       temperature (K)
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_de   derivative of temperature w.r.t. specific internal energy
   */
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const override;

  void T_from_v_e(const DualReal & v,
                  const DualReal & e,
                  DualReal & T,
                  DualReal & dT_dv,
                  DualReal & dT_de) const override;

  /**
   * Temperature from pressure and specific enthalpy
   *
   * @param[in] p       pressure (Pa)
   * @param[in] h       specific enthalpy (J/kg)
   * @param[out] T      temperature (K)
   */
  virtual Real T_from_p_h(Real p, Real h) const override;

  using SinglePhaseFluidProperties::c_from_v_e;

  /**
   * Speed of sound from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return speed of sound (m/s)
   */
  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;

  /**
   * Isobaric specific heat from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return isobaric specific heat (J/kg.K)
   */
  virtual Real cp_from_v_e(Real v, Real e) const override;

  /**
   * Isobaric specific heat from specific volume and specific internal energy
   *
   * @param[in] v       specific volume (m$^3$/kg)
   * @param[in] e       specific internal energy (J/kg)
   * @param[out] cp     isobaric specific heat (J/kg.K)
   * @param[out] dcp_dv derivative of isobaric specific heat w.r.t. specific volume
   * @param[out] dcp_de derivative of isobaric specific heat w.r.t. specific internal energy
   */
  void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;

  /**
   * Isochoric specific heat from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return isochoric specific heat (J/kg.K)
   */
  virtual Real cv_from_v_e(Real v, Real e) const override;

  /**
   * Isochoric specific heat from specific volume and specific internal energy
   *
   * @param[in] v       specific volume (m$^3$/kg)
   * @param[in] e       specific internal energy (J/kg)
   * @param[out] cv     isochoric specific heat (J/kg.K)
   * @param[out] dcv_dv derivative of isochoric specific heat w.r.t. specific volume
   * @param[out] dcv_de derivative of isochoric specific heat w.r.t. specific internal energy
   */
  void cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;

  using SinglePhaseFluidProperties::mu_from_v_e;

  /**
   * Dynamic viscosity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return dynamic viscosity (Pa.s)
   */
  virtual Real mu_from_v_e(Real v, Real e) const override;

  using SinglePhaseFluidProperties::k_from_v_e;

  /**
   * Thermal conductivity from specific volume and specific internal energy
   *
   * @param[in] v   specific volume (m$^3$/kg)
   * @param[in] e   specific internal energy (J/kg)
   * @return thermal conductivity (W/m.K)
   */
  virtual Real k_from_v_e(Real v, Real e) const override;

  using SinglePhaseFluidProperties::beta_from_p_T;

  /**
   * Thermal expansion coefficient from pressure and temperature
   *
   * @param[in] p   pressure (Pa)
   * @param[in] T   temperature (K)
   * @return thermal expansion coefficient (1/K)
   */
  virtual Real beta_from_p_T(Real p, Real T) const override;

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

  virtual void rho_from_p_T(const DualReal & p,
                            const DualReal & T,
                            DualReal & rho,
                            DualReal & drho_dp,
                            DualReal & drho_dT) const override;
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
   * Molar mass
   *
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

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
   * @param[out] dcv_dT derivative of isochoric specific heat w.r.t. temperature (J/kg/K/K)
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

  using SinglePhaseFluidProperties::e_from_T_v;
  Real e_from_T_v(Real T, Real /*v*/) const override;
  void e_from_T_v(Real T, Real v, Real & e, Real & de_dT, Real & de_dv) const override;
  ADReal e_from_T_v(const ADReal & T, const ADReal & v) const override;
  void e_from_T_v(const ADReal & T,
                  const ADReal & v,
                  ADReal & e,
                  ADReal & de_dT,
                  ADReal & de_dv) const override;

  using SinglePhaseFluidProperties::p_from_T_v;
  Real p_from_T_v(Real T, Real v) const override;
  ADReal p_from_T_v(const ADReal & T, const ADReal & v) const override;

  using SinglePhaseFluidProperties::e_from_p_rho;
  ADReal e_from_p_rho(const ADReal & p, const ADReal & rho) const override;

protected:
  /// specific heat at constant volume
  const Real _cv;

  /// specific heat at constant pressure
  const Real _cp;
};
#pragma GCC diagnostic pop
