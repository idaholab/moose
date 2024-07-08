//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", LinearFluidProperties);

InputParameters
LinearFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addRequiredParam<Real>("p_0", "Reference pressure");
  params.addRequiredParam<Real>("rho_0", "Reference density");
  params.addRequiredParam<Real>("a2", "dp/d(rho)");
  params.addRequiredParam<Real>("beta", "Coefficient of thermal expansion");
  params.addRequiredParam<Real>("cv", "Specific heat");
  params.addRequiredParam<Real>("e_0", "Reference internal energy");
  params.addRequiredParam<Real>("T_0", "Reference internal energy");
  params.addRequiredParam<Real>("mu", "Dynamic viscosity, Pa.s");
  params.addRequiredParam<Real>("k", "Thermal conductivity, W/(m-K)");
  params.addDeprecatedParam<Real>(
      "Pr",
      "Prandtl Number, [-]",
      "This parameter is no longer required. It is computed from the other parameters.");
  params.addClassDescription(
      "Fluid properties for a fluid with density linearly dependent on temperature and pressure");
  return params;
}

LinearFluidProperties::LinearFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _rho_0(getParam<Real>("rho_0")),
    _p_0(getParam<Real>("p_0")),
    _a2(getParam<Real>("a2")),
    _beta(getParam<Real>("beta")),
    _cv(getParam<Real>("cv")),
    _e_0(getParam<Real>("e_0")),
    _T_0(getParam<Real>("T_0")),
    _mu(getParam<Real>("mu")),
    _k(getParam<Real>("k"))
{
  if (isParamValid("Pr"))
    _Pr = getParam<Real>("Pr");
  else
    _Pr = _cv / _k * _mu;

  // Sanity checks
  if (!MooseUtils::absoluteFuzzyEqual(_Pr, _cv / _k * _mu))
    paramError("Pr", "Prandtl number should be equal to cv * mu / k");
}

Real
LinearFluidProperties::p_from_v_e(Real v, Real e) const
{
  return _p_0 + _rho_0 * _a2 * ((1 / v / _rho_0 - 1.) + (_beta / _cv) * (e - _e_0));
}

void
LinearFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  dp_dv = -_a2 / v / v;
  dp_de = _rho_0 * _a2 * _beta / _cv;
}

Real
LinearFluidProperties::T_from_v_e(Real /*v*/, Real e) const
{
  // e - e0 = cv * (T - T0)
  return _T_0 + (1. / _cv) * (e - _e_0);
}

void
LinearFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_dv = 0;
  dT_de = 1 / _cv;
}

Real
LinearFluidProperties::c_from_v_e(Real, Real) const
{
  return std::sqrt(_a2);
}

void
LinearFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  c = c_from_v_e(v, e);
  dc_dv = 0;
  dc_de = 0;
}

Real
LinearFluidProperties::cp_from_v_e(Real, Real) const
{
  return _cv;
}

void
LinearFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  cp = cp_from_v_e(v, e);
  dcp_de = 0;
  dcp_dv = 0;
}

Real
LinearFluidProperties::cv_from_v_e(Real, Real) const
{
  return _cv;
}

void
LinearFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  cv = cv_from_v_e(v, e);
  dcv_de = 0;
  dcv_dv = 0;
}

Real
LinearFluidProperties::mu_from_v_e(Real, Real) const
{
  return _mu;
}

Real
LinearFluidProperties::k_from_v_e(Real, Real) const
{
  return _k;
}

Real
LinearFluidProperties::s_from_v_e(Real, Real) const
{
  mooseError(name(), ": s_from_v_e() not implemented.");
}

void
LinearFluidProperties::s_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": s_from_v_e() not implemented.");
}

Real
LinearFluidProperties::s_from_p_T(Real, Real) const
{
  mooseError(name(), ": s_from_p_T() not implemented.");
}

void
LinearFluidProperties::s_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": s_from_p_T() not implemented.");
}

