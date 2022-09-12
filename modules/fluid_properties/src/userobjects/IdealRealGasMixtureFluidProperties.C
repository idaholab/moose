//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealRealGasMixtureFluidProperties.h"
#include "SinglePhaseFluidProperties.h"
#include "BrentsMethod.h"
#include <numeric>

registerMooseObject("FluidPropertiesApp", IdealRealGasMixtureFluidProperties);

InputParameters
IdealRealGasMixtureFluidProperties::validParams()
{
  InputParameters params = VaporMixtureFluidProperties::validParams();
  params += NaNInterface::validParams();

  params.addClassDescription("Class for fluid properties of an arbitrary vapor mixture");

  params.addRequiredParam<UserObjectName>(
      "fp_primary", "Name of fluid properties user object for primary vapor component");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp_secondary", "Name of fluid properties user object(s) for secondary vapor component(s)");
  params.addParam<Real>("_T_mix_max", 1300., "Maximum temperature of the mixture");

  // This is necessary because initialize() must be called before any interface
  // can be used (which can occur as early as initialization of variables).
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  return params;
}

IdealRealGasMixtureFluidProperties::IdealRealGasMixtureFluidProperties(
    const InputParameters & parameters)
  : VaporMixtureFluidProperties(parameters),
    NaNInterface(this),
    _fp_primary(&getUserObject<SinglePhaseFluidProperties>("fp_primary")),
    _fp_secondary_names(getParam<std::vector<UserObjectName>>("fp_secondary")),
    _n_secondary_vapors(_fp_secondary_names.size()),
    _T_mix_max(getParam<Real>("_T_mix_max"))
{
  _fp_secondary.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    _fp_secondary[i] = &getUserObjectByName<SinglePhaseFluidProperties>(_fp_secondary_names[i]);
}

Real
IdealRealGasMixtureFluidProperties::p_from_v_e(Real v, Real e, const std::vector<Real> & x) const
{
  Real p, T;
  p_T_from_v_e(v, e, x, p, T);

  return p;
}

void
IdealRealGasMixtureFluidProperties::p_from_v_e(Real v,
                                               Real e,
                                               const std::vector<Real> & x,
                                               Real & p,
                                               Real & dp_dv,
                                               Real & dp_de,
                                               std::vector<Real> & dp_dx) const
{
  Real T, dT_dv, dT_de;
  std::vector<Real> dT_dx(_n_secondary_vapors);
  p_T_from_v_e(v, e, x, p, dp_dv, dp_de, dp_dx, T, dT_dv, dT_de, dT_dx);
}

Real
IdealRealGasMixtureFluidProperties::T_from_v_e(Real v, Real e, const std::vector<Real> & x) const
{
  Real p, T;
  p_T_from_v_e(v, e, x, p, T);

  return T;
}

void
IdealRealGasMixtureFluidProperties::T_from_v_e(Real v,
                                               Real e,
                                               const std::vector<Real> & x,
                                               Real & T,
                                               Real & dT_dv,
                                               Real & dT_de,
                                               std::vector<Real> & dT_dx) const
{
  Real p, dp_dv, dp_de;
  std::vector<Real> dp_dx(_n_secondary_vapors);
  p_T_from_v_e(v, e, x, p, dp_dv, dp_de, dp_dx, T, dT_dv, dT_de, dT_dx);
}

Real
IdealRealGasMixtureFluidProperties::c_from_v_e(Real v, Real e, const std::vector<Real> & x) const
{
  Real p, T;
  p_T_from_v_e(v, e, x, p, T);
  return c_from_T_v(T, v, x);
}

void
IdealRealGasMixtureFluidProperties::c_from_v_e(Real v,
                                               Real e,
                                               const std::vector<Real> & x,
                                               Real & c,
                                               Real & dc_dv,
                                               Real & dc_de,
                                               std::vector<Real> & dc_dx) const
{
  Real p, T;
  p_T_from_v_e(v, e, x, p, T);

  // sound of speed and derivatives
  Real dc_dT_v, dc_dv_T;
  std::vector<Real> dc_dx_Tv;
  c_from_T_v(T, v, x, c, dc_dT_v, dc_dv_T, dc_dx_Tv);

  // internal energy and derivatives
  Real e_unused, de_dT_v, de_dv_T;
  std::vector<Real> de_dx_Tv;
  e_from_T_v(T, v, x, e_unused, de_dT_v, de_dv_T, de_dx_Tv);

  // Compute derivatives using the following rules:
  dc_dv = dc_dv_T - dc_dT_v * de_dv_T / de_dT_v;
  dc_de = dc_dT_v / de_dT_v;

  // Derivatives with respect to mass fractions:
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    dc_dx[i] = dc_dx_Tv[i] - dc_dT_v * de_dx_Tv[i] / de_dT_v;
  }
}

