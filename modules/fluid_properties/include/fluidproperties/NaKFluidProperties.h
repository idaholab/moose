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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * NaK fluid properties as a function of pressure (Pa) and temperature (K).
 *
 * Source for fluid properties
 * - density; thermal diffusivity; dynamic viscosity; specific heat capacity:
 * SODIUM-NaK ENGINEERING HANDBOOK Volume I
 * Sodium Chemistry and Physical Properties
 * O. J. FOUST, Editor
 * ISBN 0 677 03020 4
 */
class NaKFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  NaKFluidProperties(const InputParameters & parameters);
  virtual ~NaKFluidProperties();

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

  virtual Real mu_from_p_T(Real p, Real T) const override;
  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real h_from_p_T(Real pressure, Real temperature) const override;
  virtual void
  h_from_p_T(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

protected:
  /// NaK molar mass (kg/mol)
  const Real _MNaK;

  /// K molar fraction
  Real _Nk;
};

#pragma GCC diagnostic pop
