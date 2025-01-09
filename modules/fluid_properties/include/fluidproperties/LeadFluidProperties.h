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
 * Fluid properties for (Lead) \cite Fazio.
 */
class LeadFluidProperties : public SinglePhaseFluidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();

  LeadFluidProperties(const InputParameters & parameters);

  virtual std::string fluidName() const override;
  Real molarMass() const override;

  using SinglePhaseFluidProperties::p_from_v_e;
  Real p_from_v_e(Real v, Real e) const override;
  void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;

  using SinglePhaseFluidProperties::T_from_v_e;
  Real T_from_v_e(Real v, Real e) const override;
  void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const override;

  using SinglePhaseFluidProperties::T_from_p_h;
  Real T_from_p_h(Real p, Real h) const override;
  void T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const override;

  /**
   * Temperature from pressure and density
   *
   * @param p     pressure (Pa)
   * @param rho   density (kg/m$^3$)
   * @return temperature (K)
   */
  Real T_from_p_rho(Real p, Real rho) const;
  void T_from_p_rho(Real p, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const;

  Real cp_from_v_e(Real v, Real e) const override;
  void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;

  Real cv_from_v_e(Real v, Real e) const override;
  void cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;

  Real cv_from_p_T(Real p, Real T) const override;
  void cv_from_p_T(Real p, Real T, Real & cv, Real & dcp_dp, Real & dcp_dT) const override;

  Real mu_from_v_e(Real v, Real e) const override;
  void mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const override;

  Real k_from_v_e(Real v, Real e) const override;
  void k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const override;

  using SinglePhaseFluidProperties::rho_from_p_T;
  Real rho_from_p_T(Real p, Real T) const override;
  void rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  void rho_from_p_T(const ADReal & p,
                    const ADReal & T,
                    ADReal & rho,
                    ADReal & drho_dp,
                    ADReal & drho_dT) const override;

  Real v_from_p_T(Real p, Real T) const override;

  void v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const override;

  Real h_from_p_T(Real p, Real T) const override;
  void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;

  Real h_from_v_e(Real v, Real e) const override;
  void h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const override;

  Real e_from_p_T(Real p, Real T) const override;

  Real e_from_p_rho(Real p, Real rho) const override;
  void e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const override;
  void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;

  Real cp_from_p_T(Real p, Real T) const override;
  void cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  Real k_from_p_T(Real p, Real T) const override;
  void k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const override;

  Real mu_from_p_T(Real p, Real T) const override;
  void mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_drho, Real & dmu_dT) const override;

  /**
   * Isentropic bulk modulus from pressure and temperature
   *
   * @param p   pressure (Pa)
   * @param T   temperature (K)
   * @return Bulk Modules (N/m^2)
   */
  Real bulk_modulus_from_p_T(Real p, Real T) const;

  Real c_from_v_e(Real v, Real e) const override;
  ADReal c_from_v_e(const ADReal & v, const ADReal & e) const override;

private:
  /// Melting temperature of Lead
  static constexpr Real _T_mo = 600.6;
};
#pragma GCC diagnostic pop