Real
IdealRealGasMixtureFluidProperties::rho_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  return 1.0 / v_from_p_T(p, T, x);
}

void
IdealRealGasMixtureFluidProperties::rho_from_p_T(Real p,
                                                 Real T,
                                                 const std::vector<Real> & x,
                                                 Real & rho,
                                                 Real & drho_dp,
                                                 Real & drho_dT,
                                                 std::vector<Real> & drho_dx) const
{
  Real v, dv_dp, dv_dT;
  std::vector<Real> dv_dx;
  v_from_p_T(p, T, x, v, dv_dp, dv_dT, dv_dx);

  rho = 1.0 / v;
  const Real drho_dv = -1.0 / (v * v);
  drho_dp = drho_dv * dv_dp;
  drho_dT = drho_dv * dv_dT;
  drho_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    drho_dx[i] = drho_dv * dv_dx[i];
}

Real
IdealRealGasMixtureFluidProperties::v_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;
  Real v_ideal = R_molar * T / (M_star * p);

  // check range of validity for primary (condensable) component
  static const Real Tc = _fp_primary->criticalTemperature();
  static const Real vc = 1. / _fp_primary->criticalDensity();
  Real v_spndl, e_spndl;
  if (T < Tc)
    _fp_primary->v_e_spndl_from_T(T, v_spndl, e_spndl);
  else
    v_spndl = vc;

  Real lower_spec_volume = v_spndl * x_primary;
  Real upper_spec_volume = v_ideal; // p*v/(RT) <= 1

  // Initial estimate of a bracketing interval for the temperature
  Real p_max = p_from_T_v(T, lower_spec_volume, x);
  if (p > p_max || upper_spec_volume < lower_spec_volume)
    return getNaN();

  // Use BrentsMethod to find temperature
  auto pressure_diff = [&T, &p, &x, this](Real v) { return this->p_from_T_v(T, v, x) - p; };

  BrentsMethod::bracket(pressure_diff, lower_spec_volume, upper_spec_volume);
  Real v = BrentsMethod::root(pressure_diff, lower_spec_volume, upper_spec_volume);

  return v;
}

void
IdealRealGasMixtureFluidProperties::v_from_p_T(Real p,
                                               Real T,
                                               const std::vector<Real> & x,
                                               Real & v,
                                               Real & dv_dp,
                                               Real & dv_dT,
                                               std::vector<Real> & dv_dx) const
{
  Real p_unused, dp_dT, dp_dv;
  std::vector<Real> dp_dx;
  dp_dx.resize(_n_secondary_vapors);

  v = v_from_p_T(p, T, x);
  p_from_T_v(T, v, x, p_unused, dp_dT, dp_dv, dp_dx);

  dv_dp = 1. / dp_dv;
  dv_dT = -dp_dT / dp_dv;

  dv_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    dv_dx[i] = -dp_dx[i] / dp_dv;
}

Real
IdealRealGasMixtureFluidProperties::e_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  Real v = v_from_p_T(p, T, x);
  return e_from_T_v(T, v, x);
}

void
IdealRealGasMixtureFluidProperties::e_from_p_T(Real p,
                                               Real T,
                                               const std::vector<Real> & x,
                                               Real & e,
                                               Real & de_dp,
                                               Real & de_dT,
                                               std::vector<Real> & de_dx) const
{
  Real v, de_dT_v, de_dv_T, p_unused, dp_dT_v, dp_dv_T;
  std::vector<Real> de_dx_Tv, dp_dx_Tv;
  de_dx_Tv.resize(_n_secondary_vapors);
  dp_dx_Tv.resize(_n_secondary_vapors);

  v = v_from_p_T(p, T, x);
  e_from_T_v(T, v, x, e, de_dT_v, de_dv_T, de_dx_Tv);
  p_from_T_v(T, v, x, p_unused, dp_dT_v, dp_dv_T, dp_dx_Tv);

  de_dp = de_dv_T / dp_dv_T;
  de_dT = de_dT_v - de_dv_T * dp_dT_v / dp_dv_T;

  de_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    de_dx[i] = -(de_dv_T - de_dx_Tv[i] * dp_dv_T / dp_dx_Tv[i]) * dp_dx_Tv[i] / dp_dv_T;
}

Real
IdealRealGasMixtureFluidProperties::c_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  Real v;
  Real p_unused, dp_dT, dp_dv;
  Real s, ds_dT, ds_dv;

  v = v_from_p_T(p, T, x);
  p_from_T_v(T, v, x, p_unused, dp_dT, dp_dv);
  s_from_T_v(T, v, x, s, ds_dT, ds_dv);

  Real dp_dv_s = dp_dv - dp_dT * ds_dv / ds_dT;

  if (dp_dv_s >= 0)
    mooseWarning("c_from_p_T(), dp_dv_s = ", dp_dv_s, ". Should be negative.");
  return v * std::sqrt(-dp_dv_s);
}

