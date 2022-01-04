#include "OneDStagnationPandTBase.h"
#include "SinglePhaseFluidProperties.h"

OneDStagnationPandTBase::OneDStagnationPandTBase(const SinglePhaseFluidProperties & fp) : _fp(fp) {}

void
OneDStagnationPandTBase::rho_p_from_p0_T0_vel(Real p0, Real T0, Real vel, Real & rho, Real & p)
{
  // compute stagnation density and internal energy
  const Real rho0 = _fp.rho_from_p_T(p0, T0);
  const Real e0 = _fp.e_from_p_rho(p0, rho0);
  const Real v0 = 1.0 / rho0;

  // compute stagnation and static enthalpy
  const Real h0 = _fp.h_from_p_T(p0, T0);
  const Real h = h0 - 0.5 * vel * vel;

  // compute entropy (which is equivalent to stagnation entropy by definition of the stagnation
  // state)
  const Real s = _fp.s_from_v_e(v0, e0);

  // compute pressure from enthalpy and entropy
  p = _fp.p_from_h_s(h, s);

  // compute density
  rho = _fp.rho_from_p_s(p, s);
}

Real
OneDStagnationPandTBase::dpdu_from_p0_T0_vel(Real p0, Real T0, Real vel)
{
  // compute stagnation density and internal energy
  const Real rho0 = _fp.rho_from_p_T(p0, T0);
  const Real e0 = _fp.e_from_p_rho(p0, rho0);
  const Real v0 = 1.0 / rho0;

  // compute stagnation and static enthalpy
  const Real h0 = _fp.h_from_p_T(p0, T0);
  const Real h = h0 - 0.5 * vel * vel;
  const Real dh_du = -vel;

  // compute entropy (which is equivalent to stagnation entropy by definition of the stagnation
  // state)
  const Real s = _fp.s_from_v_e(v0, e0);

  Real p, dp_dh, dp_ds;
  _fp.p_from_h_s(h, s, p, dp_dh, dp_ds);

  return dp_dh * dh_du;
}

Real
OneDStagnationPandTBase::drhodu_from_p0_T0_vel(Real p0, Real T0, Real vel)
{
  // compute stagnation density and internal energy
  const Real rho0 = _fp.rho_from_p_T(p0, T0);
  const Real e0 = _fp.e_from_p_rho(p0, rho0);
  const Real v0 = 1.0 / rho0;

  // compute stagnation and static enthalpy
  const Real h0 = _fp.h_from_p_T(p0, T0);
  const Real h = h0 - 0.5 * vel * vel;

  // compute entropy
  const Real s = _fp.s_from_v_e(v0, e0);

  // compute static pressure
  Real p, dp_dh, dp_ds;
  _fp.p_from_h_s(h, s, p, dp_dh, dp_ds);

  // compute drho/dp
  Real rho, drho_dp, drho_ds;
  _fp.rho_from_p_s(p, s, rho, drho_dp, drho_ds);

  return -vel * dp_dh * drho_dp;
}

Real
OneDStagnationPandTBase::dEdu_from_p0_T0_vel(Real p0, Real T0, Real vel)
{
  // compute stagnation density and internal energy
  const Real rho0 = _fp.rho_from_p_T(p0, T0);
  const Real e0 = _fp.e_from_p_rho(p0, rho0);
  const Real v0 = 1.0 / rho0;

  // compute stagnation and static enthalpy
  const Real h0 = _fp.h_from_p_T(p0, T0);
  const Real h = h0 - 0.5 * vel * vel;
  const Real dh_du = -vel;

  // compute entropy and static pressure
  const Real s = _fp.s_from_v_e(v0, e0);

  // compute static pressure
  Real p, dp_dh, dp_ds;
  _fp.p_from_h_s(h, s, p, dp_dh, dp_ds);
  const Real dp_du = dp_dh * dh_du;

  // compute rho, drho/dp, and de/dp
  Real rho, drho_dp, drho_ds_dummy;
  _fp.rho_from_p_s(p, s, rho, drho_dp, drho_ds_dummy);

  // compute e, de/dp and de/drho
  Real e, de_dp, de_drho;
  _fp.e_from_p_rho(p, rho, e, de_dp, de_drho);

  return de_dp * dp_du + de_drho * drho_dp * dp_du + vel;
}
