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
 * Stiffened gas fluid properties
 */
class StiffenedGasFluidProperties : public SinglePhaseFluidProperties, public NaNInterface
{
public:
  static InputParameters validParams();

  StiffenedGasFluidProperties(const InputParameters & parameters);
  virtual ~StiffenedGasFluidProperties();

  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;
  virtual Real cp_from_v_e(Real v, Real e) const override;
  virtual void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;
  virtual Real cv_from_v_e(Real v, Real e) const override;
  virtual void cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;
  virtual Real mu_from_v_e(Real v, Real e) const override;
  virtual void mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const override;
  virtual Real k_from_v_e(Real v, Real e) const override;
  virtual void k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const override;
  virtual Real s_from_h_p(Real h, Real p) const override;
  virtual void s_from_h_p(Real h, Real p, Real & s, Real & ds_dh, Real & ds_dp) const override;
  virtual Real rho_from_p_s(Real p, Real s) const override;
  virtual void
  rho_from_p_s(Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const override;
  propfuncWithDefinitionOverride(rho, p, T);
  propfuncWithDefinitionOverride(e, p, rho);
  propfuncWithDefinitionOverride(s, v, e);
  propfuncWithDefinitionOverride(s, p, T);
  propfuncWithDefinitionOverride(T, v, e);
  propfuncWithDefinitionOverride(p, v, e);
  propfuncWithDefinitionOverride(T, p, h);
  virtual Real e_from_T_v(Real T, Real v) const override;
  virtual void e_from_T_v(Real T, Real v, Real & e, Real & de_dT, Real & de_dv) const override;
  virtual Real p_from_T_v(Real T, Real v) const override;
  virtual void p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const override;
  virtual Real h_from_T_v(Real T, Real v) const override;
  virtual void h_from_T_v(Real T, Real v, Real & h, Real & dh_dT, Real & dh_dv) const override;
  virtual Real s_from_T_v(Real T, Real v) const override;
  virtual void s_from_T_v(Real T, Real v, Real & s, Real & ds_dT, Real & ds_dv) const override;
  virtual Real cv_from_T_v(Real T, Real v) const override;
  virtual Real e_spndl_from_v(Real v) const override;
  virtual void v_e_spndl_from_T(Real T, Real & v, Real & e) const override;
  virtual Real e_from_v_h(Real v, Real h) const override;
  virtual void e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const override;
  virtual Real h_from_p_T(Real p, Real T) const override;
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;
  virtual Real e_from_p_T(Real p, Real T) const override;
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;
  virtual Real p_from_h_s(Real h, Real s) const override;
  virtual void p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const override;
  virtual Real g_from_v_e(Real v, Real e) const override;
  virtual Real molarMass() const override;
  virtual Real criticalTemperature() const override;
  virtual Real criticalDensity() const override;
  virtual Real criticalInternalEnergy() const override;
  virtual Real cv_from_p_T(Real p, Real T) const override;
  virtual void cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const override;
  virtual Real cp_from_p_T(Real p, Real T) const override;
  virtual void cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;
  virtual Real mu_from_p_T(Real p, Real T) const override;
  virtual void mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;
  virtual Real k_from_p_T(Real p, Real T) const override;
  virtual void k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const override;
  virtual Real beta_from_p_T(Real p, Real T) const override;
  virtual void
  beta_from_p_T(Real p, Real T, Real & beta, Real & dbeta_dp, Real & dbeta_dT) const override;

  virtual Real c2_from_p_rho(Real pressure, Real rho) const;

  virtual Real pp_sat_from_p_T(Real /*p*/, Real /*T*/) const override;

protected:
  bool _allow_nonphysical_states;

  Real _gamma;
  Real _cv;
  Real _q;
  Real _q_prime;
  Real _p_inf;
  Real _cp;

  Real _mu;
  Real _k;
  Real _molar_mass;
  // properties at critical point
  Real _T_c;
  Real _rho_c;
  Real _e_c;
};

#pragma GCC diagnostic pop

template <typename CppType>
CppType
StiffenedGasFluidProperties::rho_from_p_T_template(const CppType & p, const CppType & T) const
{
  mooseAssert(((_gamma - 1.0) * _cv * T) != 0.0, "Invalid gamma or cv or temperature detected!");
  CppType rho = (p + _p_inf) / ((_gamma - 1.0) * _cv * T);
  if (!_allow_nonphysical_states && rho <= 0.)
    return getNaN();
  else
    return rho;
}

template <typename CppType>
void
StiffenedGasFluidProperties::rho_from_p_T_template(
    const CppType & p, const CppType & T, CppType & rho, CppType & drho_dp, CppType & drho_dT) const
{
  mooseAssert(((_gamma - 1.0) * _cv * T) != 0.0, "Invalid gamma or cv or temperature detected!");
  rho = (p + _p_inf) / ((_gamma - 1.0) * _cv * T);
  if (!_allow_nonphysical_states && rho <= 0.)
  {
    drho_dp = getNaN();
    drho_dT = getNaN();
  }
  else
  {
    drho_dp = 1. / ((_gamma - 1.0) * _cv * T);
    drho_dT = -(p + _p_inf) / ((_gamma - 1.0) * _cv * T * T);
  }
}

template <typename CppType>
CppType
StiffenedGasFluidProperties::e_from_p_rho_template(const CppType & p, const CppType & rho) const
{
  mooseAssert((_gamma - 1.0) * rho != 0., "Invalid gamma or density detected!");
  return (p + _gamma * _p_inf) / ((_gamma - 1.0) * rho) + _q;
}

template <typename CppType>
void
StiffenedGasFluidProperties::e_from_p_rho_template(
    const CppType & p, const CppType & rho, CppType & e, CppType & de_dp, CppType & de_drho) const
{
  e = e_from_p_rho_template(p, rho);
  de_dp = 1.0 / ((_gamma - 1.0) * rho);
  de_drho = -(p + _gamma * _p_inf) / ((_gamma - 1.0) * rho * rho);
}

template <typename CppType>
CppType
StiffenedGasFluidProperties::T_from_v_e_template(const CppType & v, const CppType & e) const
{
  return (1.0 / _cv) * (e - _q - _p_inf * v);
}

template <typename CppType>
void
StiffenedGasFluidProperties::T_from_v_e_template(
    const CppType & v, const CppType & e, CppType & T, CppType & dT_dv, CppType & dT_de) const
{
  T = T_from_v_e_template(v, e);
  dT_dv = -_p_inf / _cv;
  dT_de = 1.0 / _cv;
}

template <typename CppType>
CppType
StiffenedGasFluidProperties::T_from_p_h_template(const CppType & /*p*/, const CppType & h) const
{
  return (1.0 / _cv) * (h - _q) / _gamma;
}

template <typename CppType>
void
StiffenedGasFluidProperties::T_from_p_h_template(
    const CppType & p, const CppType & h, CppType & T, CppType & dT_dp, CppType & dT_dh) const
{
  T = T_from_p_h_template(p, h);
  dT_dp = 0;
  dT_dh = 1.0 / _cv / _gamma;
}

template <typename CppType>
CppType
StiffenedGasFluidProperties::p_from_v_e_template(const CppType & v, const CppType & e) const
{
  return (_gamma - 1.0) * (e - _q) / v - _gamma * _p_inf;
}

template <typename CppType>
void
StiffenedGasFluidProperties::p_from_v_e_template(
    const CppType & v, const CppType & e, CppType & p, CppType & dp_dv, CppType & dp_de) const
{
  p = p_from_v_e_template(v, e);
  dp_dv = -(_gamma - 1.0) * (e - _q) / v / v;
  dp_de = (_gamma - 1.0) / v;
}

template <typename CppType>
CppType
StiffenedGasFluidProperties::s_from_v_e_template(const CppType & v, const CppType & e) const
{
  CppType T = T_from_v_e_template(v, e);
  CppType p = p_from_v_e_template(v, e);
  CppType n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0);
  if (n <= 0.0)
    return getNaN();
  else
    return _cv * std::log(n) + _q_prime;
}