void
IdealRealGasMixtureFluidProperties::c_from_p_T(Real p,
                                               Real T,
                                               const std::vector<Real> & x,
                                               Real & c,
                                               Real & dc_dp,
                                               Real & dc_dT,
                                               std::vector<Real> & dc_dx) const
{
  Real p_perturbed, T_perturbed, c_perturbed;

  c = c_from_p_T(p, T, x);
  // For derived properties, we would need higher order derivatives;
  // therefore, numerical derivatives are used here
  Real dp = p * 1.e-6;
  p_perturbed = p + dp;
  c_perturbed = c_from_p_T(p_perturbed, T, x);
  dc_dp = (c_perturbed - c) / (dp);

  Real dT = 1.e-6;
  T_perturbed = T + dT;
  c_perturbed = c_from_p_T(p, T_perturbed, x);
  dc_dT = (c_perturbed - c) / (dT);

  dc_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real c_perturbed;
    std::vector<Real> x_perturbed(x);
    Real dx_i = 1e-6;
    for (unsigned int j = 0; j < _n_secondary_vapors; j++)
    {
      if (j != i)
        x_perturbed[j] =
            x[j] * (1.0 - (x[i] + dx_i)) / (1.0 - x[i]); // recalculate new mass fractions
    }
    x_perturbed[i] += dx_i;
    c_perturbed = c_from_p_T(p, T, x_perturbed);
    dc_dx[i] = ((c_perturbed - c) / dx_i);
  }
}

Real
IdealRealGasMixtureFluidProperties::cp_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  Real p_unused, dp_dT, dp_dv;
  Real h, dh_dT, dh_dv;

  Real v = v_from_p_T(p, T, x);
  p_from_T_v(T, v, x, p_unused, dp_dT, dp_dv);

  const Real x_primary = primaryMassFraction(x);

  _fp_primary->h_from_T_v(T, v / x_primary, h, dh_dT, dh_dv);
  Real cp = x_primary * (dh_dT - dh_dv * dp_dT / dp_dv);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->h_from_T_v(T, v / x[i], h, dh_dT, dh_dv);
    cp += x[i] * (dh_dT - dh_dv * dp_dT / dp_dv);
  }

  return cp;
}
void
IdealRealGasMixtureFluidProperties::cp_from_p_T(Real p,
                                                Real T,
                                                const std::vector<Real> & x,
                                                Real & cp,
                                                Real & dcp_dp,
                                                Real & dcp_dT,
                                                std::vector<Real> & dcp_dx) const
{
  Real p_unused, dp_dT, dp_dv;
  Real h, dh_dT, dh_dv;

  Real v = v_from_p_T(p, T, x);
  p_from_T_v(T, v, x, p_unused, dp_dT, dp_dv);

  const Real x_primary = primaryMassFraction(x);

  _fp_primary->h_from_T_v(T, v / x_primary, h, dh_dT, dh_dv);
  const Real cp_xp = (dh_dT - dh_dv * dp_dT / dp_dv);
  cp = x_primary * cp_xp;
  dcp_dT = 0;
  dcp_dp = 0;

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->h_from_T_v(T, v / x[i], h, dh_dT, dh_dv);
    cp += x[i] * (dh_dT - dh_dv * dp_dT / dp_dv);
    dcp_dx[i] = (dh_dT - dh_dv * dp_dT / dp_dv) - cp_xp;
  }
}

Real
IdealRealGasMixtureFluidProperties::cv_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  Real v = v_from_p_T(p, T, x);

  const Real x_primary = primaryMassFraction(x);
  Real cv = x_primary * _fp_primary->cv_from_T_v(T, v / x_primary);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    cv += x[i] * _fp_secondary[i]->cv_from_T_v(T, v / x[i]);

  return cv;
}
void
IdealRealGasMixtureFluidProperties::cv_from_p_T(Real p,
                                                Real T,
                                                const std::vector<Real> & x,
                                                Real & cv,
                                                Real & dcv_dp,
                                                Real & dcv_dT,
                                                std::vector<Real> & dcv_dx) const

