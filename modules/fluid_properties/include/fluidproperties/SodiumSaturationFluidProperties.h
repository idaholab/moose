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

/**
 * Fluid properties for liquid sodium at saturation conditions \cite{sas} \cite{fink}.
 */
class SodiumSaturationFluidProperties : public SinglePhaseFluidProperties
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
public:
  static InputParameters validParams();

  SodiumSaturationFluidProperties(const InputParameters & parameters);

  virtual std::string fluidName() const override;

  virtual Real rho_from_p_T(Real p, Real T) const override;

  virtual void
  rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;
  virtual void rho_from_p_T(const ADReal & p,
                            const ADReal & T,
                            ADReal & rho,
                            ADReal & drho_dp,
                            ADReal & drho_dT) const override;

  virtual Real rho_from_p_s(Real p, Real s) const override;
  virtual void
  rho_from_p_s(Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const override;

  virtual Real v_from_p_T(Real p, Real T) const override;
  virtual void v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const override;

  virtual Real p_from_v_e(Real v, Real e) const override;
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;

  virtual Real T_from_v_e(Real v, Real e) const override;
  virtual void T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const override;

  virtual Real c_from_v_e(Real v, Real e) const override;
  virtual void c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const override;

  virtual Real h_from_p_T(Real p, Real T) const override;
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real T_from_p_h(Real p, Real h) const override;
  virtual void T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const override;

  virtual Real s_from_p_T(Real p, Real T) const override;
  virtual void s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const override;

  virtual Real s_from_v_e(Real v, Real e) const override;
  virtual void s_from_v_e(Real v, Real e, Real & s, Real & ds_dv, Real & ds_de) const override;

  virtual Real e_from_p_T(Real p, Real T) const override;
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual Real e_from_p_rho(Real p, Real rho) const override;
  virtual void
  e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const override;

  virtual Real cp_from_p_T(Real p, Real T) const override;
  virtual void cp_from_p_T(Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const override;

  virtual Real cp_from_v_e(Real v, Real e) const override;
  virtual void cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const override;

  using SinglePhaseFluidProperties::cv_from_p_T;

  virtual Real cv_from_p_T(Real p, Real T) const override;
  virtual void cv_from_p_T(Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const override;

  virtual Real cv_from_v_e(Real v, Real e) const override;
  virtual void cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const override;

  virtual Real molarMass() const override;

  virtual Real k_from_p_T(Real p, Real T) const override;
  virtual void k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real k_from_v_e(Real v, Real e) const override;
  virtual void k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const override;

  virtual Real mu_from_p_T(Real p, Real T) const override;
  virtual void
  mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_drho, Real & dmu_dT) const override;

  virtual Real mu_from_v_e(Real v, Real e) const override;
  virtual void mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const override;

private:
  static constexpr Real _reference_pressure = 1.0e5;

  void specific_volume_derivatives(
      Real temperature, Real & v, Real & dv_dT, Real & d2v_dT2, Real & d3v_dT3) const;
  Real cp0_from_T(Real temperature) const;
  Real dcp0_dT_from_T(Real temperature) const;
};
#pragma GCC diagnostic pop
