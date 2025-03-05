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
 * Class that implements the equations of state for single phase liquid sodium
 */
class PBSodiumFluidProperties : public SinglePhaseFluidProperties
{
public:
  PBSodiumFluidProperties(const InputParameters & parameters);
  using SinglePhaseFluidProperties::beta_from_p_T;
  using SinglePhaseFluidProperties::cp_from_p_T;
  using SinglePhaseFluidProperties::cv_from_p_T;
  using SinglePhaseFluidProperties::h_from_p_T;
  using SinglePhaseFluidProperties::k_from_p_T;
  using SinglePhaseFluidProperties::mu_from_p_T;
  using SinglePhaseFluidProperties::mu_from_rho_T;
  using SinglePhaseFluidProperties::rho_from_p_T;
  using SinglePhaseFluidProperties::T_from_p_h;

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;
  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual Real h_from_p_T(Real pressure, Real temperature) const override;

  virtual Real beta_from_p_T(Real pressure, Real temperature) const override;
  virtual Real cv_from_p_T(Real pressure, Real temperature) const override;
  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;
  virtual void cp_from_p_T(
      Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;
  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;
  virtual Real mu_from_rho_T(Real rho, Real temperature) const override;
  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual Real T_from_p_h(Real pressure, Real enthalpy) const override;

protected:
  static const std::vector<Real> _temperature_vec;
  static const std::vector<Real> _e_vec;

  Real temperature_correction(Real & temperature) const;
  Real F_enthalpy(Real temperature) const;

  /// reference pressure
  const Real & _p_0;
  /// reference temperature
  static constexpr Real _T0 = 628.15;
  /// max temperature
  static constexpr Real _Tmax = 1154.55;
  /// min temperature
  static constexpr Real _Tmin = 373.15;
  /// reference enthalpy
  Real _H0;
  /// max enthalpy
  Real _H_Tmax;
  /// min enthalpy
  Real _H_Tmin;
  /// max cp
  Real _Cp_Tmax;
  /// min cp
  Real _Cp_Tmin;

public:
  static InputParameters validParams();
};