{
  Real v = v_from_p_T(p, T, x);

  const Real x_primary = primaryMassFraction(x);
  const Real cv_xp = _fp_primary->cv_from_T_v(T, v / x_primary);
  cv = x_primary * cv_xp;
  dcv_dT = 0;
  dcv_dp = 0;

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    cv += x[i] * _fp_secondary[i]->cv_from_T_v(T, v / x[i]);
    dcv_dx[i] = _fp_secondary[i]->cv_from_T_v(T, v / x[i]) - cv_xp;
  }
}
Real
IdealRealGasMixtureFluidProperties::mu_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  Real v = v_from_p_T(p, T, x);

  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = v / x_primary;
  Real ep = _fp_primary->e_from_T_v(T, vp);
  Real mu = x_primary * M_star / M_primary * _fp_primary->mu_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real vi = v / x[i];
    Real ei = _fp_secondary[i]->e_from_T_v(T, vi);
    Real Mi = _fp_secondary[i]->molarMass();
    mu += x[i] * M_star / Mi * _fp_secondary[i]->mu_from_v_e(vi, ei);
  }

  return mu;
}
void
IdealRealGasMixtureFluidProperties::mu_from_p_T(Real p,
                                                Real T,
                                                const std::vector<Real> & x,
                                                Real & mu,
                                                Real & dmu_dp,
                                                Real & dmu_dT,
                                                std::vector<Real> & dmu_dx) const
{
  Real v = v_from_p_T(p, T, x);

  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = v / x_primary;
  Real ep = _fp_primary->e_from_T_v(T, vp);
  Real mup = _fp_primary->mu_from_v_e(vp, ep);
  mu = x_primary * M_star / M_primary * mup;
  dmu_dT = 0;
  dmu_dp = 0;

  Real sum_muj = 0;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    const Real vi = v / x[i];
    const Real ei = _fp_secondary[i]->e_from_T_v(T, vi);
    const Real Mi = _fp_secondary[i]->molarMass();
    const Real mui = _fp_secondary[i]->mu_from_v_e(vi, ei);
    mu += x[i] * M_star / Mi * mui;
    dmu_dx[i] = -M_star / M_primary * mup -
                x_primary * (1 / Mi - 1 / M_primary) * M_star * M_star / M_primary * mup +
                M_star / Mi * mui;
    sum_muj += x[i] * mui / Mi;
  }
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    const Real Mi = _fp_secondary[i]->molarMass();
    dmu_dx[i] += -M_star * M_star * (1 / Mi - 1 / M_primary) * sum_muj;
  }
}

Real
IdealRealGasMixtureFluidProperties::k_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  Real v = v_from_p_T(p, T, x);

  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = v / x_primary;
  Real ep = _fp_primary->e_from_T_v(T, vp);
  Real k = x_primary * M_star / M_primary * _fp_primary->k_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real vi = v / x[i];
    Real ei = _fp_secondary[i]->e_from_T_v(T, vi);
    Real Mi = _fp_secondary[i]->molarMass();
    k += x[i] * M_star / Mi * _fp_secondary[i]->k_from_v_e(vi, ei);
  }

  return k;
}
void
IdealRealGasMixtureFluidProperties::k_from_p_T(Real p,
                                               Real T,
                                               const std::vector<Real> & x,
                                               Real & k,
                                               Real & dk_dp,
                                               Real & dk_dT,
                                               std::vector<Real> & dk_dx) const
{
  Real v = v_from_p_T(p, T, x);

  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = v / x_primary;
  Real ep = _fp_primary->e_from_T_v(T, vp);
  const Real kp = _fp_primary->k_from_v_e(vp, ep);
  k = x_primary * M_star / M_primary * kp;
  dk_dp = 0;
  dk_dT = 0;

  Real sum_kj = 0;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real vi = v / x[i];
    Real ei = _fp_secondary[i]->e_from_T_v(T, vi);
    Real Mi = _fp_secondary[i]->molarMass();
    Real ki = _fp_secondary[i]->k_from_v_e(vi, ei);
    k += x[i] * M_star / Mi * ki;
    dk_dx[i] = -M_star / M_primary * kp -
               x_primary * (1 / Mi - 1 / M_primary) * M_star * M_star / M_primary * kp +
               M_star / Mi * ki;
    sum_kj += x[i] * ki / Mi;
  }
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real Mi = _fp_secondary[i]->molarMass();
    dk_dx[i] += -M_star * M_star * (1 / Mi - 1 / M_primary) * sum_kj;
  }
}

Real
IdealRealGasMixtureFluidProperties::e_from_p_rho(Real p,
                                                 Real rho,
                                                 const std::vector<Real> & x) const
{
  Real v = 1. / rho;
  Real T = T_from_p_v(p, v, x);

  return e_from_T_v(T, v, x);
}

