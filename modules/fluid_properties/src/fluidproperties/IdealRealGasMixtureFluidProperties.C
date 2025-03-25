//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

/**
 * Defines the value and derivative methods from (v,e) for a property y
 * using T(v,e) and y(T,v).
 */
#define define_from_v_e_using_T_v(prop)                                                            \
  Real IdealRealGasMixtureFluidProperties::prop##_from_v_e(                                        \
      Real v, Real e, const std::vector<Real> & x) const                                           \
  {                                                                                                \
    const Real T = T_from_v_e(v, e, x);                                                            \
    return prop##_from_T_v(T, v, x);                                                               \
  }                                                                                                \
                                                                                                   \
  void IdealRealGasMixtureFluidProperties::prop##_from_v_e(Real v,                                 \
                                                           Real e,                                 \
                                                           const std::vector<Real> & x,            \
                                                           Real & y,                               \
                                                           Real & dy_dv,                           \
                                                           Real & dy_de,                           \
                                                           std::vector<Real> & dy_dx) const        \
  {                                                                                                \
    Real T, dT_dv_e, dT_de_v;                                                                      \
    std::vector<Real> dT_dx_ve(_n_secondary_vapors);                                               \
    T_from_v_e(v, e, x, T, dT_dv_e, dT_de_v, dT_dx_ve);                                            \
                                                                                                   \
    Real dy_dT_v, dy_dv_T;                                                                         \
    std::vector<Real> dy_dx_Tv;                                                                    \
    prop##_from_T_v(T, v, x, y, dy_dT_v, dy_dv_T, dy_dx_Tv);                                       \
                                                                                                   \
    dy_dv = dy_dv_T + dy_dT_v * dT_dv_e;                                                           \
    dy_de = dy_dT_v * dT_de_v;                                                                     \
    dy_dx.resize(_n_secondary_vapors);                                                             \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
      dy_dx[i] = dy_dx_Tv[i] + dy_dT_v * dT_dx_ve[i];                                              \
  }

/**
 * Defines the value and derivative methods from (p,T) for a property y
 * using v(p,T) and y(T,v).
 */
#define define_from_p_T_using_T_v(prop)                                                            \
  Real IdealRealGasMixtureFluidProperties::prop##_from_p_T(                                        \
      Real p, Real T, const std::vector<Real> & x) const                                           \
  {                                                                                                \
    const Real v = v_from_p_T(p, T, x);                                                            \
    return prop##_from_T_v(T, v, x);                                                               \
  }                                                                                                \
                                                                                                   \
  void IdealRealGasMixtureFluidProperties::prop##_from_p_T(Real p,                                 \
                                                           Real T,                                 \
                                                           const std::vector<Real> & x,            \
                                                           Real & y,                               \
                                                           Real & dy_dp,                           \
                                                           Real & dy_dT,                           \
                                                           std::vector<Real> & dy_dx) const        \
  {                                                                                                \
    Real v, dv_dp_T, dv_dT_p;                                                                      \
    std::vector<Real> dv_dx_pT;                                                                    \
    v_from_p_T(p, T, x, v, dv_dp_T, dv_dT_p, dv_dx_pT);                                            \
                                                                                                   \
    Real dy_dT_v, dy_dv_T;                                                                         \
    std::vector<Real> dy_dx_Tv;                                                                    \
    prop##_from_T_v(T, v, x, y, dy_dT_v, dy_dv_T, dy_dx_Tv);                                       \
                                                                                                   \
    dy_dp = dy_dv_T * dv_dp_T;                                                                     \
    dy_dT = dy_dT_v + dy_dv_T * dv_dT_p;                                                           \
    dy_dx.resize(_n_secondary_vapors);                                                             \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
      dy_dx[i] = dy_dx_Tv[i] + dy_dv_T * dv_dx_pT[i];                                              \
  }

/**
 * Defines the value and derivative methods from (T,v) for a mass-specific property y
 */
