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
 * Class that contains the equations of state for single-phase liquid sodium.
 */
class PBSodiumFluidProperties : public SinglePhaseFluidProperties
{
public:
  PBSodiumFluidProperties(const InputParameters & parameters);

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
  virtual Real mu_from_rho_T(Real rho, Real temprature) const override;
  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual Real T_from_p_h(Real temperature, Real enthalpy) const override;

#pragma GCC diagnostic pop

protected:
  static const std::vector<Real> _temperature_vec;
  static const std::vector<Real> _e_vec;

  Real temperature_correction(Real & temperature) const;
  Real F_enthalpy(Real temperature) const;

  const Real & _p_0;
  Real _T0;
  Real _Tmax;
  Real _Tmin;
  Real _H0;
  Real _H_Tmax;
  Real _H_Tmin;
  Real _Cp_Tmax;
  Real _Cp_Tmin;

public:
  static InputParameters validParams();
};