void
IdealRealGasMixtureFluidProperties::e_from_p_rho(Real p,
                                                 Real rho,
                                                 const std::vector<Real> & x,
                                                 Real & e,
                                                 Real & de_dp,
                                                 Real & de_drho,
                                                 std::vector<Real> & de_dx) const
{
  Real v = 1. / rho;
  Real T = T_from_p_v(p, v, x);
  Real p_, dp_dT, dp_dv, de_dT, de_dv;
  std::vector<Real> dp_dx_Tv;
  std::vector<Real> de_dx_Tv;

  p_from_T_v(T, v, x, p_, dp_dT, dp_dv, dp_dx_Tv);
  e_from_T_v(T, v, x, e, de_dT, de_dv, de_dx_Tv);

  de_dp = de_dT / dp_dT;
  de_drho = (-v * v) * (de_dv - de_dT * dp_dv / dp_dT);

  // Derivatives with respect to mass fractions:
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    de_dx[i] = de_dx_Tv[i] - de_dT * dp_dx_Tv[i] / dp_dT;
  }
}

void
IdealRealGasMixtureFluidProperties::p_T_from_v_e(
    Real v, Real e, const std::vector<Real> & x, Real & p, Real & T) const
{
  Real v_primary = v / primaryMassFraction(x);
  static const Real vc = 1. / _fp_primary->criticalDensity();
  static const Real ec = _fp_primary->criticalInternalEnergy();

  // Initial estimate of a bracketing interval for the temperature
  Real lower_temperature, upper_temperature;
  if (v_primary > vc)
  {
    Real e_sat_primary = _fp_primary->e_spndl_from_v(v_primary);
    lower_temperature = _fp_primary->T_from_v_e(v_primary, e_sat_primary);
  }
  else
    lower_temperature = _fp_primary->T_from_v_e(v_primary, ec);

  upper_temperature = _T_mix_max;

  // Use BrentsMethod to find temperature
  auto energy_diff = [&v, &e, &x, this](Real T) { return this->e_from_T_v(T, v, x) - e; };

  BrentsMethod::bracket(energy_diff, lower_temperature, upper_temperature);
  T = BrentsMethod::root(energy_diff, lower_temperature, upper_temperature);

  p = p_from_T_v(T, v, x);
}

void
IdealRealGasMixtureFluidProperties::p_T_from_v_e(Real v,
                                                 Real e,
                                                 const std::vector<Real> & x,
                                                 Real & p,
                                                 Real & dp_dv,
                                                 Real & dp_de,
                                                 std::vector<Real> & dp_dx,
                                                 Real & T,
                                                 Real & dT_dv,
                                                 Real & dT_de,
                                                 std::vector<Real> & dT_dx) const
{
  p_T_from_v_e(v, e, x, p, T);

  // pressure and derivatives
  Real p_unused, dp_dT_v, dp_dv_T;
  std::vector<Real> dp_dx_Tv;
  p_from_T_v(T, v, x, p_unused, dp_dT_v, dp_dv_T, dp_dx_Tv);

  // internal energy and derivatives
  Real e_unused, de_dT_v, de_dv_T;
  std::vector<Real> de_dx_Tv;
  e_from_T_v(T, v, x, e_unused, de_dT_v, de_dv_T, de_dx_Tv);

  // Compute derivatives using the following rules:
  dp_dv = dp_dv_T - dp_dT_v * de_dv_T / de_dT_v;
  dp_de = dp_dT_v / de_dT_v;
  dT_dv = -de_dv_T / de_dT_v;
  dT_de = 1. / de_dT_v;

  // Derivatives with respect to mass fractions:
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    dT_dx[i] = -de_dx_Tv[i] / de_dT_v;
    dp_dx[i] = dp_dx_Tv[i] + dp_dT_v * dT_dx[i];
  }
}

Real
IdealRealGasMixtureFluidProperties::T_from_p_v(Real p, Real v, const std::vector<Real> & x) const
{
  Real v_primary = v / primaryMassFraction(x);
  static const Real vc = 1. / _fp_primary->criticalDensity();
  static const Real ec = _fp_primary->criticalInternalEnergy();

  // Initial estimate of a bracketing interval for the temperature
  Real lower_temperature, upper_temperature;
  if (v_primary > vc)
  {
    Real e_sat_primary = _fp_primary->e_spndl_from_v(v_primary);
    lower_temperature = _fp_primary->T_from_v_e(v_primary, e_sat_primary);
  }
  else
    lower_temperature = _fp_primary->T_from_v_e(v_primary, ec);

  upper_temperature = _T_mix_max;

  // Use BrentsMethod to find temperature
  auto pressure_diff = [&p, &v, &x, this](Real T) { return this->p_from_T_v(T, v, x) - p; };

  BrentsMethod::bracket(pressure_diff, lower_temperature, upper_temperature);
  Real T = BrentsMethod::root(pressure_diff, lower_temperature, upper_temperature);

  return T;
}

