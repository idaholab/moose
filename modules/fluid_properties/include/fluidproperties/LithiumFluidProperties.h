//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Liquid lithium fluid properties as a function of pressure (Pa) and temperature (K).
 *
 * Source for fluid properties:
 * R. W. Ohse (Ed.), Handbook of Thermodynamic and Transport Properties of Alkali Metals,
 * IUPAC Chemical Data Series No. 30, Blackwell, 1985, as tabulated in
 * S. J. Zinkle, Summary of Physical Properties for Lithium, Pb-17Li, and (LiF)n-BeF2 Coolants,
 * APEX Study Meeting, 1998
 */
class LithiumFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  LithiumFluidProperties(const InputParameters & parameters);

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real T_from_p_h(Real pressure, Real enthalpy) const override;

  Real T_from_p_rho(Real pressure, Real density) const;

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;
  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  e_from_p_T(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual Real e_from_p_rho(Real pressure, Real density) const override;

  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;
  virtual void cp_from_p_T(
      Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  virtual Real cv_from_p_T(Real pressure, Real temperature) const override;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;
  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real h_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  h_from_p_T(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

protected:
  /// Flags an invalid solution if the temperature is outside of the range of the correlations
  void checkTemperature(Real temperature, const std::string & property) const;

  /// Antiderivative of the specific heat correlation (J/kg)
  static Real cpIntegral(Real temperature);

  /// Melting temperature of lithium (K), also the reference temperature for the enthalpy
  static constexpr Real _T_melt = 454.;
  /// Lower and upper temperatures (K) of the validity range of the correlations
  static constexpr Real _T_min = 455.;
  static constexpr Real _T_max = 1500.;
  /// Molar mass of lithium (kg/mol)
  static constexpr Real _M = 6.941e-3;
};

#pragma GCC diagnostic pop
