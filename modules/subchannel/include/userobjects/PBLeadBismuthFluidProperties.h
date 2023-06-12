#ifndef PBLEADBISMUTHFLUIDPROPERTIES_H
#define PBLEADBISMUTHFLUIDPROPERTIES_H

#include "SinglePhaseFluidProperties.h"

class PBLeadBismuthFluidProperties : public SinglePhaseFluidProperties
{
public:
  PBLeadBismuthFluidProperties(const InputParameters & parameters);
  // virtual ~PBLeadBismuthFluidProperties();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual Real rho_from_p_T(Real pressure, Real temperature) const;
  virtual void
  rho_from_p_T(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const;
  virtual Real h_from_p_T(Real pressure, Real temperature) const;
  virtual Real T_from_p_h(Real temperature, Real enthalpy) const;

  virtual Real beta_from_p_T(Real pressure, Real temperature) const;
  virtual Real cv_from_p_T(Real pressure, Real temperature) const;
  virtual Real cp_from_p_T(Real pressure, Real temperature) const;
  virtual void
  cp_from_p_T(Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const;
  virtual Real mu_from_p_T(Real pressure, Real temperature) const;
  virtual Real k_from_p_T(Real pressure, Real temperature) const;

  virtual Real tm_from_p_T(Real pressure, Real temperature) const;
  virtual Real dH_from_p_T(Real pressure, Real temperature) const;

#pragma GCC diagnostic pop

protected:
  static const std::vector<Real> _temperature_lead_vec;
  static const std::vector<Real> _e_lead_vec;
  static const std::vector<Real> _temperature_bismuth_vec;
  static const std::vector<Real> _e_bismuth_vec;
  static const std::vector<Real> _temperature_lbe_vec;
  static const std::vector<Real> _e_lbe_vec;

  Real temperature_correction(Real & temperature) const;
  MooseEnum metal_type;

  Real _T0;
  Real _H0;
  Real _beta0;
  Real _dH;

  const Real & _p_0;
  Real _Tm, _Tb;
  Real _C1_rho, _C0_rho;
  Real _C2_k, _C1_k, _C0_k;
  Real _C1_mu, _C0_mu;
  Real _C3_cp, _C2_cp, _C1_cp, _C0_cp;
  Real _C3_h, _C2_h, _C1_h, _C0_h;
  Real _H_Tmax;
  Real _H_Tmin;
  Real _Cp_Tmax;
  Real _Cp_Tmin;

public:
  static InputParameters validParams();
};

#endif