void
IdealRealGasMixtureFluidProperties::T_from_p_v(Real p,
                                               Real v,
                                               const std::vector<Real> & x,
                                               Real & T,
                                               Real & dT_dp,
                                               Real & dT_dv,
                                               std::vector<Real> & dT_dx) const
{
  T = T_from_p_v(p, v, x);

  // pressure and derivatives
  Real p_unused, dp_dT_v, dp_dv_T;
  std::vector<Real> dp_dx_Tv;
  p_from_T_v(T, v, x, p_unused, dp_dT_v, dp_dv_T, dp_dx_Tv);

  // Compute derivatives using the following rules:
  dT_dp = 1. / dp_dT_v;
  dT_dv = -dp_dv_T / dp_dT_v;

  // Derivatives with respect to mass fractions:
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    dT_dx[i] = -dp_dx_Tv[i] / dp_dT_v;
  }
}

Real
IdealRealGasMixtureFluidProperties::p_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real p = _fp_primary->p_from_T_v(T, v / x_primary);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    p += _fp_secondary[i]->p_from_T_v(T, v / x[i]);

  return p;
}

void
IdealRealGasMixtureFluidProperties::p_from_T_v(
    Real T, Real v, const std::vector<Real> & x, Real & p, Real & dp_dT, Real & dp_dv) const
{
  Real p_primary, dp_dT_primary, dp_dv_primary;
  Real p_sec, dp_dT_sec, dp_dv_sec;

  const Real x_primary = primaryMassFraction(x);

  _fp_primary->p_from_T_v(T, v / x_primary, p_primary, dp_dT_primary, dp_dv_primary);
  p = p_primary;
  dp_dT = dp_dT_primary;
  dp_dv = dp_dv_primary / x_primary;

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->p_from_T_v(T, v / x[i], p_sec, dp_dT_sec, dp_dv_sec);

    p += p_sec;
    dp_dT += dp_dT_sec;
    dp_dv += dp_dv_sec / x[i];
  }
}

void
IdealRealGasMixtureFluidProperties::p_from_T_v(Real T,
                                               Real v,
                                               const std::vector<Real> & x,
                                               Real & p,
                                               Real & dp_dT,
                                               Real & dp_dv,
                                               std::vector<Real> & dp_dx) const
{
  Real p_primary, dp_dT_primary, dp_dv_primary, dp_dx_primary, dxi_dx_primary;
  Real p_sec, dp_dT_sec, dxj_dxi;
  std::vector<Real> dp_dv_sec;

  const Real x_primary = primaryMassFraction(x);
  dp_dx.resize(_n_secondary_vapors);
  dp_dv_sec.resize(_n_secondary_vapors);

  _fp_primary->p_from_T_v(T, v / x_primary, p_primary, dp_dT_primary, dp_dv_primary);
  p = p_primary;
  dp_dT = dp_dT_primary;
  dp_dv = dp_dv_primary / x_primary;
  dp_dx_primary = -dp_dv_primary * v / (x_primary * x_primary);

  // get the partial pressures and their derivatives first
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->p_from_T_v(T, v / x[i], p_sec, dp_dT_sec, dp_dv_sec[i]);

    p += p_sec;
    dp_dT += dp_dT_sec;
    dp_dv += dp_dv_sec[i] / x[i];
    dxi_dx_primary = -x[i] / (1. - x_primary);
    dp_dx_primary += -dp_dv_sec[i] * v / (x[i] * x[i]) * dxi_dx_primary;
  }

  // get the composition dependent derivatives of the secondary vapors
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    dp_dx[i] = -dp_dv_sec[i] * v / (x[i] * x[i]);
    for (unsigned int j = 0; j < _n_secondary_vapors; j++)
    {
      if (j == i)
        continue;
      dxj_dxi = -x[j] / (1. - x[i]);
      dp_dx[i] += -dp_dv_sec[j] * v / (x[j] * x[j]) * dxj_dxi;
    }
    dp_dx[i] += -dp_dv_primary * v / (x_primary * x_primary) * (-x_primary / (1. - x[i]));
  }
}

Real
IdealRealGasMixtureFluidProperties::e_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real e = x_primary * _fp_primary->e_from_T_v(T, v / x_primary);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    e += x[i] * _fp_secondary[i]->e_from_T_v(T, v / x[i]);

  return e;
}