template <typename CppType>
void
StiffenedGasFluidProperties::s_from_v_e_template(
    const CppType & v, const CppType & e, CppType & s, CppType & ds_dv, CppType & ds_de) const
{
  CppType T, dT_dv, dT_de;
  T_from_v_e_template(v, e, T, dT_dv, dT_de);

  CppType p, dp_dv, dp_de;
  p_from_v_e_template(v, e, p, dp_dv, dp_de);

  const CppType n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0);
  if (n <= 0.0)
  {
    s = getNaN();
    ds_dv = getNaN();
    ds_de = getNaN();
  }
  else
  {
    s = _cv * std::log(n) + _q_prime;

    const CppType dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p + _p_inf, _gamma - 1.0);
    const CppType dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p + _p_inf, -_gamma);

    const CppType dn_dv = dn_dT * dT_dv + dn_dp * dp_dv;
    const CppType dn_de = dn_dT * dT_de + dn_dp * dp_de;

    ds_dv = _cv / n * dn_dv;
    ds_de = _cv / n * dn_de;
  }
}

template <typename CppType>
CppType
StiffenedGasFluidProperties::s_from_p_T_template(const CppType & p, const CppType & T) const
{
  CppType n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0);
  if (n <= 0.0)
    return getNaN();
  else
    return _cv * std::log(n) + _q_prime;
}

template <typename CppType>
void
StiffenedGasFluidProperties::s_from_p_T_template(
    const CppType & p, const CppType & T, CppType & s, CppType & ds_dp, CppType & ds_dT) const
{
  const CppType n = std::pow(T, _gamma) / std::pow(p + _p_inf, _gamma - 1.0);
  if (n <= 0.0)
  {
    s = getNaN();
    ds_dp = getNaN();
    ds_dT = getNaN();
  }
  else
  {
    s = _cv * std::log(n) + _q_prime;

    const CppType dn_dT = _gamma * std::pow(T, _gamma - 1.0) / std::pow(p + _p_inf, _gamma - 1.0);
    const CppType dn_dp = std::pow(T, _gamma) * (1.0 - _gamma) * std::pow(p + _p_inf, -_gamma);

    ds_dp = _cv / n * dn_dp;
    ds_dT = _cv / n * dn_dT;
  }
}
