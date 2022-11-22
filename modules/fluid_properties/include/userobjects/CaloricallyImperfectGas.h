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
#include "Function.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * A calorically imperfect gas fluid property class
 * This fluid property assumes that internal energy is
 * a general monotonic function of temperature; behaves
 * like an ideal gas otherwise. In particular, it assumes:
 * p v = R_s T
 */
class CaloricallyImperfectGas : public SinglePhaseFluidProperties, public NaNInterface
{
public:
  static InputParameters validParams();

  CaloricallyImperfectGas(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual Real p_from_v_e(Real v, Real e) const override;
  virtual ADReal p_from_v_e(const ADReal & v, const ADReal & e) const override;
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;
  virtual Real T_from_v_e(Real v, Real e) const override;
  virtual ADReal T_from_v_e(const ADReal & v, const ADReal & e) const override;
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const override;

  virtual Real cp_from_v_e(Real v, Real e) const override;
  virtual void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;
  virtual Real cv_from_v_e(Real v, Real e) const override;
  virtual ADReal cv_from_v_e(ADReal v, ADReal e) const;
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
  propfuncWithDefinitionOverride(e, p, rho);
  virtual Real e_from_T_v(Real T, Real v) const override;
  virtual void e_from_T_v(Real T, Real v, Real & e, Real & de_dT, Real & de_dv) const override;
  virtual ADReal e_from_T_v(const ADReal & T, const ADReal & v) const override;
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
  virtual ADReal e_from_p_T(ADReal p, ADReal T) const;
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;
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
  virtual ADReal gamma_from_v_e(ADReal v, ADReal e) const;
  virtual void
  gamma_from_v_e(Real v, Real e, Real & gamma, Real & dgamma_dv, Real & dgamma_de) const override;
  virtual Real gamma_from_p_T(Real p, Real T) const override;
  virtual ADReal gamma_from_p_T(ADReal p, ADReal T) const;
  virtual void
  gamma_from_p_T(Real p, Real T, Real & gamma, Real & dgamma_dp, Real & dgamma_dT) const override;
  virtual Real g_from_v_e(Real v, Real e) const override;
  virtual Real p_from_h_s(Real h, Real s) const override;
  virtual void p_from_h_s(Real h, Real s, Real & p, Real & dp_dh, Real & dp_ds) const override;
  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual ADReal c_from_v_e(const ADReal & v, const ADReal & e) const override;
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;
  virtual Real c_from_p_T(Real p, Real T) const override;
  virtual ADReal c_from_p_T(const ADReal & p, const ADReal & T) const override;
  virtual void c_from_p_T(Real /*p*/, Real T, Real & c, Real & dc_dp, Real & dc_dT) const override;

protected:
  /// sets up the T(e) reverse lookup table
  void setupLookupTables();

  /// function that handles exceeding parameter limits
  void outOfBounds(const std::string & function, Real value, Real min, Real max) const;

  ///@{ helper functions for e(T) and h(T), T(e), T(h), cv(T), cp(T)
  Real e_from_T(Real T) const;
  Real h_from_T(Real T) const;
  Real T_from_e(Real e) const;
  Real T_from_h(Real h) const;
  template <typename CppType>
  Real cv_from_T(const CppType & T) const;
  Real cp_from_T(Real T) const;
  void cv_from_T(Real T, Real & cv, Real & dcv_dT) const;
  void cp_from_T(Real T, Real & cp, Real & dcp_dT) const;
  Real Z_from_T(Real T) const;
  ///@}

  /// molar mass
  const Real & _molar_mass;

  /// Specific gas constant (R / molar mass)
  const Real _R_specific;

  // properties at critical point (used by IdealRealGasMixtureFluidProperties (primary component))
  Real _T_c;
  Real _rho_c;
  Real _e_c;

  ///@{ temperature limits when creating lookup tables
  Real _min_temperature;
  Real _max_temperature;
  ///@}

  /// temperature interval in lookup tables
  Real _delta_T;

  /// Flag to error if out of bounds
  const bool _out_of_bound_error;

  // tolerance for checking monotonicity of T for h(T) & e(T)
  Real _tol;

  /// Internal energy as a function of temperature
  const Function * _e_T;
  /// Dynamic viscosity
  const Function * _mu_T;
  /// Thermal conductivity
  const Function * _k_T;

  ///@{ internal energy and enthalpy limits when creating lookup tables
  Real _min_e;
  Real _max_e;
  Real _min_h;
  Real _max_h;
  ///@}

  ///@{ step size in internal energy and enthalpy
  Real _delta_e;
  Real _delta_h;
  ///@}

  ///@{ inverse lookup table data
  std::vector<Real> _T_h_lookup;
  std::vector<Real> _T_e_lookup;
  ///@}

  /// Z(T) lookup table on uniform grid between _min_temperature and _max_temperature
  std::vector<Real> _Z_T_lookup;
};

#pragma GCC diagnostic pop

template <typename CppType>
CppType
CaloricallyImperfectGas::e_from_p_rho_template(const CppType & p, const CppType & rho) const
{
  CppType T = p / (rho * _R_specific);
  return e_from_p_T(p, T);
}

template <typename CppType>
void
CaloricallyImperfectGas::e_from_p_rho_template(
    const CppType & p, const CppType & rho, CppType & e, CppType & de_dp, CppType & de_drho) const
{
  CppType T = p / (rho * _R_specific);
  e = e_from_p_rho_template(p, rho);
  CppType cv = cv_from_T(T);
  de_dp = cv / (rho * _R_specific);
  de_drho = -cv * p / (rho * rho * _R_specific);
}

template <typename CppType>
Real
CaloricallyImperfectGas::cv_from_T(const CppType & T) const
{
  const auto raw_T = MetaPhysicL::raw_value(T);
  if (raw_T < _min_temperature || raw_T > _max_temperature)
    outOfBounds("cv_from_T", raw_T, _min_temperature, _max_temperature);

  return _e_T->timeDerivative(raw_T, Point());
}