#define define_mass_specific_prop_from_T_v(prop)                                                   \
  Real IdealRealGasMixtureFluidProperties::prop##_from_T_v(                                        \
      Real T, Real v, const std::vector<Real> & x) const                                           \
  {                                                                                                \
    const Real x_primary = primaryMassFraction(x);                                                 \
    Real y = x_primary * _fp_primary->prop##_from_T_v(T, v / x_primary);                           \
                                                                                                   \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
      y += x[i] * _fp_secondary[i]->prop##_from_T_v(T, v / x[i]);                                  \
                                                                                                   \
    return y;                                                                                      \
  }                                                                                                \
                                                                                                   \
  void IdealRealGasMixtureFluidProperties::prop##_from_T_v(Real T,                                 \
                                                           Real v,                                 \
                                                           const std::vector<Real> & x,            \
                                                           Real & y,                               \
                                                           Real & dy_dT,                           \
                                                           Real & dy_dv,                           \
                                                           std::vector<Real> & dy_dx) const        \
  {                                                                                                \
    const Real x_primary = primaryMassFraction(x);                                                 \
    mooseAssert(!MooseUtils::absoluteFuzzyEqual(x_primary, 0.0), "Mass fraction may not be zero"); \
                                                                                                   \
    Real y_primary, dy_dT_primary, dy_dv_primary;                                                  \
    _fp_primary->prop##_from_T_v(T, v / x_primary, y_primary, dy_dT_primary, dy_dv_primary);       \
    y = x_primary * y_primary;                                                                     \
    dy_dT = x_primary * dy_dT_primary;                                                             \
    dy_dv = dy_dv_primary;                                                                         \
                                                                                                   \
    Real dy_dT_sec;                                                                                \
    std::vector<Real> y_sec(_n_secondary_vapors), dy_dv_sec(_n_secondary_vapors);                  \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
    {                                                                                              \
      mooseAssert(!MooseUtils::absoluteFuzzyEqual(x[i], 0.0), "Mass fraction may not be zero");    \
      _fp_secondary[i]->prop##_from_T_v(T, v / x[i], y_sec[i], dy_dT_sec, dy_dv_sec[i]);           \
      y += x[i] * y_sec[i];                                                                        \
      dy_dT += x[i] * dy_dT_sec;                                                                   \
      dy_dv += dy_dv_sec[i];                                                                       \
    }                                                                                              \
                                                                                                   \
    dy_dx.resize(_n_secondary_vapors);                                                             \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
    {                                                                                              \
      dy_dx[i] = y_sec[i] - x[i] * dy_dv_sec[i] * v / (x[i] * x[i]);                               \
      for (unsigned int j = 0; j < _n_secondary_vapors; j++)                                       \
      {                                                                                            \
        if (j == i)                                                                                \
          continue;                                                                                \
        if (_n_secondary_vapors > 1)                                                               \
          imperfectJacobianMessage(                                                                \
              "The mass fraction derivatives in the following "                                    \
              "function have not been tested for mixtures of 3 or more components:\n\n",           \
              __PRETTY_FUNCTION__);                                                                \
        const Real dxj_dxi = 0;                                                                    \
        dy_dx[i] += dxj_dxi * y_sec[j] - x[j] * dy_dv_sec[j] * v / (x[j] * x[j]) * dxj_dxi;        \
      }                                                                                            \
      const Real dx_primary_dxi = -x_primary / (1. - x[i]);                                        \
      dy_dx[i] += dx_primary_dxi * y_primary -                                                     \
                  x_primary * dy_dv_primary * v / (x_primary * x_primary) * dx_primary_dxi;        \
    }                                                                                              \
  }

/**
 * Defines the value and derivative methods from (T,v) for a transport property y
 */
