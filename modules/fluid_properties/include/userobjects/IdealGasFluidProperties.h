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
#include "NaNInterface.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Ideal gas fluid properties
 * Default parameters are for air at atmospheric pressure and temperature
 */
class IdealGasFluidProperties : public SinglePhaseFluidProperties, public NaNInterface
{
public:
  static InputParameters validParams();

  IdealGasFluidProperties(const InputParameters & parameters);
  virtual ~IdealGasFluidProperties();

  virtual Real p_from_v_e(Real v, Real e) const override;
  virtual ADReal p_from_v_e(const ADReal & v, const ADReal & e) const override;
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;
  virtual void p_from_v_e(const DualReal & v,
                          const DualReal & e,
                          DualReal & p,
                          DualReal & dp_dv,
                          DualReal & dp_de) const override;
  virtual Real T_from_v_e(Real v, Real e) const override;
  virtual ADReal T_from_v_e(const ADReal & v, const ADReal & e) const override;
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const override;
  virtual void T_from_v_e(const DualReal & v,
                          const DualReal & e,
                          DualReal & T,
                          DualReal & dT_dv,
                          DualReal & dT_de) const override;
  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual ADReal c_from_v_e(const ADReal & v, const ADReal & e) const override;
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;
  virtual Real cp_from_v_e(Real v, Real e) const override;
  virtual void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;
  virtual Real cv_from_v_e(Real v, Real e) const override;
  virtual void cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;
  virtual Real mu_from_v_e(Real v, Real e) const override;
  virtual void mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const override;
  virtual Real k_from_v_e(Real v, Real e) const override;
  virtual void k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const override;
  virtual Real s_from_v_e(Real v, Real e) const override;
  virtual void s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const override;
  virtual Real s_from_p_T(Real p, Real T) const override;
  virtual void s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const override;
  virtual Real s_from_h_p(Real h, Real p) const override;
  virtual void s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const override;
  virtual Real rho_from_p_s(Real p, Real s) const override;
  virtual void
  rho_from_p_s(Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const override;
  virtual Real e_from_v_h(Real v, Real h) const override;
  virtual void e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const override;
  virtual Real rho_from_p_T(Real p, Real T) const override;
  virtual ADReal rho_from_p_T(const ADReal & p, const ADReal & T) const override;
  virtual void
  rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual void rho_from_p_T(const DualReal & p,
                            const DualReal & T,
                            DualReal & rho,
                            DualReal & drho_dp,
                            DualReal & drho_dT) const override;
  virtual Real e_from_p_rho(Real p, Real rho) const override;
  virtual ADReal e_from_p_rho(const ADReal & p, const ADReal & rho) const override;
  virtual void
  e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const override;
  virtual void e_from_p_rho(const ADReal & p,
                            const ADReal & rho,
                            ADReal & e,
                            ADReal & de_dp,
                            ADReal & de_drho) const override;
  virtual Real e_from_T_v(Real T, Real v) const override;
  virtual void e_from_T_v(Real T, Real v, Real & e, Real & de_dT, Real & de_dv) const override;
  virtual ADReal e_from_T_v(const ADReal & T, const ADReal & v) const override;
  virtual void e_from_T_v(const ADReal & T,
                          const ADReal & v,
                          ADReal & e,
                          ADReal & de_dT,
                          ADReal & de_dv) const override;
  virtual Real p_from_T_v(Real T, Real v) const override;
  virtual void p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const override;
  virtual Real h_from_T_v(Real T, Real v) const override;
  virtual void h_from_T_v(Real T, Real v, Real & h, Real & dh_dT, Real & dh_dv) const override;
  virtual Real s_from_T_v(Real T, Real v) const override;
  virtual void s_from_T_v(Real T, Real v, Real & s, Real & ds_dT, Real & ds_dv) const override;
  virtual Real cv_from_T_v(Real T, Real v) const override;
  virtual Real e_spndl_from_v(Real v) const override;
  virtual void v_e_spndl_from_T(Real T, Real & v, Real & e) const override;
  virtual Real h_from_p_T(Real p, Real T) const override;
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;
  virtual Real e_from_p_T(Real p, Real T) const override;
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;
  virtual Real p_from_h_s(Real h, Real s) const override;
  virtual void p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const override;
  virtual Real g_from_v_e(Real v, Real e) const override;
  virtual Real T_from_p_h(Real p, Real h) const override;
  virtual void T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const override;
  virtual Real cv_from_p_T(Real p, Real T) const override;
  virtual void cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const override;
  virtual Real cp_from_p_T(Real p, Real T) const override;
  virtual void cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;
  virtual Real mu_from_p_T(Real p, Real T) const override;
  virtual void mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;
  virtual Real k_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;
  virtual std::string fluidName() const override;
  virtual Real molarMass() const override;
  virtual Real criticalTemperature() const override;
  virtual Real criticalDensity() const override;
  virtual Real criticalInternalEnergy() const override;
  virtual Real gamma_from_v_e(Real v, Real e) const override;
  virtual Real gamma_from_p_T(Real p, Real T) const override;
  virtual Real c_from_p_T(Real p, Real T) const override;
  virtual ADReal c_from_p_T(const ADReal & p, const ADReal & T) const override;
  virtual void c_from_p_T(Real /*p*/, Real T, Real & c, Real & dc_dp, Real & dc_dT) const override;

  virtual Real pp_sat_from_p_T(Real /*p*/, Real /*T*/) const override;

  // Methods used by Navier-Stokes module
  virtual Real gamma() const { return _gamma; };
  virtual Real cv() const { return _cv; };
  virtual Real cp() const { return _cp; };

protected:
  /// Adiabatic index (ratio of specific heats cp/cv)
  const Real & _gamma;
  /// molar mass
  const Real & _molar_mass;

  /// Specific gas constant (R / molar mass)
  const Real _R_specific;
  /// Specific heat at constant pressure
  const Real _cp;
  /// Specific heat at constant volume
  const Real _cv;

  /// Dynamic viscosity
  const Real _mu;
  /// Thermal conductivity
  const Real _k;

  // properties at critical point (used by IdealRealGasMixtureFluidProperties (primary component))
  Real _T_c;
  Real _rho_c;
  Real _e_c;
};

#pragma GCC diagnostic pop
