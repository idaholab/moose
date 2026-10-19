//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedPVTFluidProperties.h"

#include <algorithm>

registerMooseObject("PorousFlowApp", TabulatedPVTFluidProperties);

InputParameters
TabulatedPVTFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "density_sc",
      "density_sc>0",
      "Density of the fluid at standard (surface) conditions (kg/m^3)");
  params.addRequiredParam<std::vector<Real>>(
      "pressure", "Tabulated pressures (Pa). Must be strictly increasing");
  params.addRequiredParam<std::vector<Real>>(
      "formation_volume_factor",
      "Formation volume factor B (reservoir volume / standard volume) at each tabulated pressure");
  params.addRequiredParam<std::vector<Real>>("viscosity",
                                             "Dynamic viscosity (Pa.s) at each tabulated pressure");
  params.addRangeCheckedParam<Real>(
      "cv", 2000.0, "cv>0", "Constant specific heat capacity at constant volume (J/kg/K)");
  params.addRangeCheckedParam<Real>(
      "molar_mass", "molar_mass>0", "Molar mass of the fluid (kg/mol). Only needed if requested");
  MooseEnum out_of_bounds("extrapolate clamp", "extrapolate");
  params.addParam<MooseEnum>(
      "out_of_bounds",
      out_of_bounds,
      "Behaviour for pressures outside the table: 'extrapolate' linearly from the end intervals, "
      "or 'clamp' to the end values (which gives zero pressure derivatives)");
  params.addClassDescription(
      "Isothermal fluid properties from a table of pressure, formation volume factor and "
      "viscosity (PVDO/PVDG style). 1/B and 1/(B mu) are interpolated linearly in pressure.");
  return params;
}

TabulatedPVTFluidProperties::TabulatedPVTFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _density_sc(getParam<Real>("density_sc")),
    _p(getParam<std::vector<Real>>("pressure")),
    _cv(getParam<Real>("cv")),
    _extrapolate(getParam<MooseEnum>("out_of_bounds") == "extrapolate")
{
  const auto & B = getParam<std::vector<Real>>("formation_volume_factor");
  const auto & mu = getParam<std::vector<Real>>("viscosity");

  if (_p.size() < 2)
    paramError("pressure", "At least two tabulated pressures are required");
  if (B.size() != _p.size())
    paramError("formation_volume_factor", "Must have the same number of entries as 'pressure'");
  if (mu.size() != _p.size())
    paramError("viscosity", "Must have the same number of entries as 'pressure'");

  for (std::size_t i = 1; i < _p.size(); ++i)
    if (_p[i] <= _p[i - 1])
      paramError("pressure", "Tabulated pressures must be strictly increasing");

  _inv_B.resize(_p.size());
  _inv_B_mu.resize(_p.size());
  for (std::size_t i = 0; i < _p.size(); ++i)
  {
    if (B[i] <= 0.0)
      paramError("formation_volume_factor", "All entries must be positive");
    if (mu[i] <= 0.0)
      paramError("viscosity", "All entries must be positive");
    _inv_B[i] = 1.0 / B[i];
    _inv_B_mu[i] = 1.0 / (B[i] * mu[i]);
  }
}

std::string
TabulatedPVTFluidProperties::fluidName() const
{
  return "tabulated_pvt";
}

Real
TabulatedPVTFluidProperties::molarMass() const
{
  if (!isParamValid("molar_mass"))
    paramError("molar_mass",
               "molar_mass must be provided because it is required by another object");
  return getParam<Real>("molar_mass");
}

void
TabulatedPVTFluidProperties::interpolate(Real p,
                                         const std::vector<Real> & y,
                                         Real & value,
                                         Real & dvalue_dp) const
{
  const std::size_t n = _p.size();

  if (!_extrapolate && p <= _p.front())
  {
    value = y.front();
    dvalue_dp = 0.0;
    return;
  }
  if (!_extrapolate && p >= _p.back())
  {
    value = y.back();
    dvalue_dp = 0.0;
    return;
  }

  // Index i of the interval [_p[i], _p[i+1]] used for interpolation (or extrapolation)
  const auto it = std::upper_bound(_p.begin(), _p.end(), p);
  std::size_t i = (it == _p.begin()) ? 0 : static_cast<std::size_t>(it - _p.begin()) - 1;
  i = std::min(i, n - 2);

  dvalue_dp = (y[i + 1] - y[i]) / (_p[i + 1] - _p[i]);
  value = y[i] + dvalue_dp * (p - _p[i]);
}

Real
TabulatedPVTFluidProperties::rho_from_p_T(Real p, Real T) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  return rho;
}

void
TabulatedPVTFluidProperties::rho_from_p_T(
    Real p, Real /*T*/, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  Real inv_B, dinv_B_dp;
  interpolate(p, _inv_B, inv_B, dinv_B_dp);

  if (inv_B <= 0.0)
    throw MooseException(name(),
                         ": the formation volume factor is non-positive at pressure ",
                         p,
                         ", which is outside the tabulated range. Extend the table.");

  rho = _density_sc * inv_B;
  drho_dp = _density_sc * dinv_B_dp;
  drho_dT = 0.0;
}

Real
TabulatedPVTFluidProperties::mu_from_p_T(Real p, Real T) const
{
  Real mu, dmu_dp, dmu_dT;
  mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  return mu;
}

void
TabulatedPVTFluidProperties::mu_from_p_T(
    Real p, Real /*T*/, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real inv_B, dinv_B_dp, inv_B_mu, dinv_B_mu_dp;
  interpolate(p, _inv_B, inv_B, dinv_B_dp);
  interpolate(p, _inv_B_mu, inv_B_mu, dinv_B_mu_dp);

  if (inv_B <= 0.0 || inv_B_mu <= 0.0)
    throw MooseException(name(),
                         ": the viscosity is non-positive at pressure ",
                         p,
                         ", which is outside the tabulated range. Extend the table.");

  // mu = (1/B) / (1/(B mu))
  mu = inv_B / inv_B_mu;
  dmu_dp = (dinv_B_dp * inv_B_mu - inv_B * dinv_B_mu_dp) / (inv_B_mu * inv_B_mu);
  dmu_dT = 0.0;
}

Real
TabulatedPVTFluidProperties::e_from_p_T(Real /*p*/, Real T) const
{
  return _cv * T;
}

void
TabulatedPVTFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  e = e_from_p_T(p, T);
  de_dp = 0.0;
  de_dT = _cv;
}

Real
TabulatedPVTFluidProperties::h_from_p_T(Real p, Real T) const
{
  Real h, dh_dp, dh_dT;
  h_from_p_T(p, T, h, dh_dp, dh_dT);
  return h;
}

void
TabulatedPVTFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  // h = e + p / rho
  h = e_from_p_T(p, T) + p / rho;
  dh_dp = 1.0 / rho - p * drho_dp / (rho * rho);
  dh_dT = _cv;
}