#define define_transport_prop_from_T_v(prop)                                                       \
  Real IdealRealGasMixtureFluidProperties::prop##_from_T_v(                                        \
      Real T, Real v, const std::vector<Real> & x) const                                           \
  {                                                                                                \
    const Real x_primary = primaryMassFraction(x);                                                 \
    Real M_primary = _fp_primary->molarMass();                                                     \
                                                                                                   \
    Real sum = x_primary / M_primary;                                                              \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
      sum += x[i] / _fp_secondary[i]->molarMass();                                                 \
    const Real M_star = 1. / sum;                                                                  \
                                                                                                   \
    const Real vp = v / x_primary;                                                                 \
    const Real ep = _fp_primary->e_from_T_v(T, vp);                                                \
    const Real yp = _fp_primary->prop##_from_v_e(vp, ep);                                          \
    Real y = x_primary * M_star / M_primary * yp;                                                  \
                                                                                                   \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
    {                                                                                              \
      const Real vi = v / x[i];                                                                    \
      const Real ei = _fp_secondary[i]->e_from_T_v(T, vp);                                         \
      const Real Mi = _fp_secondary[i]->molarMass();                                               \
      const Real yi = _fp_secondary[i]->prop##_from_v_e(vi, ei);                                   \
      y += x[i] * M_star / Mi * yi;                                                                \
    }                                                                                              \
                                                                                                   \
    return y;                                                                                      \
  }                                                                                                \
                                                                                                   \
  void IdealRealGasMixtureFluidProperties::prop##_from_T_v(Real T,                                 \
                                                           Real v,                                 \
                                                           const std::vector<Real> & x,            \
                                                           Real & y,                               \
                                                           Real & dy_dT,                           \
                                                           Real & dy_dv,                           \
                                                           std::vector<Real> & dy_dx) const        \
  {                                                                                                \
    const Real x_primary = primaryMassFraction(x);                                                 \
    Real M_primary = _fp_primary->molarMass();                                                     \
                                                                                                   \
    Real sum = x_primary / M_primary;                                                              \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
      sum += x[i] / _fp_secondary[i]->molarMass();                                                 \
    const Real M_star = 1. / sum;                                                                  \
                                                                                                   \
    const Real vp = v / x_primary;                                                                 \
    const Real ep = _fp_primary->e_from_T_v(T, vp);                                                \
    const Real yp = _fp_primary->prop##_from_v_e(vp, ep);                                          \
    y = x_primary * M_star / M_primary * yp;                                                       \
                                                                                                   \
    imperfectJacobianMessage("The temperature and specific volume derivatives in the following "   \
                             "function are currently neglected:\n\n",                              \
                             __PRETTY_FUNCTION__);                                                 \
    dy_dT = 0;                                                                                     \
    dy_dv = 0;                                                                                     \
                                                                                                   \
    Real sum_yj = 0;                                                                               \
    dy_dx.resize(_n_secondary_vapors);                                                             \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
    {                                                                                              \
      const Real vi = v / x[i];                                                                    \
      const Real ei = _fp_secondary[i]->e_from_T_v(T, vi);                                         \
      const Real Mi = _fp_secondary[i]->molarMass();                                               \
      const Real yi = _fp_secondary[i]->prop##_from_v_e(vi, ei);                                   \
      y += x[i] * M_star / Mi * yi;                                                                \
      dy_dx[i] = -M_star / M_primary * yp -                                                        \
                 x_primary * (1 / Mi - 1 / M_primary) * M_star * M_star / M_primary * yp +         \
                 M_star / Mi * yi;                                                                 \
      sum_yj += x[i] * yi / Mi;                                                                    \
    }                                                                                              \
                                                                                                   \
    for (unsigned int i = 0; i < _n_secondary_vapors; i++)                                         \
    {                                                                                              \
      const Real Mi = _fp_secondary[i]->molarMass();                                               \
      dy_dx[i] += -M_star * M_star * (1 / Mi - 1 / M_primary) * sum_yj;                            \
    }                                                                                              \
  }

// clang-format off
define_mass_specific_prop_from_T_v(e)
define_mass_specific_prop_from_T_v(s)

define_transport_prop_from_T_v(mu)
define_transport_prop_from_T_v(k)

define_from_p_T_using_T_v(e)
define_from_p_T_using_T_v(s)
define_from_p_T_using_T_v(c)
define_from_p_T_using_T_v(cp)
define_from_p_T_using_T_v(cv)
define_from_p_T_using_T_v(mu)
define_from_p_T_using_T_v(k)

define_from_v_e_using_T_v(p)
define_from_v_e_using_T_v(c)