void
IdealRealGasMixtureFluidProperties::e_from_T_v(Real T,
                                               Real v,
                                               const std::vector<Real> & x,
                                               Real & e,
                                               Real & de_dT,
                                               Real & de_dv,
                                               std::vector<Real> & de_dx) const
{
  Real e_primary, de_dT_primary, de_dv_primary, de_dx_primary, dxi_dx_primary;
  Real de_dT_sec, dxj_dxi, dx_primary_dxi;
  std::vector<Real> e_sec, de_dv_sec;

  const Real x_primary = primaryMassFraction(x);
  de_dx.resize(_n_secondary_vapors);
  e_sec.resize(_n_secondary_vapors);
  de_dv_sec.resize(_n_secondary_vapors);

  _fp_primary->e_from_T_v(T, v / x_primary, e_primary, de_dT_primary, de_dv_primary);
  e = x_primary * e_primary;
  de_dT = x_primary * de_dT_primary;
  de_dv = de_dv_primary;
  de_dx_primary = e_primary - x_primary * de_dv_primary * v / (x_primary * x_primary);

  // get the partial pressures and their derivatives first
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->e_from_T_v(T, v / x[i], e_sec[i], de_dT_sec, de_dv_sec[i]);

    e += x[i] * e_sec[i];
    de_dT += x[i] * de_dT_sec;
    de_dv += de_dv_sec[i];
    dxi_dx_primary = -x[i] / (1. - x_primary);
    de_dx_primary +=
        dxi_dx_primary * e_sec[i] - x[i] * de_dv_sec[i] * v / (x[i] * x[i]) * dxi_dx_primary;
  }

  // get the composition dependent derivatives of the secondary vapors
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    de_dx[i] = e_sec[i] - x[i] * de_dv_sec[i] * v / (x[i] * x[i]);
    for (unsigned int j = 0; j < _n_secondary_vapors; j++)
    {
      if (j == i)
        continue;
      dxj_dxi = -x[j] / (1. - x[i]);
      de_dx[i] += dxj_dxi * e_sec[j] - x[j] * de_dv_sec[j] * v / (x[j] * x[j]) * dxj_dxi;
    }
    dx_primary_dxi = -x_primary / (1. - x[i]);
    de_dx[i] += dx_primary_dxi * e_primary -
                x_primary * de_dv_primary * v / (x_primary * x_primary) * dx_primary_dxi;
  }
}

void
IdealRealGasMixtureFluidProperties::s_from_T_v(
    Real T, Real v, const std::vector<Real> & x, Real & s, Real & ds_dT, Real & ds_dv) const
{
  Real s_primary, ds_dT_primary, ds_dv_primary;
  Real s_sec, ds_dT_sec, ds_dv_sec;

  const Real x_primary = primaryMassFraction(x);

  _fp_primary->s_from_T_v(T, v / x_primary, s_primary, ds_dT_primary, ds_dv_primary);
  s = x_primary * s_primary;
  ds_dT = x_primary * ds_dT_primary;
  ds_dv = ds_dv_primary;

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->s_from_T_v(T, v / x[i], s_sec, ds_dT_sec, ds_dv_sec);

    s += x[i] * s_sec;
    ds_dT += x[i] * ds_dT_sec;
    ds_dv += ds_dv_sec;
  }
}

Real
IdealRealGasMixtureFluidProperties::c_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  Real p, dp_dT, dp_dv;
  Real s, ds_dT, ds_dv;

  p_from_T_v(T, v, x, p, dp_dT, dp_dv);
  s_from_T_v(T, v, x, s, ds_dT, ds_dv);

  Real dp_dv_s = dp_dv - dp_dT * ds_dv / ds_dT;

  if (dp_dv_s >= 0)
    mooseWarning("c_from_T_v(), dp_dv_s = ", dp_dv_s, ". Should be negative.");
  return v * std::sqrt(-dp_dv_s);
}

void
IdealRealGasMixtureFluidProperties::c_from_T_v(Real T,
                                               Real v,
                                               const std::vector<Real> & x,
                                               Real & c,
                                               Real & dc_dT,
                                               Real & dc_dv,
                                               std::vector<Real> & dc_dx) const
{
  Real T_perturbed, v_perturbed, c_perturbed;

  c = c_from_T_v(T, v, x);
  // For derived properties, we would need higher order derivatives;
  // therefore, numerical derivatives are used here.
  Real dT = 1.e-6;
  T_perturbed = T + dT;
  c_perturbed = c_from_T_v(T_perturbed, v, x);
  dc_dT = (c_perturbed - c) / (dT);

  Real dv = v * 1.e-6;
  v_perturbed = v + dv;
  c_perturbed = c_from_T_v(T, v_perturbed, x);
  dc_dv = (c_perturbed - c) / (dv);

  dc_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real c_perturbed;
    std::vector<Real> x_perturbed(x);
    Real dx_i = 1e-6;
    for (unsigned int j = 0; j < _n_secondary_vapors; j++)
    {
      if (j != i)
        x_perturbed[j] =
            x[j] * (1.0 - (x[i] + dx_i)) / (1.0 - x[i]); // recalculate new mass fractions
    }
    x_perturbed[i] += dx_i;
    c_perturbed = c_from_T_v(T, v, x_perturbed);
    dc_dx[i] = ((c_perturbed - c) / dx_i);
  }
}