Real
LinearFluidProperties::s_from_h_p(Real, Real) const
{
  mooseError(name(), ": s(h,p) is not implemented");
}

void
LinearFluidProperties::s_from_h_p(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": s(h,p) is not implemented");
}

Real
LinearFluidProperties::rho_from_p_s(Real, Real) const
{
  mooseError(name(), ": rho_from_p_s() not implemented.");
}

void
LinearFluidProperties::rho_from_p_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": rho_from_p_s() not implemented.");
}

Real
LinearFluidProperties::e_from_v_h(Real v, Real h) const
{
  return (h - v * p_from_v_e(v, 0)) / (1 + v * _beta / _cv * _rho_0 * _a2);
}

void
LinearFluidProperties::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  const auto num = (h - v * (_p_0 + _a2 * ((1 / v - _rho_0) - _rho_0 * _beta / _cv * _e_0)));
  const auto denum = (1 + v * _beta / _cv * _rho_0 * _a2);
  e = num / denum;
  de_dh = 1 / denum;
  de_dv = ((-_p_0 - _a2 * _rho_0 * (-1 - _beta / _cv * _e_0)) * denum -
           num * _beta / _cv * _rho_0 * _a2) /
          denum / denum;
}

Real
LinearFluidProperties::beta_from_p_T(Real, Real) const
{
  return _beta;
}

void
LinearFluidProperties::beta_from_p_T(
    Real p, Real T, Real & beta, Real & dbeta_dp, Real & dbeta_dT) const
{
  beta = beta_from_p_T(p, T);
  dbeta_dp = 0;
  dbeta_dT = 0;
}

Real
LinearFluidProperties::rho_from_p_T(Real p, Real T) const
{
  Real e = _e_0 + _cv * (T - _T_0);
  return (p - _p_0) / _a2 - _rho_0 * (_beta / _cv) * (e - _e_0) + _rho_0;
}

void
LinearFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  Real e = _e_0 + _cv * (T - _T_0);
  rho = (p - _p_0) / _a2 - _rho_0 * (_beta / _cv) * (e - _e_0) + _rho_0;
  drho_dp = 1 / _a2;
  drho_dT = -_rho_0 * _beta;
}

Real
LinearFluidProperties::e_from_p_T(Real p, Real T) const
{
  const auto rho = rho_from_p_T(p, T);
  return e_from_p_rho(p, rho);
}

void
LinearFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real de_drho, de_dp_rho;
  e_from_p_rho(p, rho, e, de_dp_rho, de_drho);
  de_dp = de_drho * drho_dp + de_dp_rho;
  de_dT = de_drho * drho_dT;
}

Real
LinearFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return (_cv / _beta) * (((p - _p_0) / (_rho_0 * _a2)) - (rho / _rho_0) + 1) + _e_0;
}

void
LinearFluidProperties::e_from_p_rho(Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  e = e_from_p_rho(p, rho);
  de_dp = _cv / _beta / _rho_0 / _a2;
  de_drho = -_cv / _beta / _rho_0;
}

Real
LinearFluidProperties::h_from_p_T(Real p, Real T) const
{
  Real rho = rho_from_p_T(p, T);
  Real e = e_from_p_rho(p, rho);
  return e + p / rho;
}

void
LinearFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);

  Real rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  dh_dp = 1.0 / rho - p / rho / rho * drho_dp;
  dh_dT = _cv - p / rho / rho * drho_dT;
}

Real
LinearFluidProperties::p_from_h_s(Real, Real) const
{
  mooseError(name(), ": p_from_h_s() not implemented");
}

void
LinearFluidProperties::p_from_h_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": p_from_h_s() not implemented");
}

Real
LinearFluidProperties::g_from_v_e(Real, Real) const
{
  mooseError(name(), ": g_from_v_e(v, e) not implemented");
}

Real
LinearFluidProperties::molarMass() const
{
  mooseError(name(), ": molarMass() not implemented");
}

Real
LinearFluidProperties::Pr(Real, Real) const
{
  return _Pr;
}