InputParameters IdealRealGasMixtureFluidProperties::validParams()
// clang-format on
{
  InputParameters params = VaporMixtureFluidProperties::validParams();
  params += NaNInterface::validParams();

  params.addClassDescription("Class for fluid properties of an arbitrary vapor mixture");

  params.addRequiredParam<UserObjectName>(
      "fp_primary", "Name of fluid properties user object for primary vapor component");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp_secondary", "Name of fluid properties user object(s) for secondary vapor component(s)");
  params.addParam<Real>("_T_mix_max", 1300., "Maximum temperature of the mixture");

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

const SinglePhaseFluidProperties &
IdealRealGasMixtureFluidProperties::getPrimaryFluidProperties() const
{
  return *_fp_primary;
}

const SinglePhaseFluidProperties &
IdealRealGasMixtureFluidProperties::getSecondaryFluidProperties(unsigned int i) const
{
  mooseAssert(i < getNumberOfSecondaryVapors(), "Requested secondary index too high.");
  return *_fp_secondary[i];
}

Real
IdealRealGasMixtureFluidProperties::T_from_v_e(Real v, Real e, const std::vector<Real> & x) const
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
  return BrentsMethod::root(energy_diff, lower_temperature, upper_temperature);
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
  T = T_from_v_e(v, e, x);

  Real e_unused, de_dT_v, de_dv_T;
  std::vector<Real> de_dx_Tv;
  e_from_T_v(T, v, x, e_unused, de_dT_v, de_dv_T, de_dx_Tv);

  dT_dv = -de_dv_T / de_dT_v;
  dT_de = 1.0 / de_dT_v;
  dT_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    dT_dx[i] = -de_dx_Tv[i] / de_dT_v;
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
  return BrentsMethod::root(pressure_diff, lower_spec_volume, upper_spec_volume);
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
  v = v_from_p_T(p, T, x);

  Real p_unused, dp_dT, dp_dv;
  std::vector<Real> dp_dx;
  p_from_T_v(T, v, x, p_unused, dp_dT, dp_dv, dp_dx);

  dv_dp = 1. / dp_dv;
  dv_dT = -dp_dT / dp_dv;
  dv_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
    dv_dx[i] = -dp_dx[i] / dp_dv;
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
  return BrentsMethod::root(pressure_diff, lower_temperature, upper_temperature);
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
    dT_dx[i] = -dp_dx_Tv[i] / dp_dT_v;
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
IdealRealGasMixtureFluidProperties::p_from_T_v(Real T,
                                               Real v,
                                               const std::vector<Real> & x,
                                               Real & p,
                                               Real & dp_dT,
                                               Real & dp_dv,
                                               std::vector<Real> & dp_dx) const
{
  const Real x_primary = primaryMassFraction(x);

  Real p_primary, dp_dT_primary, dp_dv_primary;
  _fp_primary->p_from_T_v(T, v / x_primary, p_primary, dp_dT_primary, dp_dv_primary);
  p = p_primary;
  dp_dT = dp_dT_primary;
  dp_dv = dp_dv_primary / x_primary;

  // get the partial pressures and their derivatives first
  Real p_sec, dp_dT_sec, dxj_dxi;
  std::vector<Real> dp_dv_sec(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->p_from_T_v(T, v / x[i], p_sec, dp_dT_sec, dp_dv_sec[i]);
    p += p_sec;
    dp_dT += dp_dT_sec;
    dp_dv += dp_dv_sec[i] / x[i];
  }

  // get the composition dependent derivatives of the secondary vapors
  dp_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    dp_dx[i] = -dp_dv_sec[i] * v / (x[i] * x[i]);
    for (unsigned int j = 0; j < _n_secondary_vapors; j++)
    {
      if (j == i)
        continue;
      // Note: this was previously as follows, but we were unable to understand
      // why and this is not currently tested (requires 3 or more components in
      // the mixture):
      // dxj_dxi = -x[j] / (1. - x[i]);
      if (_n_secondary_vapors > 1)
        imperfectJacobianMessage(
            "The mass fraction derivatives in the following "
            "function have not been tested for mixtures of 3 or more components:\n\n",
            __PRETTY_FUNCTION__);
      dxj_dxi = 0;
      dp_dx[i] += -dp_dv_sec[j] * v / (x[j] * x[j]) * dxj_dxi;
    }
    dp_dx[i] += -dp_dv_primary * v / (x_primary * x_primary) * (-x_primary / (1. - x[i]));
  }
}

Real
IdealRealGasMixtureFluidProperties::c_from_T_v(Real T, Real v, const std::vector<Real> & x) const
{
  Real p, dp_dT, dp_dv;
  std::vector<Real> dp_dx;
  p_from_T_v(T, v, x, p, dp_dT, dp_dv, dp_dx);

  Real s, ds_dT, ds_dv;
  std::vector<Real> ds_dx;
  s_from_T_v(T, v, x, s, ds_dT, ds_dv, ds_dx);

  const Real dp_dv_s = dp_dv - dp_dT * ds_dv / ds_dT;

  if (dp_dv_s >= 0)
    mooseWarning("c_from_T_v(): dp_dv_s = ", dp_dv_s, ". Should be negative.");
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
  c = c_from_T_v(T, v, x);

  // For derived properties, we would need higher order derivatives;
  // therefore, numerical derivatives are used here.
  const Real dT = 1.e-6;
  const Real T_perturbed = T + dT;
  Real c_perturbed = c_from_T_v(T_perturbed, v, x);
  dc_dT = (c_perturbed - c) / dT;

  const Real dv = v * 1.e-6;
  const Real v_perturbed = v + dv;
  c_perturbed = c_from_T_v(T, v_perturbed, x);
  dc_dv = (c_perturbed - c) / dv;

  dc_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    std::vector<Real> x_perturbed(x);
    const Real dx_i = 1e-6;
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
  const Real x_primary = primaryMassFraction(x);

  Real p, dp_dT, dp_dv;
  std::vector<Real> dp_dx;
  p_from_T_v(T, v, x, p, dp_dT, dp_dv, dp_dx);

  Real h, dh_dT, dh_dv;
  _fp_primary->h_from_T_v(T, v / x_primary, h, dh_dT, dh_dv);
  const Real cp_primary = dh_dT - dh_dv * dp_dT / dp_dv;
  Real cp = x_primary * cp_primary;

  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->h_from_T_v(T, v / x[i], h, dh_dT, dh_dv);
    const Real cp_sec = dh_dT - dh_dv * dp_dT / dp_dv;
    cp += x[i] * cp_sec;
  }

  return cp;
}

