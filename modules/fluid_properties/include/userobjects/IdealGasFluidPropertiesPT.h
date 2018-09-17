//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IDEALGASFLUIDPROPERTIESPT_H
#define IDEALGASFLUIDPROPERTIESPT_H

#include "SinglePhaseFluidPropertiesPT.h"

class IdealGasFluidPropertiesPT;

template <>
InputParameters validParams<IdealGasFluidPropertiesPT>();

/**
 * Ideal gas fluid properties for (pressure, temperature) variables.
 * Default parameters are for air at atmospheric pressure and temperature.
 */
class IdealGasFluidPropertiesPT : public SinglePhaseFluidPropertiesPT
{
public:
  IdealGasFluidPropertiesPT(const InputParameters & parameters);
  virtual ~IdealGasFluidPropertiesPT();

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;

  virtual Real cv_from_p_T(Real pressure, Real temperature) const override;

  virtual Real c_from_p_T(Real pressure, Real temperature) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real s_from_p_T(Real pressure, Real temperature) const override;
  virtual void s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const override;

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;

  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  e_from_p_T(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual void rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const override;

  virtual void rho_mu_dpT(Real pressure,
                          Real temperature,
                          Real & rho,
                          Real & drho_dp,
                          Real & drho_dT,
                          Real & mu,
                          Real & dmu_dp,
                          Real & dmu_dT) const override;

  virtual Real h_from_p_T(Real p, Real T) const override;

  virtual void
  h_from_p_T(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real henryConstant(Real temperature) const override;

  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

protected:
  /// molar mass
  const Real _molar_mass;
  /// specific heat at constant volume
  const Real _cv;
  /// specific heat at constant pressure
  const Real _cp;
  /// thermal conductivity
  const Real _thermal_conductivity;
  /// specific entropy
  const Real _specific_entropy;
  /// viscosity
  const Real _viscosity;
  /// Henry constant
  const Real _henry_constant;
};

#endif /* IDEALGASFLUIDPROPERTIESPT_H */