Real
IdealRealGasMixtureFluidProperties::cp_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  Real p, dp_dT, dp_dv;
  Real h, dh_dT, dh_dv;

  p_from_T_v(T, v, x, p, dp_dT, dp_dv);

  const Real x_primary = primaryMassFraction(x);

  _fp_primary->h_from_T_v(T, v / x_primary, h, dh_dT, dh_dv);
  Real cp = x_primary * (dh_dT - dh_dv * dp_dT / dp_dv);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->h_from_T_v(T, v / x[i], h, dh_dT, dh_dv);
    cp += x[i] * (dh_dT - dh_dv * dp_dT / dp_dv);
  }

  return cp;
}

Real
IdealRealGasMixtureFluidProperties::cv_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real cv = x_primary * _fp_primary->cv_from_T_v(T, v / x_primary);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    cv += x[i] * _fp_secondary[i]->cv_from_T_v(T, v / x[i]);

  return cv;
}

Real
IdealRealGasMixtureFluidProperties::mu_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = v / x_primary;
  Real ep = _fp_primary->e_from_T_v(T, vp);
  Real mu = x_primary * M_star / M_primary * _fp_primary->mu_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real vi = v / x[i];
    Real ei = _fp_secondary[i]->e_from_T_v(T, vp);
    Real Mi = _fp_secondary[i]->molarMass();
    mu += x[i] * M_star / Mi * _fp_secondary[i]->mu_from_v_e(vi, ei);
  }

  return mu;
}

Real
IdealRealGasMixtureFluidProperties::k_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = v / x_primary;
  Real ep = _fp_primary->e_from_T_v(T, vp);
  Real k = x_primary * M_star / M_primary * _fp_primary->k_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    Real vi = v / x[i];
    Real ei = _fp_secondary[i]->e_from_T_v(T, vp);
    Real Mi = _fp_secondary[i]->molarMass();
    k += x[i] * M_star / Mi * _fp_secondary[i]->k_from_v_e(vi, ei);
  }

  return k;
}

Real
IdealRealGasMixtureFluidProperties::xs_prim_from_p_T(Real p,
                                                     Real T,
                                                     const std::vector<Real> & x) const
{
  Real T_c = _fp_primary->criticalTemperature();
  Real xs;
  if (T > T_c)
    // return 1. to indicate that no water would condense for
    // given (p,T)
    xs = 1.;
  else
  {
    Real pp_sat = _fp_primary->pp_sat_from_p_T(p, T);
    if (pp_sat < 0.)
    {
      // return 1. to indicate that no water would condense for
      // given (p,T)
      xs = 1.;
      return xs;
    }
    Real v_primary = _fp_primary->v_from_p_T(pp_sat, T);
    Real pp_sat_secondary = p - pp_sat;

    Real v_secondary;
    if (_n_secondary_vapors == 1)
      v_secondary = _fp_secondary[0]->v_from_p_T(pp_sat_secondary, T);
    else
    {
      std::vector<Real> x_sec(_n_secondary_vapors);
      const Real x_primary = primaryMassFraction(x);
      Real sum = 0.;
      for (unsigned int i = 0; i < _n_secondary_vapors; i++)
      {
        x_sec[i] = x[i] / (1. - x_primary);
        sum += x_sec[i] / _fp_secondary[i]->molarMass();
      }
      Real M_star = 1. / sum;
      v_secondary = R_molar * T / (M_star * pp_sat_secondary);
      int it = 0;
      double f = 1., df_dvs, pp_sec, p_sec, dp_dT_sec, dp_dv_sec, dp_dT, dp_dv;
      double tol_p = 1.e-8;
      while (std::fabs(f / pp_sat_secondary) > tol_p)
      {
        pp_sec = 0.;
        dp_dT = 0.;
        dp_dv = 0.;
        for (unsigned int i = 0; i < _n_secondary_vapors; i++)
        {
          _fp_secondary[i]->p_from_T_v(T, v_secondary / x_sec[i], p_sec, dp_dT_sec, dp_dv_sec);
          pp_sec += p_sec;
          dp_dT += dp_dT_sec;
          dp_dv += dp_dv_sec / x_sec[i];
        }
        f = pp_sec - pp_sat_secondary;
        df_dvs = dp_dv;
        v_secondary -= f / df_dvs;
        if (it++ > 15)
          return getNaN();
      }
    }

    xs = v_secondary / (v_primary + v_secondary);
  }

  return xs;
}