void
IdealRealGasMixtureFluidProperties::cp_from_T_v(Real T,
                                                Real v,
                                                const std::vector<Real> & x,
                                                Real & cp,
                                                Real & dcp_dT,
                                                Real & dcp_dv,
                                                std::vector<Real> & dcp_dx) const
{
  const Real x_primary = primaryMassFraction(x);

  Real p, dp_dT, dp_dv;
  std::vector<Real> dp_dx;
  p_from_T_v(T, v, x, p, dp_dT, dp_dv, dp_dx);

  Real h, dh_dT, dh_dv;
  _fp_primary->h_from_T_v(T, v / x_primary, h, dh_dT, dh_dv);
  const Real cp_primary = dh_dT - dh_dv * dp_dT / dp_dv;
  cp = x_primary * cp_primary;

  // Neglect these for now. These require higher-order derivatives, so a finite
  // difference should probably be used, as for sound speed.
  imperfectJacobianMessage("The temperature and specific volume derivatives in the following "
                           "function are currently neglected:\n\n",
                           __PRETTY_FUNCTION__);
  dcp_dT = 0;
  dcp_dv = 0;

  dcp_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    _fp_secondary[i]->h_from_T_v(T, v / x[i], h, dh_dT, dh_dv);
    const Real cp_sec = dh_dT - dh_dv * dp_dT / dp_dv;
    cp += x[i] * cp_sec;
    dcp_dx[i] = cp_sec - cp_primary;
  }
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

void
IdealRealGasMixtureFluidProperties::cv_from_T_v(Real T,
                                                Real v,
                                                const std::vector<Real> & x,
                                                Real & cv,
                                                Real & dcv_dT,
                                                Real & dcv_dv,
                                                std::vector<Real> & dcv_dx) const
{
  const Real x_primary = primaryMassFraction(x);
  const Real cv_primary = _fp_primary->cv_from_T_v(T, v / x_primary);
  cv = x_primary * cv_primary;

  // Neglect these for now. These require higher-order derivatives, so a finite
  // difference should probably be used, as for sound speed.
  imperfectJacobianMessage("The temperature and specific volume derivatives in the following "
                           "function are currently neglected:\n\n",
                           __PRETTY_FUNCTION__);
  dcv_dT = 0;
  dcv_dv = 0;

  dcv_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; i++)
  {
    const Real cv_sec = _fp_secondary[i]->cv_from_T_v(T, v / x[i]);
    cv += x[i] * cv_sec;
    dcv_dx[i] = cv_sec - cv_primary;
  }
}

Real
IdealRealGasMixtureFluidProperties::xs_prim_from_p_T(Real p,
                                                     Real T,
                                                     const std::vector<Real> & x) const
{
  Real T_c = _fp_primary->criticalTemperature();
  Real xs;
  if (T > T_c)
    // return 1. to indicate that the primary fluid will not condense for
    // given (p,T)
    xs = 1.;
  else
  {
    Real pp_sat = _fp_primary->pp_sat_from_p_T(p, T);
    if (pp_sat < 0.)
    {
      // return 1. to indicate that the primary fluid will not condense for
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
      double f = 1., df_dvs, pp_sec, p_sec, dp_dT_sec, dp_dv_sec, dp_dv;
      double tol_p = 1.e-8;
      while (std::fabs(f / pp_sat_secondary) > tol_p)
      {
        pp_sec = 0.;
        dp_dv = 0.;
        for (unsigned int i = 0; i < _n_secondary_vapors; i++)
        {
          _fp_secondary[i]->p_from_T_v(T, v_secondary / x_sec[i], p_sec, dp_dT_sec, dp_dv_sec);
          pp_sec += p_sec;
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
